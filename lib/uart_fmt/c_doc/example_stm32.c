#include "main.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "../src/board_com.h"

extern UART_HandleTypeDef huart2;

/* --- UART line buffer --- */
static cb_linebuf_t s_lb;
static uint8_t      s_rx;

/* --- helpers UART --- */
static inline uint32_t t_ms(void){ return HAL_GetTick(); }
static void uart_write(const char* s){ HAL_UART_Transmit(&huart2,(uint8_t*)s,(uint16_t)strlen(s),100); }
static void uart_write_n(const char* s, size_t n){ HAL_UART_Transmit(&huart2,(uint8_t*)s,(uint16_t)n,100); }

/* --- (stubs) pilote LED/CFG à remplacer par ton driver --- */
static void led_set(uint8_t idx, uint8_t r, uint8_t g, uint8_t b){ (void)idx;(void)r;(void)g;(void)b; }
static void led_off_all(void){}
static void led_fill(uint8_t r, uint8_t g, uint8_t b){ (void)r;(void)g;(void)b; }
static void led_rect(uint8_t a, uint8_t b, uint8_t r, uint8_t g, uint8_t bl){ (void)a;(void)b;(void)r;(void)g;(void)bl; }
static void led_bitboard(uint64_t bits){ (void)bits; }
static void led_bright(uint8_t v){ (void)v; }
static void led_gamma(float g){ (void)g; }
static void cfg_set_kv(const char* kv){ (void)kv; }

/* --- Callback à chaque ligne ASCII reçue (sans CR/LF) --- */
static void on_line(const char* line, void* user){
    (void)user;

    if(line[0]==':'){ // Commande App->PCB
        cb_cmd_t cmd;
        if(!cb_parse_cmd(line, &cmd)){ uart_write("ERR PARSE\r\n"); return; }

        switch(cmd.type){
        case CB_CMD_PING:      uart_write("OK PING\r\n"); break;
        case CB_CMD_VER_Q:     uart_write("OK FW=FW1.0.0 HW=PCBv1\r\n"); break;
        case CB_CMD_TIME_Q:   { char o[48]; int n=snprintf(o,sizeof o,"OK TIME %lu\r\n",(unsigned long)t_ms()); uart_write_n(o,(size_t)n); } break;
        case CB_CMD_RST:       NVIC_SystemReset(); break;
        case CB_CMD_SAVE:      uart_write("OK SAVE\r\n"); break;
        case CB_CMD_STREAM:    /* cmd.u.stream.on */ uart_write("OK STREAM\r\n"); break;

        /* READ */
        case CB_CMD_READ_ALL:  uart_write("OK READ ALL 0x0000000000000000\r\n"); break;
        case CB_CMD_READ_SQ:  { char sq[3]; cb_sq_to_str(cmd.u.read_sq.idx,sq);
                                char o[32]; int n=snprintf(o,sizeof o,"OK READ SQ %s 0\r\n",sq); uart_write_n(o,(size_t)n); } break;
        case CB_CMD_READ_MASK_Q:  uart_write("OK READ MASK 0x0000000000000000\r\n"); break;
        case CB_CMD_READ_MASK_SET: uart_write("OK\r\n"); break;

        /* LED */
        case CB_CMD_LED_SET:       led_set(cmd.u.led_set.idx, cmd.u.led_set.r, cmd.u.led_set.g, cmd.u.led_set.b); uart_write("OK\r\n"); break;
        case CB_CMD_LED_OFF_ALL:   led_off_all(); uart_write("OK\r\n"); break;
        case CB_CMD_LED_FILL:      led_fill(cmd.u.led_fill.r, cmd.u.led_fill.g, cmd.u.led_fill.b); uart_write("OK\r\n"); break;
        case CB_CMD_LED_RECT:      led_rect(cmd.u.led_rect.from_idx, cmd.u.led_rect.to_idx, cmd.u.led_rect.r, cmd.u.led_rect.g, cmd.u.led_rect.b); uart_write("OK\r\n"); break;
        case CB_CMD_LED_BITBOARD:  led_bitboard(cmd.u.led_bitboard.bits); uart_write("OK\r\n"); break;
        case CB_CMD_LED_BRIGHT:    led_bright(cmd.u.led_bright.bright); uart_write("OK\r\n"); break;
        case CB_CMD_LED_GAMMA:     led_gamma(cmd.u.led_gamma.gamma); uart_write("OK\r\n"); break;
        case CB_CMD_LED_MAP_HEX:   /* cmd.u.led_map_hex.hex192 */ uart_write("OK\r\n"); break;
        case CB_CMD_LED_MOVES:     uart_write("OK\r\n"); break;
        case CB_CMD_LED_OK:        uart_write("OK\r\n"); break;
        case CB_CMD_LED_FAIL:      uart_write("OK\r\n"); break;

        /* MOVE */
        case CB_CMD_MOVE_ACK:      uart_write("OK\r\n"); break;
        case CB_CMD_MOVE_NACK:     uart_write("OK\r\n"); break;

        /* CFG */
        case CB_CMD_CFG_Q:         uart_write("OK CFG\r\n"); break;
        case CB_CMD_CFG_GET:       uart_write("OK CFG VAL\r\n"); break;
        case CB_CMD_CFG_SET_KV:    for(int i=0;i<cmd.u.cfg_set_kv.n_pairs;i++) cfg_set_kv(cmd.u.cfg_set_kv.pairs[i]); uart_write("OK\r\n"); break;

        default: uart_write("ERR CMD\r\n"); break;
        }
        return;
    }

    /* EVT/OK/ERR (rare côté MCU) */
    cb_msg_t msg;
    if(cb_parse_line(line,&msg)){ /* si besoin: traiter */ }
}

/* --- IT RX --- */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef* hu){
    if(hu==&huart2){
        cb_linebuf_feed(&s_lb,&s_rx,1,on_line,NULL);
        HAL_UART_Receive_IT(&huart2,&s_rx,1);
    }
}

/* --- Exemples d'émission d'EVT via les formatters --- */
static void send_evt_boot(void){
    char out[CB_MAX_LINE];
    size_t n = cb_fmt_evt_boot(out,sizeof out,"FW1.0.0","PCBv1",t_ms());
    uart_write_n(out,n);
}
static void send_evt_btn(void){
    char out[CB_MAX_LINE];
    size_t n = cb_fmt_evt_btn(out,sizeof out,t_ms());
    uart_write_n(out,n);
}
static void send_evt_place(uint8_t idx){
    char out[CB_MAX_LINE];
    size_t n = cb_fmt_evt_place(out,sizeof out,idx,t_ms());
    uart_write_n(out,n);
}

int main(void){
    HAL_Init(); SystemClock_Config(); MX_GPIO_Init(); MX_USART2_UART_Init();

    cb_linebuf_init(&s_lb);
    HAL_UART_Receive_IT(&huart2,&s_rx,1);

    send_evt_boot();

    uint8_t btn_prev = 1;
    for(;;){
        uint8_t btn_now = HAL_GPIO_ReadPin(B1_GPIO_Port,B1_Pin);
        if(btn_now!=btn_prev){
            btn_prev=btn_now;
            if(btn_now==GPIO_PIN_RESET) send_evt_btn();
        }
        static uint32_t last=0; uint32_t now=t_ms();
        if(now-last>3000){ last=now; send_evt_place(cb_coords_to_idx(4,1)); }
        HAL_Delay(1);
    }
}
