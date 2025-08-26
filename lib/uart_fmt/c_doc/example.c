// =============================================
// test_board_com
// Compile: gcc example.c "../src/board_com.c" -o test_board_com
// Win(MinGW): gcc example.c ../src/board_com.c -o test_board_com.exe
// =============================================
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../src/board_com.h"

/* --- Helpers --- */
static void print_sq(uint8_t idx){ char s[3]; cb_sq_to_str(idx,s); printf("%s",s); }

/* --- Affichage compact du parse côté PCB (commandes ':...') --- */
static void print_cmd(const cb_cmd_t* c){
    switch(c->type){
    case CB_CMD_PING:     puts("[CMD] :PING"); break;
    case CB_CMD_VER_Q:    puts("[CMD] :VER?"); break;
    case CB_CMD_TIME_Q:   puts("[CMD] :TIME?"); break;
    case CB_CMD_RST:      puts("[CMD] :RST"); break;
    case CB_CMD_SAVE:     puts("[CMD] :SAVE"); break;
    case CB_CMD_STREAM:   printf("[CMD] :STREAM %s\n", c->u.stream.on?"ON":"OFF"); break;

    case CB_CMD_READ_ALL: puts("[CMD] :READ ALL"); break;
    case CB_CMD_READ_SQ:  printf("[CMD] :READ SQ "); print_sq(c->u.read_sq.idx); puts(""); break;
    case CB_CMD_READ_MASK_Q: puts("[CMD] :READ MASK?"); break;
    case CB_CMD_READ_MASK_SET: printf("[CMD] :READ MASK SET 0x%016llX\n",(unsigned long long)c->u.read_mask_set.mask); break;

    case CB_CMD_LED_SET:
        printf("[CMD] :LED SET "); print_sq(c->u.led_set.idx);
        printf(" %u %u %u\n", c->u.led_set.r,c->u.led_set.g,c->u.led_set.b); break;
    case CB_CMD_LED_OFF_SQ: printf("[CMD] :LED OFF "); print_sq(c->u.led_off_sq.idx); puts(""); break;
    case CB_CMD_LED_OFF_ALL: puts("[CMD] :LED OFF ALL"); break;
    case CB_CMD_LED_FILL: printf("[CMD] :LED FILL %u %u %u\n", c->u.led_fill.r,c->u.led_fill.g,c->u.led_fill.b); break;
    case CB_CMD_LED_RECT:
        printf("[CMD] :LED RECT "); print_sq(c->u.led_rect.from_idx); printf(" ");
        print_sq(c->u.led_rect.to_idx); printf(" %u %u %u\n", c->u.led_rect.r,c->u.led_rect.g,c->u.led_rect.b); break;
    case CB_CMD_LED_BITBOARD: printf("[CMD] :LED BITBOARD 0x%016llX\n",(unsigned long long)c->u.led_bitboard.bits); break;
    case CB_CMD_LED_BRIGHT: printf("[CMD] :LED BRIGHT %u\n", c->u.led_bright.bright); break;
    case CB_CMD_LED_GAMMA:  printf("[CMD] :LED GAMMA %.3f\n", c->u.led_gamma.gamma); break;
    case CB_CMD_LED_MAP_HEX: printf("[CMD] :LED MAP <hex192 len=%zu>\n", strlen(c->u.led_map_hex.hex192)); break;
    case CB_CMD_LED_MOVES:
        printf("[CMD] :LED MOVES "); print_sq(c->u.led_moves.from_idx);
        printf(" n=%u\n", c->u.led_moves.n_to); break;
    case CB_CMD_LED_OK:   printf("[CMD] :LED OK "); print_sq(c->u.led_ok.from_idx); printf(" "); print_sq(c->u.led_ok.to_idx); puts(""); break;
    case CB_CMD_LED_FAIL: printf("[CMD] :LED FAIL "); print_sq(c->u.led_fail.from_idx); printf(" "); print_sq(c->u.led_fail.to_idx); puts(""); break;

    case CB_CMD_MOVE_ACK:
        printf("[CMD] :MOVE ACK "); print_sq(c->u.move_ack.from_idx); printf(" "); print_sq(c->u.move_ack.to_idx);
        printf(" promo=%c castle=%c ep=%d\n", c->u.move_ack.promo?c->u.move_ack.promo:'-', c->u.move_ack.castle?c->u.move_ack.castle:'-', c->u.move_ack.enpassant_idx);
        break;
    case CB_CMD_MOVE_NACK:
        printf("[CMD] :MOVE NACK "); print_sq(c->u.move_nack.from_idx); printf(" "); print_sq(c->u.move_nack.to_idx);
        printf(" reason=\"%s\"\n", c->u.move_nack.reason); break;

    case CB_CMD_CFG_Q:   puts("[CMD] :CFG?"); break;
    case CB_CMD_CFG_GET: printf("[CMD] :CFG GET %s\n", c->u.cfg_get.key); break;
    case CB_CMD_CFG_SET_KV:
        printf("[CMD] :CFG SET (%d kv)\n", c->u.cfg_set_kv.n_pairs); break;

    default: puts("[CMD] Unknown"); break;
    }
}

/* --- Affichage compact du parse PCB->App (EVT/OK/ERR) --- */
static void print_evt(const cb_msg_t* m){
    switch(m->type){
    case CB_EVT_BOOT:   printf("[EVT] BOOT FW=%s HW=%s t=%u\n", m->u.boot.fw,m->u.boot.hw,(unsigned)m->u.boot.t_ms); break;
    case CB_EVT_LIFT:   printf("[EVT] LIFT "); print_sq(m->u.sq.idx); printf(" t=%u\n",(unsigned)m->u.sq.t_ms); break;
    case CB_EVT_PLACE:  printf("[EVT] PLACE "); print_sq(m->u.sq.idx); printf(" t=%u\n",(unsigned)m->u.sq.t_ms); break;
    case CB_EVT_MOVE:   printf("[EVT] MOVE "); print_sq(m->u.move.from_idx); printf("->"); print_sq(m->u.move.to_idx); printf(" t=%u\n",(unsigned)m->u.move.t_ms); break;
    case CB_EVT_LIFT_CANCEL: printf("[EVT] LIFT_CANCEL "); print_sq(m->u.sq.idx); printf(" t=%u\n",(unsigned)m->u.sq.t_ms); break;
    case CB_EVT_BTN:    printf("[EVT] BTN t=%u\n",(unsigned)m->u.sq.t_ms); break;
    case CB_EVT_TIMEOUT:printf("[EVT] TIMEOUT %s t=%u\n",m->u.timeout.state,(unsigned)m->u.timeout.t_ms); break;
    case CB_EVT_ERROR:  printf("[EVT] ERROR %s \"%s\" t=%u\n",m->u.err_evt.code,m->u.err_evt.details,(unsigned)m->u.err_evt.t_ms); break;
    case CB_RSP_OK:     printf("[RSP] OK \"%s\"\n", m->u.ok.text); break;
    case CB_RSP_ERR:    printf("[RSP] ERR %s \"%s\"\n", m->u.err.code, m->u.err.details); break;
    default:            printf("[WARN] Unrecognized (type=%d)\n",(int)m->type); break;
    }
}

/* --- Line-buffer callback: route selon le 1er char --- */
static void on_line_cb(const char* line, void* user){
    (void)user;
    if(line[0]==':'){                 // App -> PCB (commandes)
        cb_cmd_t cmd;
        if(cb_parse_cmd(line, &cmd))  print_cmd(&cmd);
        else                          puts("[CMD] parse error");
    }else{                            // PCB -> App (EVT/OK/ERR)
        cb_msg_t m;
        if(cb_parse_line(line, &m))   print_evt(&m);
        else                          puts("[EVT] parse error");
    }
}

/* --- Démo formatters : on montre juste les lignes générées --- */
static void demo_formatters(void){
    char out[CB_MAX_LINE];

    cb_fmt_ping(out,sizeof out);          printf("[TX ] %s", out);
    cb_fmt_ver_q(out,sizeof out);         printf("[TX ] %s", out);
    cb_fmt_stream(out,sizeof out,true);   printf("[TX ] %s", out);

    uint8_t e2 = cb_coords_to_idx(4,1), e4 = cb_coords_to_idx(4,3);
    cb_fmt_led_set(out,sizeof out,e2,255,128,0); printf("[TX ] %s", out);
    cb_fmt_led_ok(out,sizeof out,e2,e4);         printf("[TX ] %s", out);
    cb_fmt_move_ack(out,sizeof out,e2,e4,'Q',0,-1); printf("[TX ] %s", out);
    cb_fmt_cfg_set_debounce(out,sizeof out,30,100);  printf("[TX ] %s", out);
    cb_fmt_read_mask_set(out,sizeof out,0x00000000FFFFFFFFULL); printf("[TX ] %s", out);
}

/* --- Démo line-buffer: mélange EVT et commandes --- */
static void demo_linebuffer(void){
    cb_linebuf_t lb; cb_linebuf_init(&lb);

    const char* sim =
        /* PCB->App */
        "EVT BOOT FW=1.2.3 HW=CHESS-01 t=100\r\n"
        "EVT LIFT E2 t=102345\n"
        "OK VER FW=1.2.3 HW=CHESS-01\r\n"
        "ERR E_BAUD Unsupported baud rate\r\n"
        /* App->PCB */
        ":PING\r\n"
        ":STREAM ON\r\n"
        ":LED SET E2 255 128 0\r\n"
        ":READ SQ E4\r\n"
        ":CFG SET DEBOUNCE_REED=30 DEBOUNCE_BTN=100\r\n";

    cb_linebuf_feed(&lb,(const uint8_t*)sim,strlen(sim),on_line_cb,NULL);
}

int main(void){
    puts("=== Formatters (exemples) ===");
    demo_formatters();

    puts("\n=== Line-buffer + parsing (EVT + :CMD) ===");
    demo_linebuffer();

    puts("\nDone.");
    return 0;
}
