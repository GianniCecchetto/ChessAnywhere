// =============================================
// FILE: firmware_demo.c
// Minimal STM32 (HAL) firmware demo using board_com
// - RX UART byte-by-byte -> cb_linebuf_feed (board_com)
// - Parse commands ":LED ..." (uses cb_sq_from_str)
// - Emit events "EVT ..."     (uses cb_sq_to_str)
// =============================================
#include "stm32g0xx_hal.h"   // <-- adapte à ta MCU
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "board_com.h"       // API de la lib (line buffer, helpers)  [:contentReference[oaicite:2]{index=2}]

// ================== Configuration UART ==================
extern UART_HandleTypeDef huart2;      // généré par CubeMX
static uint8_t rx_byte;                // réception 1 octet

// ================== Line-buffer (lib board_com) ==================
static cb_linebuf_t g_lb;              // buffer d'assemblage de lignes  [:contentReference[oaicite:3]{index=3}]
static volatile int g_stream_on = 1;

// ================== Abstraction LEDs (remplacer par ton driver) ==================
static void leds_init(void) { /* TODO: init WS2812/driver user */ }
static void leds_set_sq(uint8_t idx, uint8_t r, uint8_t g, uint8_t b) {
    (void)idx; (void)r; (void)g; (void)b;
    // TODO: set pixel + show
}
static void leds_off_sq(uint8_t idx){ leds_set_sq(idx, 0,0,0); }
static void leds_off_all(void){ for (uint8_t i=0;i<64;i++) leds_off_sq(i); }
static void leds_fill(uint8_t r, uint8_t g, uint8_t b){ for(uint8_t i=0;i<64;i++) leds_set_sq(i,r,g,b); }
static void leds_set_brightness(uint8_t br){ (void)br; /* TODO if supported */ }
static void leds_show_bitboard(uint64_t bits){
    for(uint8_t i=0;i<64;i++){
        if(bits & (1ULL<<i)) leds_set_sq(i,0,255,0);
        else                 leds_off_sq(i);
    }
}

// ================== UART helpers ==================
static inline void uart_send(const char* s){ HAL_UART_Transmit(&huart2, (uint8_t*)s, (uint16_t)strlen(s), 1000); }
static inline void start_rx_it(void){ HAL_UART_Receive_IT(&huart2, &rx_byte, 1); }
static inline uint32_t millis(void){ return HAL_GetTick(); }

// ================== Emission d'EVT (uses cb_sq_to_str) ==================
static void evt_boot(const char* fw, const char* hw){
    char line[CB_MAX_LINE];
    snprintf(line, sizeof(line), "EVT BOOT FW=%s HW=%s t=%u\r\n",
             fw?fw:"?", hw?hw:"?", (unsigned)millis());
    if(g_stream_on) uart_send(line);
}
static void evt_lift(uint8_t idx){
    char sq[3]; cb_sq_to_str(idx, sq);     // helper lib  [:contentReference[oaicite:4]{index=4}]
    char line[CB_MAX_LINE];
    snprintf(line, sizeof(line), "EVT LIFT %s t=%u\r\n", sq, (unsigned)millis());
    if(g_stream_on) uart_send(line);
}
static void evt_place(uint8_t idx){
    char sq[3]; cb_sq_to_str(idx, sq);
    char line[CB_MAX_LINE];
    snprintf(line, sizeof(line), "EVT PLACE %s t=%u\r\n", sq, (unsigned)millis());
    if(g_stream_on) uart_send(line);
}
static void evt_move(uint8_t from_idx, uint8_t to_idx){
    char a[3], b[3]; cb_sq_to_str(from_idx,a); cb_sq_to_str(to_idx,b);
    char line[CB_MAX_LINE];
    snprintf(line, sizeof(line), "EVT MOVE %s %s t=%u\r\n", a, b, (unsigned)millis());
    if(g_stream_on) uart_send(line);
}

// ================== Réponses simples ==================
static void rsp_ok(const char* text){
    char line[CB_MAX_LINE];
    if(text && *text) snprintf(line,sizeof(line),"OK %s\r\n",text);
    else              snprintf(line,sizeof(line),"OK\r\n");
    uart_send(line);
}
static void rsp_err(const char* code, const char* details){
    char line[CB_MAX_LINE];
    if(details && *details) snprintf(line,sizeof(line),"ERR %s %s\r\n", code?code:"E", details);
    else                    snprintf(line,sizeof(line),"ERR %s\r\n", code?code:"E");
    uart_send(line);
}

// ================== Parser des commandes ":..." ==================
static void handle_command_line(const char* line){
    // On veut un code facile à lire : tokenisation minimaliste par espaces.
    char buf[CB_MAX_LINE+2]; size_t L = strnlen(line, CB_MAX_LINE);
    memcpy(buf, line, L); buf[L] = '\0';

    char* tok[20] = {0};
    int nt=0; char* p=buf;
    while(*p && nt<20){
        while(*p==' ') ++p; if(!*p) break;
        tok[nt++] = p;
        while(*p && *p!=' ') ++p;
        if(*p){ *p++ = '\0'; }
    }
    if(nt==0) return;

    // :PING
    if(strcmp(tok[0], ":PING")==0){ rsp_ok("PONG"); return; }

    // :STREAM ON|OFF
    if(strcmp(tok[0], ":STREAM")==0 && nt>=2){
        if(strcmp(tok[1],"ON")==0){ g_stream_on=1; rsp_ok("STREAM=ON"); }
        else if(strcmp(tok[1],"OFF")==0){ g_stream_on=0; rsp_ok("STREAM=OFF"); }
        else rsp_err("E_ARG","STREAM needs ON|OFF");
        return;
    }

    // :LED ...
    if(strcmp(tok[0], ":LED")==0 && nt>=2){
        // :LED OFF ALL
        if(strcmp(tok[1],"OFF")==0){
            if(nt>=3 && strcmp(tok[2],"ALL")==0){ leds_off_all(); rsp_ok("LED OFF ALL"); return; }
            if(nt>=3){
                uint8_t idx=0;
                if(!cb_sq_from_str(tok[2], &idx)){ rsp_err("E_SQ","Bad square"); return; } // helper lib  [:contentReference[oaicite:5]{index=5}]
                leds_off_sq(idx); rsp_ok("LED OFF SQ");
                return;
            }
            rsp_err("E_ARG","LED OFF ALL|<SQ>"); return;
        }

        // :LED SET <SQ> R G B
        if(strcmp(tok[1],"SET")==0){
            if(nt>=6){
                uint8_t idx=0;
                if(!cb_sq_from_str(tok[2], &idx)){ rsp_err("E_SQ","Bad square"); return; }
                int r=atoi(tok[3]), g=atoi(tok[4]), b=atoi(tok[5]);
                if(r<0)r=0; if(r>255)r=255; if(g<0)g=0; if(g>255)g=255; if(b<0)b=0; if(b>255)b=255;
                leds_set_sq(idx,(uint8_t)r,(uint8_t)g,(uint8_t)b);
                rsp_ok("LED SET");
            }else rsp_err("E_ARG","LED SET <SQ> R G B");
            return;
        }

        // :LED FILL R G B
        if(strcmp(tok[1],"FILL")==0){
            if(nt>=5){
                int r=atoi(tok[2]), g=atoi(tok[3]), b=atoi(tok[4]);
                if(r<0)r=0; if(r>255)r=255; if(g<0)g=0; if(g>255)g=255; if(b<0)b=0; if(b>255)b=255;
                leds_fill((uint8_t)r,(uint8_t)g,(uint8_t)b);
                rsp_ok("LED FILL");
            }else rsp_err("E_ARG","LED FILL R G B");
            return;
        }

        // :LED BRIGHT <0..255>
        if(strcmp(tok[1],"BRIGHT")==0){
            if(nt>=3){
                int br=atoi(tok[2]); if(br<0)br=0; if(br>255)br=255;
                leds_set_brightness((uint8_t)br);
                rsp_ok("LED BRIGHT");
            }else rsp_err("E_ARG","LED BRIGHT <0..255>");
            return;
        }

        // :LED BITBOARD 0x<64bits>
        if(strcmp(tok[1],"BITBOARD")==0){
            if(nt>=3){
                uint64_t bits=0ULL;
                if(!strncmp(tok[2],"0x",2)||!strncmp(tok[2],"0X",2)) bits=strtoull(tok[2]+2,NULL,16);
                else bits=strtoull(tok[2],NULL,0);
                leds_show_bitboard(bits);
                rsp_ok("LED BITBOARD");
            }else rsp_err("E_ARG","LED BITBOARD 0x<64bits>");
            return;
        }

        rsp_err("E_CMD","Unknown LED subcmd");
        return;
    }

    rsp_err("E_CMD","Unknown command");
}

// ================== Callback line-buffer (lib) ==================
static void on_line_cb(const char* line, void* user){
    (void)user;
    if(!line || !*line) return;
    if(line[0] == ':'){
        // Commande App->Board (formatée côté PC avec les formatters de la lib)  [:contentReference[oaicite:6]{index=6}]
        handle_command_line(line);
    }else{
        // Si besoin: on pourrait interpréter des "OK/ERR/EVT" renvoyés par un simulateur côté PC
        // via cb_parse_line(line, &msg); (la lib sait parser ces lignes).  [:contentReference[oaicite:7]{index=7}]
    }
}

// ================== Hooks HAL ==================
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    if(huart == &huart2){
        // Assemble proprement avec la lib board_com  [:contentReference[oaicite:8]{index=8}]
        cb_linebuf_feed(&g_lb, &rx_byte, 1, on_line_cb, NULL);
        HAL_UART_Receive_IT(&huart2, &rx_byte, 1);
    }
}

// (optionnel) redirection printf
int _write(int file, char *ptr, int len){
    (void)file;
    HAL_UART_Transmit(&huart2, (uint8_t*)ptr, (uint16_t)len, 100);
    return len;
}

// ================== main() ==================
int main(void){
    HAL_Init();
    SystemClock_Config();     // généré par CubeMX
    MX_GPIO_Init();
    MX_USART2_UART_Init();

    leds_init();
    cb_linebuf_init(&g_lb);   // init du line-buffer de la lib  [:contentReference[oaicite:9]{index=9}]

    // Démarre la RX IT
    HAL_UART_Receive_IT(&huart2, &rx_byte, 1);

    // Annonce BOOT
    evt_boot("1.0.0", "CHESS-01");

    // Démo simple : allumer A1 bleu puis annoncer LIFT/PLACE/MOVE
    uint8_t A1 = cb_coords_to_idx(0,0);       // helper lib  [:contentReference[oaicite:10]{index=10}]
    leds_set_sq(A1, 0,0,64);
    evt_lift(A1);
    HAL_Delay(200);
    evt_place(A1);
    HAL_Delay(200);
    uint8_t A2 = cb_coords_to_idx(0,1);
    evt_move(A1, A2);

    while(1){
        // Ici: ton scan reeds/boutons → appelle evt_lift/evt_place/evt_move au bon moment.
    }
}
