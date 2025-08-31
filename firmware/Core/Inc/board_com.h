// =============================================
// FILE: board_com.h
// Minimal C library to format commands (App->PCB),
// assemble lines from UART, and parse events/responses (PCB->App)
// for the ASCII protocol defined in the README.
// =============================================
#ifndef CB_PROTO_H
#define CB_PROTO_H

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

// -------- Constants --------
#define CB_MAX_LINE         32   // max single line (without CRLF)
#define CB_MAX_TOKENS       24
#define CB_MAX_STR          26
#define CEILING_POS(X) ((X-(int)(X)) > 0 ? (int)(X+1) : (int)(X))

// -------- Square helpers --------
// Index mapping: 0..63 with bit0=A1, bit63=H8

/** @brief Convertit (file,rank) -> index 0..63 (A1=0, H8=63).
 *  @param file Colonne 0..7 correspondant à A..H.
 *  @param rank Rang 0..7 correspondant à 1..8.
 *  @return Index linéaire 0..63.
 */
static inline uint8_t cb_coords_to_idx(uint8_t file/*0=A..7=H*/, uint8_t rank/*0=1..7=8*/){
    return (uint8_t)(rank*8u + file);
}

/** @brief Convertit un index 0..63 en coordonnées (file, rank).
 *  @param idx  Index 0..63.
 *  @param file [out] Reçoit 0..7 (A..H).
 *  @param rank [out] Reçoit 0..7 (1..8).
 */
static inline void cb_idx_to_coords(uint8_t idx, uint8_t* file, uint8_t* rank){
    *file = (uint8_t)(idx % 8u);
    *rank = (uint8_t)(idx / 8u);
}

bool cb_sq_from_str(const char* s, uint8_t* out_idx);

/** @brief Convertit un index 0..63 en chaîne "A1".."H8".
 *  @param idx Index 0..63.
 *  @param out Buffer de 3 chars min. (ex: "E2\0").
 */
static inline void cb_sq_to_str(uint8_t idx, char out[3]){
    uint8_t f,r; cb_idx_to_coords(idx,&f,&r);
    out[0] = (char)('H'-f); out[1]=(char)('1'+r); out[2]='\0';
}

// -------- Parser côté PCB (App -> PCB) --------

typedef enum {
    CB_CMD_NONE = 0,
    CB_CMD_PING,
    CB_CMD_VER_Q,
    CB_CMD_TIME_Q,
    CB_CMD_RST,
    CB_CMD_SAVE,
    CB_CMD_STREAM,           // on: bool
    CB_CMD_READ_ALL,
    CB_CMD_READ_SQ,          // idx
    CB_CMD_READ_MASK_SET,    // mask64
    CB_CMD_READ_MASK_Q,

		CB_CMD_WIN,
		CB_CMD_DRAW,

    CB_CMD_LED_SET,          // idx, r,g,b
    CB_CMD_LED_OFF_SQ,       // idx
    CB_CMD_LED_OFF_ALL,
    CB_CMD_LED_FILL,         // r,g,b
    CB_CMD_LED_RECT,         // from_idx, to_idx, r,g,b
    CB_CMD_LED_BITBOARD,     // bits64
    CB_CMD_LED_BRIGHT,       // bright 0..255
    CB_CMD_LED_GAMMA,        // float gamma
    CB_CMD_LED_MAP_HEX,      // hex192[385] (192 bytes hex, +NUL)

    CB_CMD_COLOR_SET,        // name, r,g,b
    CB_CMD_COLOR_GET,        // name
    CB_CMD_COLOR_LIST_Q,     // (COLOR?)

    CB_CMD_LED_MOVES,        // from_idx, n_to, to_list[0..n-1]  (n<=64)
    CB_CMD_LED_OK,           // from_idx, to_idx
    CB_CMD_LED_FAIL,         // from_idx, to_idx

    CB_CMD_MOVE_ACK,         // from_idx, to_idx, promo(0/Q/R/B/N), castle(0/K/Q), enpassant_idx(-1..63)
    CB_CMD_MOVE_NACK,        // from_idx, to_idx, reason (string)

    CB_CMD_CFG_Q,
    CB_CMD_CFG_GET,          // key
    CB_CMD_CFG_SET_KV,       // list of "KEY=VALUE" pairs (count, array)

    CB_CMD_UNKNOWN
} cb_cmd_type_t;

typedef struct {
    cb_cmd_type_t type;
    union {
        struct { bool on; } stream;

        struct { uint8_t idx; } read_sq;

        struct { uint8_t idx, r,g,b; } led_set;
        struct { uint8_t idx; } led_off_sq;
        struct { uint8_t r,g,b; } led_fill;
        struct { uint8_t from_idx, to_idx, r,g,b; } led_rect;
        struct { uint64_t bits; } led_bitboard;

        struct { char name[CB_MAX_STR]; uint8_t r,g,b; } color_set;
        struct { char name[CB_MAX_STR]; } color_get;

        struct { uint8_t from_idx; uint8_t n_to; uint8_t to_list[64]; } led_moves;
        struct { uint8_t from_idx, to_idx; } led_ok;
        struct { uint8_t from_idx, to_idx; } led_fail;

        struct { uint8_t from_idx, to_idx; char reason[CB_MAX_STR]; } move_nack;

        struct { char key[CB_MAX_STR]; } cfg_get;
        struct { int n_pairs; const char* pairs[CB_MAX_TOKENS]; } cfg_set_kv; // pointers into internal buffer

    } u;

    // Espace interne pour stocker une copie modifiable de la ligne (tokenisation)
    // et les paires k=v de CFG SET (référencées par u.cfg_set_kv.pairs).
    char _scratch[CB_MAX_LINE + 2];
} cb_cmd_t;

/** @brief Parse une ligne de commande (App->PCB) commençant par ':'.
 *  Ex: ":LED SET E2 255 0 0"
 *  Remplit @p out->type et l'union associée.
 *  @return true si reconnu, false sinon (type=CB_CMD_UNKNOWN).
 */
bool cb_parse_cmd(const char* line, cb_cmd_t* out);


// -------- Message types --------
typedef enum {
    CB_MSG_NONE = 0,
    // Events (PCB->App)
    CB_EVT_BOOT,
    CB_EVT_LIFT,
    CB_EVT_PLACE,
    CB_EVT_MOVE,
    CB_EVT_LIFT_CANCEL,
    CB_EVT_BTN,
    CB_EVT_TIMEOUT,
    CB_EVT_ERROR,
    // Responses (PCB->App)
    CB_RSP_OK,
    CB_RSP_ERR,
    CB_RSP_UNKNOWN
} cb_msg_type_t;

// Parsed message
typedef struct {
    cb_msg_type_t type;
    union {
        struct { char fw[CB_MAX_STR]; char hw[CB_MAX_STR]; uint32_t t_ms; } boot;
        struct { uint8_t idx; uint32_t t_ms; } sq;          // LIFT/PLACE
        struct { uint8_t from_idx, to_idx; uint32_t t_ms; } move;
        struct { char state[CB_MAX_STR]; uint32_t t_ms; } timeout;
        struct { char code[CB_MAX_STR]; char details[CB_MAX_STR]; uint32_t t_ms; } err_evt; // EVT ERROR
        struct { char text[CB_MAX_STR]; } ok;               // whole OK payload as text
        struct { char code[CB_MAX_STR]; char details[CB_MAX_STR]; } err; // ERR reply
    } u;
} cb_msg_t;

// -------- UART line assembler --------
// Feed raw bytes from UART; every time a full line (\r?\n) is detected,
// your callback is invoked with a zero-terminated line (without CR/LF).

/** @brief Prototype de callback appelé à chaque ligne complète reçue.
 *  @param line Ligne ASCII zéro-terminée (sans CR/LF).
 *  @param user Pointeur utilisateur transmis inchangé.
 */
typedef void (*cb_on_line_fn)(const char* line, void* user);

typedef struct {
    char buf[CB_MAX_LINE+2];
    size_t len; // number of bytes currently in buf (no CRLF)
} cb_linebuf_t;

/** @brief Initialise le tampon d’assemblage de lignes UART.
 *  @param lb Contexte de line-buffer à initialiser.
 */
void cb_linebuf_init(cb_linebuf_t* lb);

/** @brief Alimente le line-buffer avec des octets UART et déclenche le callback
 *         dès qu’une fin de ligne est détectée (LF). CR est ignoré.
 *  @param lb       Contexte de line-buffer.
 *  @param data     Octets reçus (brut UART).
 *  @param n        Taille du buffer @p data.
 *  @param on_line  Callback appelé pour chaque ligne complète (sans CR/LF).
 *  @param user     Contexte utilisateur passé au callback.
 */
void cb_linebuf_feed(cb_linebuf_t* lb, const uint8_t* data, size_t n, cb_on_line_fn on_line, void* user);

// -------- Parser --------
// Parse a single ASCII line into a structured cb_msg_t.
// Returns true on success; false if unrecognized (type=CB_RSP_UNKNOWN).

/** @brief Parse une ligne ASCII du protocole en structure typée.
 *  @param line Ligne ASCII zéro-terminée (sans CR/LF).
 *  @param out  [out] Message décodé (type + payload).
 *  @return true si reconnu et parsé, false sinon (type=CB_RSP_UNKNOWN).
 */
bool cb_parse_line(const char* line, cb_msg_t* out);

uint8_t convert_reed_index_to_led_index(uint8_t reed_index);

// -------- Formatters (App->PCB) --------
// Each formatter writes a zero-terminated command into out[0..cap-1]
// and returns the byte length (excluding the implicit CRLF you will add when sending).

/** @brief Formate la commande :PING. */
size_t cb_fmt_ping(char* out, size_t cap);
/** @brief Formate la requête :VER? (version FW/HW). */
size_t cb_fmt_ver_q(char* out, size_t cap);
/** @brief Formate la requête :TIME? (uptime en ms). */
size_t cb_fmt_time_q(char* out, size_t cap);
/** @brief Formate la commande :RST (reset logiciel). */
size_t cb_fmt_rst(char* out, size_t cap);
/** @brief Formate la commande :SAVE (persiste la configuration). */
size_t cb_fmt_save(char* out, size_t cap);
/** @brief Formate :STREAM ON/OFF (active/désactive l’émission d’EVT).
 *  @param on true->ON, false->OFF.
 */
size_t cb_fmt_stream(char* out, size_t cap, bool on);

/** @brief Formate :READ ALL (bitboard complet des reeds). */
size_t cb_fmt_read_all(char* out, size_t cap);
/** @brief Formate :READ SQ <SQ> (état d’une case).
 *  @param idx Index 0..63 de la case.
 */
size_t cb_fmt_read_sq(char* out, size_t cap, uint8_t idx);
/** @brief Formate :READ MASK SET 0x<64bits> (ignore events mask).
 *  @param mask Bit=1 pour ignorer les EVT de la case.
 */
size_t cb_fmt_read_mask_set(char* out, size_t cap, uint64_t mask);
/** @brief Formate :READ MASK? (interroge le masque courant). */
size_t cb_fmt_read_mask_q(char* out, size_t cap);

/** @brief Formate :LED SET <SQ> R G B (allume une case en RGB). */
size_t cb_fmt_led_set(char* out, size_t cap, uint8_t idx, uint8_t r, uint8_t g, uint8_t b);
/** @brief Formate :LED OFF <SQ> (éteint une case). */
size_t cb_fmt_led_off_sq(char* out, size_t cap, uint8_t idx);
/** @brief Formate :LED OFF ALL (éteint tout le plateau). */
size_t cb_fmt_led_off_all(char* out, size_t cap);
/** @brief Formate :LED FILL R G B (remplit tout le plateau). */
size_t cb_fmt_led_fill(char* out, size_t cap, uint8_t r, uint8_t g, uint8_t b);
/** @brief Formate :LED RECT <FROM> <TO> R G B (bloc rectangulaire). */
size_t cb_fmt_led_rect(char* out, size_t cap, uint8_t from_idx, uint8_t to_idx, uint8_t r, uint8_t g, uint8_t b);
/** @brief Formate :LED BITBOARD 0x<64bits> (allume bits=1 avec COLOR.HINT). */
size_t cb_fmt_led_bitboard(char* out, size_t cap, uint64_t bits);
/** @brief Formate :LED BRIGHT <0..255> (luminosité globale). */
size_t cb_fmt_led_bright(char* out, size_t cap, uint8_t bright);
/** @brief Formate :LED GAMMA <float> (gamma d’affichage). */
size_t cb_fmt_led_gamma(char* out, size_t cap, float gamma);
/** @brief Formate :LED MAP <hex192> (frame complète 64×RGB).
 *  @param hex192 192 octets hex concaténés (A1..H8).
 */
size_t cb_fmt_led_map_hex(char* out, size_t cap, const char* hex192);

/** @brief Formate :COLOR SET <NAME> R G B (définit une couleur nommée). */
size_t cb_fmt_color_set(char* out, size_t cap, const char* name, uint8_t r, uint8_t g, uint8_t b);
/** @brief Formate :COLOR GET <NAME> (lit une couleur nommée). */
size_t cb_fmt_color_get(char* out, size_t cap, const char* name);
/** @brief Formate :COLOR? (liste toutes les couleurs). */
size_t cb_fmt_color_list_q(char* out, size_t cap);

/** @brief Formate :LED MOVES <FROM> <N> <SQ1>.. (surligne coups légaux).
 *  @param from_idx Case source.
 *  @param to_list  Tableau d’index destinations.
 *  @param n        Nombre d’entrées dans @p to_list.
 */
size_t cb_fmt_led_moves(char* out, size_t cap, uint8_t from_idx, const uint8_t* to_list, size_t n);
/** @brief Formate :LED OK <FROM> <TO> (animation de validation). */
size_t cb_fmt_led_ok(char* out, size_t cap, uint8_t from_idx, uint8_t to_idx);
/** @brief Formate :LED FAIL <FROM> <TO> (animation d’erreur). */
size_t cb_fmt_led_fail(char* out, size_t cap, uint8_t from_idx, uint8_t to_idx);

/** @brief Formate :MOVE ACK <FROM> <TO> [PROMO=..] [CASTLE=..] [ENPASSANT=..].
 *  @param promo    'Q','R','B','N' ou 0 si non applicable.
 *  @param castle   'K','Q' ou 0 si non applicable.
 *  @param enpassant_idx Index >=0 pour la case EP, ou -1 si absent.
 */
size_t cb_fmt_move_ack(char* out, size_t cap, uint8_t from_idx, uint8_t to_idx,
                       char promo/*'Q','R','B','N' or 0*/, char castle/*'K','Q' or 0*/, int enpassant_idx/*-1 if none*/);
/** @brief Formate :MOVE NACK <FROM> <TO> [REASON=txt] (refuse un coup).
 *  @param reason Texte optionnel de diagnostic (peut être NULL).
 */
size_t cb_fmt_move_nack(char* out, size_t cap, uint8_t from_idx, uint8_t to_idx, const char* reason/*nullable*/);

/** @brief Formate :CFG? (dump de la configuration). */
size_t cb_fmt_cfg_q(char* out, size_t cap);
/** @brief Formate :CFG GET <KEY> (lecture d’une clé).
 *  @param key Nom de la clé (ex: "DEBOUNCE_REED").
 */
size_t cb_fmt_cfg_get(char* out, size_t cap, const char* key);
/** @brief Formate :CFG SET k1=v1 [k2=v2 ...] (écriture de clés arbitraires).
 *  @param kv_pairs Tableau de chaînes "KEY=VALUE".
 *  @param n_pairs  Nombre d’entrées.
 */
size_t cb_fmt_cfg_set_kv(char* out, size_t cap, const char* const* kv_pairs, size_t n_pairs);
/** @brief Formate :CFG SET DEBOUNCE_REED=.. DEBOUNCE_BTN=.. */
size_t cb_fmt_cfg_set_debounce(char* out, size_t cap, uint16_t reed_ms, uint16_t btn_ms);
/** @brief Formate :CFG SET ORIENT=<0|90|180|270> */
size_t cb_fmt_cfg_set_orient(char* out, size_t cap, int orient_deg);
/** @brief Formate :CFG SET BAUD=<bps> (effet après :RST). */
size_t cb_fmt_cfg_set_baud(char* out, size_t cap, uint32_t baud);

// -------- Event formatters (Board->App) --------
// Produisent des lignes "EVT ..." compatibles cb_parse_line (terminées CRLF).

/** EVT BOOT FW=... HW=... t=...  (FW/HW optionnels) */
size_t cb_fmt_evt_boot(char* out, size_t cap, const char* fw/*nullable*/, const char* hw/*nullable*/, uint32_t t_ms);

/** EVT LIFT <SQ> t=... */
size_t cb_fmt_evt_lift(char* out, size_t cap, uint8_t idx, uint32_t t_ms);

/** EVT PLACE <SQ> t=... */
size_t cb_fmt_evt_place(char* out, size_t cap, uint8_t idx, uint32_t t_ms);

/** EVT MOVE <FROM> <TO> t=... */
size_t cb_fmt_evt_move(char* out, size_t cap, uint8_t from_idx, uint8_t to_idx, uint32_t t_ms);

/** EVT LIFT_CANCEL <SQ> t=... */
size_t cb_fmt_evt_lift_cancel(char* out, size_t cap, uint8_t idx, uint32_t t_ms);

/** EVT BTN t=... */
size_t cb_fmt_evt_btn(char* out, size_t cap, uint32_t t_ms);

/** EVT TIMEOUT <STATE> t=...
 *  NOTE: <STATE> doit être un token sans espace (ex: "WAIT_MOVE"). */
size_t cb_fmt_evt_timeout(char* out, size_t cap, const char* state, uint32_t t_ms);

/** EVT ERROR <CODE> <DETAILS...> t=...
 *  Si details==NULL, n'écrit pas la partie détails. */
size_t cb_fmt_evt_error(char* out, size_t cap, const char* code, const char* details/*nullable*/, uint32_t t_ms);


#ifdef __cplusplus
}
#endif

#endif // CB_PROTO_H
