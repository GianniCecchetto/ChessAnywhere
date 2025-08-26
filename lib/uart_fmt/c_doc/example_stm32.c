#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "board_com.h"   // linebuf, cb_sq_from_str, cb_fmt_evt_*  (voir API) 

extern UART_HandleTypeDef huart2;

/* --- UART line buffer --- */
static cb_linebuf_t s_lb;
static uint8_t      s_rx;

/* --- helpers UART --- */
static inline uint32_t t_ms(void){ return HAL_GetTick(); }
static void uart_write(const char* s){ HAL_UART_Transmit(&huart2,(uint8_t*)s,(uint16_t)strlen(s),100); }
static void uart_write_n(const char* s, size_t n){ HAL_UART_Transmit(&huart2,(uint8_t*)s,(uint16_t)n,100); }

/* --- (stub) pilote LED à remplacer --- */
static void led_set(uint8_t idx, uint8_t r, uint8_t g, uint8_t b){ (void)idx;(void)r;(void)g;(void)b; }
static void led_flush(void){}

/* --- Callback à chaque ligne ASCII reçue (sans CR/LF) --- */
static void on_line(const char* line, void* user){
    (void)user;

    /* 1) Si c'est une commande :LED ... on parse très simple ici */
    if(line[0]==':'){
        char buf[CB_MAX_LINE+2]; strncpy(buf,line,sizeof buf-1); buf[sizeof buf-1]='\0';
        char* save=NULL;
        char* t0=strtok_r(buf," \t",&save); if(!t0) return; if(t0[0]==':') t0++;
        if(strcmp(t0,"LED")==0){
            char* sub=strtok_r(NULL," \t",&save);
            if(sub && strcmp(sub,"SET")==0){
                char* sq=strtok_r(NULL," \t",&save);
                char* rS=strtok_r(NULL," \t",&save);
                char* gS=strtok_r(NULL," \t",&save);
                char* bS=strtok_r(NULL," \t",&save);
                uint8_t idx; 
                if(sq && rS && gS && bS && cb_sq_from_str(sq,&idx)){ // helper lib
                    led_set(idx,(uint8_t)atoi(rS),(uint8_t)atoi(gS),(uint8_t)atoi(bS));
                    led_flush();
                    uart_write("OK LED SET\r\n");
                } else {
                    uart_write("ERR E_ARGS\r\n");
                }
                return;
            }
            uart_write("ERR E_CMD\r\n");
            return;
        }

        /* autres :... si besoin */
        uart_write("ERR E_CMD\r\n");
        return;
    }

    /* 2) Si on reçoit un EVT/OK/ERR (rare côté MCU), on peut le décoder avec cb_parse_line */
    cb_msg_t msg;
    if(cb_parse_line(line,&msg)){ /* EVT/OK/ERR reconnus par la lib */ }
}

/* --- IT RX --- */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* hu){
    if(hu==&huart2){
        cb_linebuf_feed(&s_lb,&s_rx,1,on_line,NULL);  // assemble les lignes \n  (lib) 
        HAL_UART_Receive_IT(&huart2,&s_rx,1);
    }
}

/* --- Exemples d'émission d'EVT via les formatters de la lib --- */
static void send_evt_boot(void){
    char out[CB_MAX_LINE];
    size_t n = cb_fmt_evt_boot(out,sizeof out,"FW1.0.0","PCBv1",t_ms()); // EVT BOOT ...
    uart_write_n(out,n);                                                 // (lib) 
}
static void send_evt_btn(void){
    char out[CB_MAX_LINE];
    size_t n = cb_fmt_evt_btn(out,sizeof out,t_ms());  // EVT BTN t=...
    uart_write_n(out,n);
}
static void send_evt_place(uint8_t idx){
    char out[CB_MAX_LINE];
    size_t n = cb_fmt_evt_place(out,sizeof out,idx,t_ms()); // EVT PLACE <SQ> t=...
    uart_write_n(out,n);
}

int main(void){
    HAL_Init(); SystemClock_Config(); MX_GPIO_Init(); MX_USART2_UART_Init();

    cb_linebuf_init(&s_lb);                               // init line-buffer (lib) 
    HAL_UART_Receive_IT(&huart2,&s_rx,1);

    send_evt_boot();                                      // annonce au démarrage

    /* Boucle: bouton -> EVT BTN, exemple d’un reed fixe -> EVT PLACE E2 */
    uint8_t btn_prev = 1;
    for(;;){
        uint8_t btn_now = HAL_GPIO_ReadPin(B1_GPIO_Port,B1_Pin);
        if(btn_now!=btn_prev){
            btn_prev=btn_now;
            if(btn_now==GPIO_PIN_RESET) send_evt_btn();   // front d’appui
        }

        /* démo: envoyer un PLACE E2 de temps en temps */
        static uint32_t last=0; uint32_t now=t_ms();
        if(now-last>3000){ last=now; send_evt_place(/*E2*/ cb_coords_to_idx(4,1)); }

        HAL_Delay(1);
    }
}
