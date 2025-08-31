// =============================================
// FILE: board_com.c
// =============================================
#include "board_com.h"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>   // va_list, va_start, va_end
#include <stdlib.h>   // strtoul
<<<<<<< HEAD
#include <math.h>
=======
>>>>>>> main

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

<<<<<<< HEAD
=======
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

>>>>>>> main
/* Convert reed index to led index
 * Return the led index corresponding to the reed triggered
 */
uint8_t convert_reed_index_to_led_index(uint8_t reed_index) {
	if((reed_index / 8) % 2 == 0) {
		return reed_index;
	} else {
		// The magical formule to get the led index
		return (16 * (uint8_t)(reed_index / 8)) + 7 - reed_index;
	}
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

<<<<<<< HEAD
// ---- Parser ----
// ===== Parser App->PCB (commandes commencant par ':') =======================

static bool parse_u8(const char* t, uint8_t* v){ if(!t||!v) return false;
    char* e=NULL; unsigned long x=strtoul(t,&e,10); if(e==t||*e) return false;
    if(x>255UL) return false;
    *v=(uint8_t)x; return true;
}
=======
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
// ===== Parser App->PCB (commandes commencant par ':') =======================
static bool is_hex64(const char* s){ if(!s) return false; size_t n=strlen(s);
    if(n<3) return false; if(!(s[0]=='0' && (s[1]=='x'||s[1]=='X'))) return false;
    for(size_t i=2;i<n;i++){ char c=s[i];
        if(!((c>='0'&&c<='9')||(c>='a'&&c<='f')||(c>='A'&&c<='F'))) return false; }
    return true; }

static bool parse_u8(const char* t, uint8_t* v){ if(!t||!v) return false;
    char* e=NULL; unsigned long x=strtoul(t,&e,10); if(e==t||*e) return false;
    if(x>255UL) return false; *v=(uint8_t)x; return true; }

static bool parse_i(const char* t, int* v){ if(!t||!v) return false;
    char* e=NULL; long x=strtol(t,&e,10); if(e==t||*e) return false; *v=(int)x; return true; }

static bool parse_f(const char* t, float* v){ if(!t||!v) return false;
    char* e=NULL; double x=strtod(t,&e); if(e==t||*e) return false; *v=(float)x; return true; }

static bool parse_hex64(const char* t, uint64_t* v){
    if(!t||!v||!is_hex64(t)) return false;
    char* e=NULL; unsigned long long x=strtoull(t,&e,16); if(e==t||*e) return false;
    *v=(uint64_t)x; return true; }

static bool parse_bool_onoff(const char* t, bool* v){
    if(!t||!v) return false;
    if(strcmp(t,"ON")==0){*v=true; return true;}
    if(strcmp(t,"OFF")==0){*v=false; return true;}
    return false; }
>>>>>>> main

static int split_tokens_ro(char* line, char* out[], int max_out){
    // Reprend split_tokens() mais en local (version ligne mutable).
    int n=0; char* p=line;
    while(*p && isspace((unsigned char)*p)) p++;
    while(*p && n<max_out){
        out[n++] = p;
        while(*p && !isspace((unsigned char)*p)) p++;
        if(!*p) break;
        *p++ = '\0';
        while(*p && isspace((unsigned char)*p)) p++;
    }
    return n;
}

/** @brief Parse une chaîne de 2 caractères (ex: "E2") en index 0..63.
 *  @param s Chaîne "A1".."H8" (insensible à la casse pour la lettre).
 *  @param out_idx [out] Reçoit l'index 0..63 si succès.
 *  @return true si parsing réussi, false sinon.
 */
bool cb_sq_from_str(const char* s, uint8_t* out_idx){
<<<<<<< HEAD
	    if(!s || !out_idx) return false;
	    char c0 = s[0]; char c1 = s[1];
	    if(!c0 || !c1 || s[2]) return false; // exactly 2 chars like 'E2'
	    if(c0>='a'&&c0<='h') c0 = (char)(c0 - 'a' + 'A');
	    if(!(c0>='A'&&c0<='H') || !(c1>='1'&&c1<='8')) return false;
	    uint8_t file = (uint8_t)('H' - c0);
	    uint8_t rank = (uint8_t)(c1 - '1');
	    *out_idx = cb_coords_to_idx(file, rank);
	    *out_idx = convert_reed_index_to_led_index(*out_idx);
			return true;
=======
    if(!s || !out_idx) return false;
    char c0 = s[0]; char c1 = s[1];
    if(!c0 || !c1 || s[2]) return false; // exactly 2 chars like 'E2'
    if(c0>='a'&&c0<='h') c0 = (char)(c0 - 'a' + 'A');
    if(!(c0>='A'&&c0<='H') || !(c1>='1'&&c1<='8')) return false;
    uint8_t file = (uint8_t)(c0 - 'A');
    uint8_t rank = (uint8_t)(c1 - '1');
    *out_idx = cb_coords_to_idx(file, rank);
    *out_idx = convert_reed_index_to_led_index(*out_idx);
    return true;
>>>>>>> main
}

bool cb_parse_cmd(const char* line_in, cb_cmd_t* out){
    if(!line_in || !out) return false;
    memset(out, 0, sizeof(*out));
    out->type = CB_CMD_UNKNOWN;

    // Doit commencer par ':'
    if(line_in[0] != ':') return false;

    // Copie mutable
    size_t L = strnlen(line_in, CB_MAX_LINE);
    if(L >= sizeof(out->_scratch)) L = sizeof(out->_scratch)-1;
    memcpy(out->_scratch, line_in, L);
    out->_scratch[L] = '\0';

    char* tok[CB_MAX_TOKENS];
    int nt = split_tokens_ro(out->_scratch, tok, CB_MAX_TOKENS);
    if(nt <= 0) return false;

    // tok[0] commence par ':'
    const char* T0 = tok[0] + 1;

    // --- Groupe simple ---
    if(strcmp(T0,"PING")==0 && nt==1){ out->type=CB_CMD_PING; return true; }
    if(strcmp(T0,"VER?")==0 && nt==1){ out->type=CB_CMD_VER_Q; return true; }
    if(strcmp(T0,"TIME?")==0 && nt==1){ out->type=CB_CMD_TIME_Q; return true; }
    if(strcmp(T0,"RST")==0 && nt==1){ out->type=CB_CMD_RST; return true; }
    if(strcmp(T0,"SAVE")==0 && nt==1){ out->type=CB_CMD_SAVE; return true; }

<<<<<<< HEAD
    if(strcmp(T0,"WIN")==0 && nt==2){
    	out->type=CB_CMD_WIN;
    	// 0 = black / 1 = white
    	uint8_t victory;
    	parse_u8(tok[1], &victory);
    	out->u.led_set.idx = victory;
    	return true;
    }

    if(strcmp(T0,"DRAW")==0 && nt==2){
    	out->type=CB_CMD_DRAW;
    	return true;
=======
    // :STREAM ON|OFF
    if(strcmp(T0,"STREAM")==0 && nt==2){
        bool on=false; if(!parse_bool_onoff(tok[1], &on)) return false;
        out->type = CB_CMD_STREAM; out->u.stream.on = on; return true;
    }

    // --- READ ---
    if(strcmp(T0,"READ")==0 && nt>=2){
        if(strcmp(tok[1],"ALL")==0 && nt==2){ out->type=CB_CMD_READ_ALL; return true; }
        if(strcmp(tok[1],"SQ")==0 && nt==3){
            uint8_t idx; if(!cb_sq_from_str(tok[2], &idx)) return false;
            out->type=CB_CMD_READ_SQ; out->u.read_sq.idx=idx; return true;
        }
        return false;
>>>>>>> main
    }

    // --- LED ---
    if(strcmp(T0,"LED")==0 && nt>=2){
        if(strcmp(tok[1],"SET")==0 && nt==6){
            uint8_t idx,r,g,b;
            if(!cb_sq_from_str(tok[2], &idx)) return false;
            if(!parse_u8(tok[3], &r) || !parse_u8(tok[4], &g) || !parse_u8(tok[5], &b)) return false;
            out->type=CB_CMD_LED_SET; out->u.led_set.idx=idx; out->u.led_set.r=r; out->u.led_set.g=g; out->u.led_set.b=b; return true;
        }
        if(strcmp(tok[1],"OFF")==0){
            if(nt==3 && strcmp(tok[2],"ALL")==0){ out->type=CB_CMD_LED_OFF_ALL; return true; }
            if(nt==3){
                uint8_t idx; if(!cb_sq_from_str(tok[2], &idx)) return false;
                out->type=CB_CMD_LED_OFF_SQ; out->u.led_off_sq.idx=idx; return true;
            }
            return false;
        }
<<<<<<< HEAD
        if(strcmp(tok[1],"BRIGHT")==0 && nt==3){
						uint8_t br; if(!parse_u8(tok[2], &br)) return false;
						out->type=CB_CMD_LED_BRIGHT; out->u.led_bright.bright=br; return true;
				}
=======
        if(strcmp(tok[1],"FILL")==0 && nt==5){
            uint8_t r,g,b; if(!parse_u8(tok[2],&r)||!parse_u8(tok[3],&g)||!parse_u8(tok[4],&b)) return false;
            out->type=CB_CMD_LED_FILL; out->u.led_fill.r=r; out->u.led_fill.g=g; out->u.led_fill.b=b; return true;
        }
        if(strcmp(tok[1],"RECT")==0 && nt==8){
            uint8_t a,bi,r,g,bb;
            if(!cb_sq_from_str(tok[2], &a) || !cb_sq_from_str(tok[3], &bi)) return false;
            if(!parse_u8(tok[4],&r)||!parse_u8(tok[5],&g)||!parse_u8(tok[6],&bb)) return false;
            out->type=CB_CMD_LED_RECT; out->u.led_rect.from_idx=a; out->u.led_rect.to_idx=bi; out->u.led_rect.r=r; out->u.led_rect.g=g; out->u.led_rect.b=bb; return true;
        }
        if(strcmp(tok[1],"BITBOARD")==0 && nt==4){
            uint64_t bits; if(!parse_hex64(tok[2], &bits)) return false;
            out->type=CB_CMD_LED_BITBOARD; out->u.led_bitboard.bits=bits; return true;
        }
        if(strcmp(tok[1],"MOVES")==0 && nt>=5){
            // :LED MOVES <FROM> <N> <SQ1> <SQ2>...
            uint8_t from; if(!cb_sq_from_str(tok[2], &from)) return false;
            int n; if(!parse_i(tok[3], &n) || n<0 || n>64) return false;
            if(nt != 4 + n) return false;
            out->type=CB_CMD_LED_MOVES; out->u.led_moves.from_idx=from; out->u.led_moves.n_to=(uint8_t)n;
            for(int i=0;i<n;i++){
                uint8_t idx; if(!cb_sq_from_str(tok[4+i], &idx)) return false;
                out->u.led_moves.to_list[i]=idx;
            }
            return true;
        }
>>>>>>> main
        if(strcmp(tok[1],"OK")==0 && nt==4){
            uint8_t a,b; if(!cb_sq_from_str(tok[2],&a)||!cb_sq_from_str(tok[3],&b)) return false;
            out->type=CB_CMD_LED_OK; out->u.led_ok.from_idx=a; out->u.led_ok.to_idx=b; return true;
        }
<<<<<<< HEAD
=======
        if(strcmp(tok[1],"FAIL")==0 && nt==4){
            uint8_t a,b; if(!cb_sq_from_str(tok[2],&a)||!cb_sq_from_str(tok[3],&b)) return false;
            out->type=CB_CMD_LED_FAIL; out->u.led_fail.from_idx=a; out->u.led_fail.to_idx=b; return true;
        }
>>>>>>> main
        return false;
    }

    // --- COLOR ---
    if(strcmp(T0,"COLOR")==0 && nt>=2){
        if(strcmp(tok[1],"SET")==0 && nt==6){
            uint8_t r,g,b;
            if(!parse_u8(tok[3],&r)||!parse_u8(tok[4],&g)||!parse_u8(tok[5],&b)) return false;
            out->type=CB_CMD_COLOR_SET;
            strncpy(out->u.color_set.name, tok[2], CB_MAX_STR-1);
            out->u.color_set.r=r; out->u.color_set.g=g; out->u.color_set.b=b; return true;
        }
<<<<<<< HEAD
=======
        if(strcmp(tok[1],"GET")==0 && nt==3){
            out->type=CB_CMD_COLOR_GET;
            strncpy(out->u.color_get.name, tok[2], CB_MAX_STR-1);
            return true;
        }
        if(strcmp(tok[1],"?")==0 && nt==2){
            out->type=CB_CMD_COLOR_LIST_Q; return true;
        }
>>>>>>> main
        return false;
    }

    // --- CFG ---
<<<<<<< HEAD
    /*
=======
>>>>>>> main
    if(strcmp(T0,"CFG?")==0 && nt==1){ out->type=CB_CMD_CFG_Q; return true; }
    if(strcmp(T0,"CFG")==0 && nt>=3 && strcmp(tok[1],"GET")==0 && nt==3){
        out->type=CB_CMD_CFG_GET; strncpy(out->u.cfg_get.key, tok[2], CB_MAX_STR-1); return true;
    }
    if(strcmp(T0,"CFG")==0 && nt>=3 && strcmp(tok[1],"SET")==0){
        out->type=CB_CMD_CFG_SET_KV;
        int cnt = 0;
        for(int i=2;i<nt && cnt<CB_MAX_TOKENS;i++){
            // attend des tokens "KEY=VALUE"
            if(strchr(tok[i],'=')) out->u.cfg_set_kv.pairs[cnt++] = tok[i];
        }
        out->u.cfg_set_kv.n_pairs = cnt;
        return cnt>0;
    }
<<<<<<< HEAD
    */
=======
>>>>>>> main

    return false;
}


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
