// =============================================
// FILE: board_com.c
// =============================================
#include "board_com.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>   // va_list, va_start, va_end
#include <stdlib.h>   // strtoul

// ---- Internal helpers ----
static size_t s_write(char* out, size_t cap, const char* fmt, ...){
    if(cap==0) return 0;
    va_list ap; va_start(ap, fmt);
    int n = vsnprintf(out, cap, fmt, ap);
    va_end(ap);
    if(n<0) return 0; // encoding error
    if((size_t)n >= cap){ // truncated
        out[cap-1] = '\0';
        return cap-1;
    }
    return (size_t)n;
}

static uint32_t parse_t_ms(const char* tok){
    if(!tok) return 0;
    if(strncmp(tok,"t=",2)==0){ return (uint32_t)strtoul(tok+2, NULL, 10); }
    return 0;
}
static const char* kv_find(const char* const* toks, int ntok, const char* key){
    size_t klen = strlen(key);
    for(int i=0;i<ntok;i++){
        if(strncmp(toks[i], key, (int)klen)==0 && toks[i][klen]=='=') return toks[i]+(int)klen+1;
    }
    return NULL;
}

// ---- Line buffer ----
void cb_linebuf_init(cb_linebuf_t* lb){ lb->len=0; lb->buf[0]='\0'; }

void cb_linebuf_feed(cb_linebuf_t* lb, const uint8_t* data, size_t n, cb_on_line_fn on_line, void* user){
    for(size_t i=0;i<n;i++){
        char c = (char)data[i];
        if(c=='\r') continue; // ignore CR; we'll terminate on LF
        if(c=='\n'){
            // complete line
            lb->buf[lb->len] = '\0';
            if(lb->len>0 && on_line) on_line(lb->buf, user);
            lb->len = 0; // reset
            continue;
        }
        if(lb->len+1 < sizeof(lb->buf)){
            lb->buf[lb->len++] = c;
        }else{
            // overflow: reset buffer
            lb->len = 0;
        }
    }
}

// ---- Tokenizer ----
static int split_tokens(char* line, char* out[], int max_out){
    int n=0; char* p=line;
    // collapse leading spaces
    while(*p && isspace((unsigned char)*p)) p++;
    while(*p && n<max_out){
        out[n++] = p;
        // advance to space
        while(*p && !isspace((unsigned char)*p)) p++;
        if(!*p) break;
        *p++ = '\0';
        while(*p && isspace((unsigned char)*p)) p++;
    }
    return n;
}

// ---- Parser ----
bool cb_parse_line(const char* line_in, cb_msg_t* out){
    if(!line_in || !out) return false;
    memset(out, 0, sizeof(*out));

    // Make a mutable copy (tokenizer needs to write NULs)
    char buf[CB_MAX_LINE+2];
    size_t L = strnlen(line_in, CB_MAX_LINE);
    memcpy(buf, line_in, L); buf[L]='\0';

    char* tok[CB_MAX_TOKENS];
    int nt = split_tokens(buf, tok, CB_MAX_TOKENS);
    if(nt<=0) return false;

    if(strcmp(tok[0], "OK")==0){
        out->type = CB_RSP_OK;
        // Copy remainder as a single text payload for convenience
        if(nt>=2){
            const char* rest = line_in + 3; // after "OK "
            while(*rest==' ') rest++;
            strncpy(out->u.ok.text, rest, CB_MAX_STR-1);
            out->u.ok.text[CB_MAX_STR-1]='\0';
        }
        return true;
    }
    if(strcmp(tok[0], "ERR")==0){
        out->type = CB_RSP_ERR;
        if(nt>=2){ strncpy(out->u.err.code, tok[1], CB_MAX_STR-1); }
        if(nt>=3){
            const char* rest = strstr(line_in, tok[2]);
            if(rest){ strncpy(out->u.err.details, rest, CB_MAX_STR-1); }
        }
        return true;
    }
    if(strcmp(tok[0], "EVT")==0 && nt>=2){
        const char* t = tok[1];
        if(strcmp(t,"BOOT")==0){
            out->type = CB_EVT_BOOT;
            const char* fw = kv_find((const char* const*)tok, nt, "FW");
            const char* hw = kv_find((const char* const*)tok, nt, "HW");
            const char* ts = kv_find((const char* const*)tok, nt, "t");
            if(fw) strncpy(out->u.boot.fw, fw, CB_MAX_STR-1);
            if(hw) strncpy(out->u.boot.hw, hw, CB_MAX_STR-1);
            if(ts) out->u.boot.t_ms = (uint32_t)strtoul(ts, NULL, 10);
            return true;
        }
        if(strcmp(t,"LIFT")==0 && nt>=3){
            out->type = CB_EVT_LIFT;
            uint8_t idx; if(cb_sq_from_str(tok[2], &idx)) out->u.sq.idx=idx;
            out->u.sq.t_ms = (uint32_t)strtoul(kv_find((const char* const*)tok, nt, "t"), NULL, 10);
            return true;
        }
        if(strcmp(t,"PLACE")==0 && nt>=3){
            out->type = CB_EVT_PLACE;
            uint8_t idx; if(cb_sq_from_str(tok[2], &idx)) out->u.sq.idx=idx;
            out->u.sq.t_ms = (uint32_t)strtoul(kv_find((const char* const*)tok, nt, "t"), NULL, 10);
            return true;
        }
        if(strcmp(t,"MOVE")==0 && nt>=4){
            out->type = CB_EVT_MOVE;
            uint8_t f,tidx; if(cb_sq_from_str(tok[2], &f)) out->u.move.from_idx=f;
            if(cb_sq_from_str(tok[3], &tidx)) out->u.move.to_idx=tidx;
            out->u.move.t_ms = (uint32_t)strtoul(kv_find((const char* const*)tok, nt, "t"), NULL, 10);
            return true;
        }
        if(strcmp(t,"LIFT_CANCEL")==0 && nt>=3){
            out->type = CB_EVT_LIFT_CANCEL;
            uint8_t idx; if(cb_sq_from_str(tok[2], &idx)) out->u.sq.idx=idx;
            out->u.sq.t_ms = (uint32_t)strtoul(kv_find((const char* const*)tok, nt, "t"), NULL, 10);
            return true;
        }
        if(strcmp(t,"BTN")==0){
            out->type = CB_EVT_BTN;
            out->u.sq.t_ms = (uint32_t)strtoul(kv_find((const char* const*)tok, nt, "t"), NULL, 10);
            return true;
        }
        if(strcmp(t,"TIMEOUT")==0 && nt>=3){
            out->type = CB_EVT_TIMEOUT;
            strncpy(out->u.timeout.state, tok[2], CB_MAX_STR-1);
            out->u.timeout.t_ms = (uint32_t)strtoul(kv_find((const char* const*)tok, nt, "t"), NULL, 10);
            return true;
        }
        if(strcmp(t,"ERROR")==0 && nt>=3){
            out->type = CB_EVT_ERROR;
            strncpy(out->u.err_evt.code, tok[2], CB_MAX_STR-1);
            // details = everything between tok[3].. before a token starting with 't='
            out->u.err_evt.details[0]='\0';
            for(int i=3;i<nt;i++){
                if(strncmp(tok[i],"t=",2)==0){ out->u.err_evt.t_ms=(uint32_t)strtoul(tok[i]+2, NULL, 10); break; }
                size_t cur = strlen(out->u.err_evt.details);
                size_t left = CB_MAX_STR-1 - cur;
                if(left<=1) break;
                size_t add = strnlen(tok[i], left-1);
                strncat(out->u.err_evt.details, tok[i], add);
                if(add<left-1){ strncat(out->u.err_evt.details, " ", 1); }
            }
            return true;
        }
    }

    out->type = CB_RSP_UNKNOWN;
    return false;
}

// ---- Command formatters ----
#define W(fmt, ...) s_write(out, cap, fmt "\r\n", __VA_ARGS__)
#define W0(lit)     s_write(out, cap, lit "\r\n")

size_t cb_fmt_ping(char* out, size_t cap){ return W0(":PING"); }
size_t cb_fmt_ver_q(char* out, size_t cap){ return W0(":VER?"); }
size_t cb_fmt_time_q(char* out, size_t cap){ return W0(":TIME?"); }
size_t cb_fmt_rst(char* out, size_t cap){ return W0(":RST"); }
size_t cb_fmt_save(char* out, size_t cap){ return W0(":SAVE"); }
size_t cb_fmt_stream(char* out, size_t cap, bool on){ return W(":STREAM %s", on?"ON":"OFF"); }

size_t cb_fmt_read_all(char* out, size_t cap){ return W0(":READ ALL"); }
size_t cb_fmt_read_sq(char* out, size_t cap, uint8_t idx){ char sq[3]; cb_sq_to_str(idx, sq); return W(":READ SQ %s", sq); }
size_t cb_fmt_read_mask_set(char* out, size_t cap, uint64_t mask){ return W(":READ MASK SET 0x%016llX", (unsigned long long)mask); }
size_t cb_fmt_read_mask_q(char* out, size_t cap){ return W0(":READ MASK?"); }

size_t cb_fmt_led_set(char* out, size_t cap, uint8_t idx, uint8_t r, uint8_t g, uint8_t b){ char sq[3]; cb_sq_to_str(idx, sq); return W(":LED SET %s %u %u %u", sq, r,g,b); }
size_t cb_fmt_led_off_sq(char* out, size_t cap, uint8_t idx){ char sq[3]; cb_sq_to_str(idx, sq); return W(":LED OFF %s", sq); }
size_t cb_fmt_led_off_all(char* out, size_t cap){ return W0(":LED OFF ALL"); }
size_t cb_fmt_led_fill(char* out, size_t cap, uint8_t r, uint8_t g, uint8_t b){ return W(":LED FILL %u %u %u", r,g,b); }
size_t cb_fmt_led_rect(char* out, size_t cap, uint8_t from_idx, uint8_t to_idx, uint8_t r, uint8_t g, uint8_t b){ char a[3],bq[3]; cb_sq_to_str(from_idx,a); cb_sq_to_str(to_idx,bq); return W(":LED RECT %s %s %u %u %u", a,bq,r,g,b); }
size_t cb_fmt_led_bitboard(char* out, size_t cap, uint64_t bits){ return W(":LED BITBOARD 0x%016llX", (unsigned long long)bits); }
size_t cb_fmt_led_bright(char* out, size_t cap, uint8_t bright){ return W(":LED BRIGHT %u", bright); }
size_t cb_fmt_led_gamma(char* out, size_t cap, float gamma){ return W(":LED GAMMA %.3f", (double)gamma); }
size_t cb_fmt_led_map_hex(char* out, size_t cap, const char* hex192){ return W(":LED MAP %s", hex192?hex192:""); }

size_t cb_fmt_color_set(char* out, size_t cap, const char* name, uint8_t r, uint8_t g, uint8_t b){ return W(":COLOR SET %s %u %u %u", name?name:"COLOR.HINT", r,g,b); }
size_t cb_fmt_color_get(char* out, size_t cap, const char* name){ return W(":COLOR GET %s", name?name:"COLOR.HINT"); }
size_t cb_fmt_color_list_q(char* out, size_t cap){ return W0(":COLOR?"); }

size_t cb_fmt_led_moves(char* out, size_t cap, uint8_t from_idx, const uint8_t* to_list, size_t n){
    char a[3]; cb_sq_to_str(from_idx,a);
    size_t pos = s_write(out, cap, ":LED MOVES %s %u", a, (unsigned)n);
    for(size_t i=0;i<n && pos<cap; i++){
        char s[3]; cb_sq_to_str(to_list[i], s);
        size_t add = s_write(out+pos, cap-pos, " %s", s);
        pos += add;
    }
    if(pos<cap){ size_t add = s_write(out+pos, cap-pos, "\r\n"); pos+=add; }
    return pos>cap?cap:pos;
}
size_t cb_fmt_led_ok(char* out, size_t cap, uint8_t from_idx, uint8_t to_idx){ char a[3],b[3]; cb_sq_to_str(from_idx,a); cb_sq_to_str(to_idx,b); return W(":LED OK %s %s", a,b); }
size_t cb_fmt_led_fail(char* out, size_t cap, uint8_t from_idx, uint8_t to_idx){ char a[3],b[3]; cb_sq_to_str(from_idx,a); cb_sq_to_str(to_idx,b); return W(":LED FAIL %s %s", a,b); }

size_t cb_fmt_move_ack(char* out, size_t cap, uint8_t from_idx, uint8_t to_idx, char promo, char castle, int enpassant_idx){
    char a[3], b[3]; cb_sq_to_str(from_idx,a); cb_sq_to_str(to_idx,b);
    size_t pos = s_write(out, cap, ":MOVE ACK %s %s", a,b);
    if(promo){ pos += s_write(out+pos, cap-pos, " PROMO=%c", promo); }
    if(castle=='K' || castle=='Q'){ pos += s_write(out+pos, cap-pos, " CASTLE=%c", castle); }
    if(enpassant_idx>=0){ char ep[3]; cb_sq_to_str((uint8_t)enpassant_idx, ep); pos += s_write(out+pos, cap-pos, " ENPASSANT=%s", ep); }
    if(pos<cap){ pos += s_write(out+pos, cap-pos, "\r\n"); }
    return pos>cap?cap:pos;
}
size_t cb_fmt_move_nack(char* out, size_t cap, uint8_t from_idx, uint8_t to_idx, const char* reason){ char a[3],b[3]; cb_sq_to_str(from_idx,a); cb_sq_to_str(to_idx,b); if(reason && *reason) return W(":MOVE NACK %s %s REASON=%s", a,b,reason); else return W(":MOVE NACK %s %s", a,b); }

size_t cb_fmt_cfg_q(char* out, size_t cap){ return W0(":CFG?"); }
size_t cb_fmt_cfg_get(char* out, size_t cap, const char* key){ return W(":CFG GET %s", key?key:"DEBOUNCE_REED"); }
size_t cb_fmt_cfg_set_kv(char* out, size_t cap, const char* const* kv_pairs, size_t n_pairs){
    size_t pos = s_write(out, cap, ":CFG SET");
    for(size_t i=0;i<n_pairs && pos<cap; i++){
        pos += s_write(out+pos, cap-pos, " %s", kv_pairs[i]);
    }
    if(pos<cap){ pos += s_write(out+pos, cap-pos, "\r\n"); }
    return pos>cap?cap:pos;
}
size_t cb_fmt_cfg_set_debounce(char* out, size_t cap, uint16_t reed_ms, uint16_t btn_ms){ return W(":CFG SET DEBOUNCE_REED=%u DEBOUNCE_BTN=%u", (unsigned)reed_ms, (unsigned)btn_ms); }
size_t cb_fmt_cfg_set_orient(char* out, size_t cap, int orient_deg){ return W(":CFG SET ORIENT=%d", orient_deg); }
size_t cb_fmt_cfg_set_baud(char* out, size_t cap, uint32_t baud){ return W(":CFG SET BAUD=%u", (unsigned)baud); }


// -------- Event formatters (Board->App) --------
// NOTE: Les events n'ont PAS de ':' en tête, cf. parser cb_parse_line.

size_t cb_fmt_evt_boot(char* out, size_t cap, const char* fw, const char* hw, uint32_t t_ms){
    size_t pos = s_write(out, cap, "EVT BOOT");
    if(fw && *fw)  pos += s_write(out+pos, cap-pos, " FW=%s", fw);
    if(hw && *hw)  pos += s_write(out+pos, cap-pos, " HW=%s", hw);
    pos += s_write(out+pos, cap-pos, " t=%u\r\n", (unsigned)t_ms);
    return pos>cap?cap:pos;
}

size_t cb_fmt_evt_lift(char* out, size_t cap, uint8_t idx, uint32_t t_ms){
    char sq[3]; cb_sq_to_str(idx, sq);
    return s_write(out, cap, "EVT LIFT %s t=%u\r\n", sq, (unsigned)t_ms);
}

size_t cb_fmt_evt_place(char* out, size_t cap, uint8_t idx, uint32_t t_ms){
    char sq[3]; cb_sq_to_str(idx, sq);
    return s_write(out, cap, "EVT PLACE %s t=%u\r\n", sq, (unsigned)t_ms);
}

size_t cb_fmt_evt_move(char* out, size_t cap, uint8_t from_idx, uint8_t to_idx, uint32_t t_ms){
    char a[3], b[3]; cb_sq_to_str(from_idx, a); cb_sq_to_str(to_idx, b);
    return s_write(out, cap, "EVT MOVE %s %s t=%u\r\n", a, b, (unsigned)t_ms);
}

size_t cb_fmt_evt_lift_cancel(char* out, size_t cap, uint8_t idx, uint32_t t_ms){
    char sq[3]; cb_sq_to_str(idx, sq);
    return s_write(out, cap, "EVT LIFT_CANCEL %s t=%u\r\n", sq, (unsigned)t_ms);
}

size_t cb_fmt_evt_btn(char* out, size_t cap, uint32_t t_ms){
    return s_write(out, cap, "EVT BTN t=%u\r\n", (unsigned)t_ms);
}

size_t cb_fmt_evt_timeout(char* out, size_t cap, const char* state, uint32_t t_ms){
    if(!state || !*state) state = "TIMEOUT";
    return s_write(out, cap, "EVT TIMEOUT %s t=%u\r\n", state, (unsigned)t_ms);
}

size_t cb_fmt_evt_error(char* out, size_t cap, const char* code, const char* details, uint32_t t_ms){
    if(!code || !*code) code = "E";
    size_t pos = s_write(out, cap, "EVT ERROR %s", code);
    if(details && *details){
        pos += s_write(out+pos, cap-pos, " %s", details);
    }
    pos += s_write(out+pos, cap-pos, " t=%u\r\n", (unsigned)t_ms);
    return pos>cap?cap:pos;
}
