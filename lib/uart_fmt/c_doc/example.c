// =============================================
// test_board_com
// Compile with:  gcc example.c "../src/board_com.c" -o test_board_com
// (MinGW)      :  gcc example.c ../src/board_com.c -o test_board_com.exe
// =============================================
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "../src/board_com.h"

// ---- Pretty print helpers ----
static void print_sq(uint8_t idx){
    char s[3]; cb_sq_to_str(idx, s);
    printf("%s", s);
}

static void dump_cmd(const char* title, const char* cmd){
    // Les formatters ajoutent déjà CRLF, on montre aussi les octets spéciaux.
    printf("[CMD] %-12s -> %s", title, cmd);
    size_t n = strlen(cmd);
    if(n==0 || cmd[n-1] != '\n'){
        printf(" (no LF at end)\n");
    }
}

// ---- Parser demo (imprime un cb_msg_t) ----
static void print_parsed(const cb_msg_t* m){
    switch(m->type){
        case CB_EVT_BOOT:
            printf("[PARSED] EVT BOOT  FW=%s  HW=%s  t=%u\n",
                   m->u.boot.fw, m->u.boot.hw, (unsigned)m->u.boot.t_ms);
            break;
        case CB_EVT_LIFT:
            printf("[PARSED] EVT LIFT "); print_sq(m->u.sq.idx);
            printf(" t=%u\n", (unsigned)m->u.sq.t_ms);
            break;
        case CB_EVT_PLACE:
            printf("[PARSED] EVT PLACE "); print_sq(m->u.sq.idx);
            printf(" t=%u\n", (unsigned)m->u.sq.t_ms);
            break;
        case CB_EVT_MOVE:
            printf("[PARSED] EVT MOVE "); print_sq(m->u.move.from_idx);
            printf(" -> "); print_sq(m->u.move.to_idx);
            printf(" t=%u\n", (unsigned)m->u.move.t_ms);
            break;
        case CB_EVT_LIFT_CANCEL:
            printf("[PARSED] EVT LIFT_CANCEL "); print_sq(m->u.sq.idx);
            printf(" t=%u\n", (unsigned)m->u.sq.t_ms);
            break;
        case CB_EVT_BTN:
            printf("[PARSED] EVT BTN t=%u\n", (unsigned)m->u.sq.t_ms);
            break;
        case CB_EVT_TIMEOUT:
            printf("[PARSED] EVT TIMEOUT state=\"%s\" t=%u\n",
                   m->u.timeout.state, (unsigned)m->u.timeout.t_ms);
            break;
        case CB_EVT_ERROR:
            printf("[PARSED] EVT ERROR code=%s details=\"%s\" t=%u\n",
                   m->u.err_evt.code, m->u.err_evt.details, (unsigned)m->u.err_evt.t_ms);
            break;
        case CB_RSP_OK:
            printf("[PARSED] RSP OK  text=\"%s\"\n", m->u.ok.text);
            break;
        case CB_RSP_ERR:
            printf("[PARSED] RSP ERR code=%s details=\"%s\"\n",
                   m->u.err.code, m->u.err.details);
            break;
        case CB_RSP_UNKNOWN:
        default:
            printf("[PARSED] Unknown / not recognized (type=%d)\n", (int)m->type);
            break;
    }
}

// ---- Line-buffer callback ----
static void on_line_cb(const char* line, void* user){
    (void)user;
    printf("[RX ] %s\n", line);

    cb_msg_t m;
    if(cb_parse_line(line, &m)){
        print_parsed(&m);
    }else{
        printf("[WARN] Unparsed line\n");
    }
}

static void test_formatters(void){
    char out[CB_MAX_LINE];

    size_t n = cb_fmt_ping(out, sizeof(out));
    dump_cmd(":PING", out);

    n = cb_fmt_ver_q(out, sizeof(out));
    dump_cmd(":VER?", out);

    n = cb_fmt_stream(out, sizeof(out), true);
    dump_cmd(":STREAM ON", out);

    // LED SET E2 255 128 0
    uint8_t e2 = cb_coords_to_idx(4/*E*/, 1/*2*/);
    n = cb_fmt_led_set(out, sizeof(out), e2, 255, 128, 0);
    dump_cmd(":LED SET", out);

    // LED OK E2 E4
    uint8_t e4 = cb_coords_to_idx(4/*E*/, 3/*4*/);
    n = cb_fmt_led_ok(out, sizeof(out), e2, e4);
    dump_cmd(":LED OK", out);

    // MOVE ACK E2 E4 PROMO=Q (juste pour montrer les options)
    n = cb_fmt_move_ack(out, sizeof(out), e2, e4, 'Q', 0, -1);
    dump_cmd(":MOVE ACK", out);

    // CFG SET DEBOUNCE
    n = cb_fmt_cfg_set_debounce(out, sizeof(out), 30, 100);
    dump_cmd(":CFG SET DEBOUNCE", out);

    // READ MASK SET
    n = cb_fmt_read_mask_set(out, sizeof(out), 0x00000000FFFFFFFFULL);
    dump_cmd(":READ MASK SET", out);
}

static void test_linebuffer_and_parser(void){
    cb_linebuf_t lb; cb_linebuf_init(&lb);

    // Flux UART simulé (concat de plusieurs lignes, CR facultatif)
    const char* sim =
        "EVT BOOT FW=1.2.3 HW=CHESS-01 t=100\r\n"
        "EVT LIFT E2 t=102345\n"
        "EVT PLACE E4 t=102900\r\n"
        "EVT MOVE E2 E4 t=103000\r\n"
        "EVT LIFT_CANCEL E2 t=103050\r\n"
        "EVT BTN t=103200\r\n"
        "EVT TIMEOUT WAIT_MOVE t=200000\r\n"
        "EVT ERROR BAD_MOVE Illegal target square t=201000\r\n"
        "OK VER FW=1.2.3 HW=CHESS-01\r\n"
        "ERR E_BAUD Unsupported baud rate\r\n";

    cb_linebuf_feed(&lb, (const uint8_t*)sim, strlen(sim), on_line_cb, NULL);
}

int main(void){
    printf("=== Test board_com (formatters) ===\n");
    test_formatters();

    printf("\n=== Test board_com (line-buffer + parser) ===\n");
    test_linebuffer_and_parser();

    printf("\nDone.\n");
    return 0;
}
