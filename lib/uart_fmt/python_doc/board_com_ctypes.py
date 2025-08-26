# board_com_ctypes.py
from __future__ import annotations
import os
import sys
from typing import Optional
from ctypes import (
    CDLL, c_bool, c_char, c_char_p, c_float, c_int, c_size_t, c_uint8,
    c_uint16, c_uint32, c_uint64, c_void_p, Structure, Union, POINTER, CFUNCTYPE,
    create_string_buffer, byref
)

# ---------- Localisation de la lib ----------
def _default_libname():
    if sys.platform.startswith("win"):
        return "../src/build/board_com.dll"
    elif sys.platform == "darwin":
        return "libboard_com.dylib"
    else:
        return "libboard_com.so"

# Priorité à la variable d'env BOARD_COM_LIB, sinon CBPROTO_LIB (compat), sinon défaut
_LIBNAME = os.environ.get("BOARD_COM_LIB") or os.environ.get("CBPROTO_LIB") or _default_libname()
_lib = CDLL(_LIBNAME)

# ---------- Constantes (doivent matcher board_com.h) ----------
CB_MAX_LINE   = 256
CB_MAX_TOKENS = 24
CB_MAX_STR    = 96

# Types d’enum (cb_msg_type_t) — on les expose en int :
CB_MSG_NONE       = 0
CB_EVT_BOOT       = 1
CB_EVT_LIFT       = 2
CB_EVT_PLACE      = 3
CB_EVT_MOVE       = 4
CB_EVT_LIFT_CANCEL= 5
CB_EVT_BTN        = 6
CB_EVT_TIMEOUT    = 7
CB_EVT_ERROR      = 8
CB_RSP_OK         = 9
CB_RSP_ERR        = 10
CB_RSP_UNKNOWN    = 11

# ---------- Structures C mappées en ctypes ----------
class cb_linebuf_t(Structure):
    _fields_ = [
        ("buf", c_char * (CB_MAX_LINE + 2)),
        ("len", c_size_t),
    ]

class _boot_t(Structure):
    _fields_ = [("fw", c_char * CB_MAX_STR), ("hw", c_char * CB_MAX_STR), ("t_ms", c_uint32)]

class _sq_t(Structure):
    _fields_ = [("idx", c_uint8), ("t_ms", c_uint32)]

class _move_t(Structure):
    _fields_ = [("from_idx", c_uint8), ("to_idx", c_uint8), ("t_ms", c_uint32)]

class _timeout_t(Structure):
    _fields_ = [("state", c_char * CB_MAX_STR), ("t_ms", c_uint32)]

class _err_evt_t(Structure):
    _fields_ = [("code", c_char * CB_MAX_STR), ("details", c_char * CB_MAX_STR), ("t_ms", c_uint32)]

class _ok_t(Structure):
    _fields_ = [("text", c_char * CB_MAX_STR)]

class _err_t(Structure):
    _fields_ = [("code", c_char * CB_MAX_STR), ("details", c_char * CB_MAX_STR)]

class _u_t(Union):
    _fields_ = [
        ("boot", _boot_t),
        ("sq", _sq_t),
        ("move", _move_t),
        ("timeout", _timeout_t),
        ("err_evt", _err_evt_t),
        ("ok", _ok_t),
        ("err", _err_t),
    ]

class cb_msg_t(Structure):
    _fields_ = [("type", c_int), ("u", _u_t)]

# ---------- Signatures des fonctions C ----------
_lib.cb_linebuf_init.argtypes = [POINTER(cb_linebuf_t)]
_lib.cb_linebuf_init.restype  = None

_cb_on_line_fn = CFUNCTYPE(None, c_char_p, c_void_p)

_lib.cb_linebuf_feed.argtypes = [
    POINTER(cb_linebuf_t), POINTER(c_uint8), c_size_t, _cb_on_line_fn, c_void_p
]
_lib.cb_linebuf_feed.restype = None

_lib.cb_parse_line.argtypes = [c_char_p, POINTER(cb_msg_t)]
_lib.cb_parse_line.restype  = c_bool

def _decl_fmt(name, argtypes, restype=c_size_t):
    fn = getattr(_lib, name)
    fn.argtypes = argtypes
    fn.restype  = restype
    return fn

cb_fmt_ping         = _decl_fmt("cb_fmt_ping",        [c_char_p, c_size_t])
cb_fmt_ver_q        = _decl_fmt("cb_fmt_ver_q",       [c_char_p, c_size_t])
cb_fmt_time_q       = _decl_fmt("cb_fmt_time_q",      [c_char_p, c_size_t])
cb_fmt_rst          = _decl_fmt("cb_fmt_rst",         [c_char_p, c_size_t])
cb_fmt_save         = _decl_fmt("cb_fmt_save",        [c_char_p, c_size_t])
cb_fmt_stream       = _decl_fmt("cb_fmt_stream",      [c_char_p, c_size_t, c_bool])

cb_fmt_read_all     = _decl_fmt("cb_fmt_read_all",    [c_char_p, c_size_t])
cb_fmt_read_sq      = _decl_fmt("cb_fmt_read_sq",     [c_char_p, c_size_t, c_uint8])
cb_fmt_read_mask_set= _decl_fmt("cb_fmt_read_mask_set",[c_char_p, c_size_t, c_uint64])
cb_fmt_read_mask_q  = _decl_fmt("cb_fmt_read_mask_q", [c_char_p, c_size_t])

cb_fmt_led_set      = _decl_fmt("cb_fmt_led_set",     [c_char_p, c_size_t, c_uint8, c_uint8, c_uint8, c_uint8])
cb_fmt_led_off_sq   = _decl_fmt("cb_fmt_led_off_sq",  [c_char_p, c_size_t, c_uint8])
cb_fmt_led_off_all  = _decl_fmt("cb_fmt_led_off_all", [c_char_p, c_size_t])
cb_fmt_led_fill     = _decl_fmt("cb_fmt_led_fill",    [c_char_p, c_size_t, c_uint8, c_uint8, c_uint8])
cb_fmt_led_bitboard = _decl_fmt("cb_fmt_led_bitboard",[c_char_p, c_size_t, c_uint64])
cb_fmt_led_bright   = _decl_fmt("cb_fmt_led_bright",  [c_char_p, c_size_t, c_uint8])
cb_fmt_led_gamma    = _decl_fmt("cb_fmt_led_gamma",   [c_char_p, c_size_t, c_float])

# ---------- Helpers Python ----------
def _fmt_wrapper(fmt_fn, *args) -> str:
    buf = create_string_buffer(CB_MAX_LINE)
    n = fmt_fn(buf, c_size_t(len(buf)), *args)
    return buf.raw[:n].decode("ascii", errors="ignore")

def fmt_led_set(idx: int, r: int, g: int, b: int) -> str:
    return _fmt_wrapper(cb_fmt_led_set, c_uint8(idx), c_uint8(r), c_uint8(g), c_uint8(b))

def fmt_ping() -> str:
    return _fmt_wrapper(cb_fmt_ping)

def fmt_stream(on: bool) -> str:
    return _fmt_wrapper(cb_fmt_stream, c_bool(on))

class LineBuffer:
    """Wrap cb_linebuf_t avec un callback collecteur de lignes."""
    def __init__(self):
        self._lb = cb_linebuf_t()
        _lib.cb_linebuf_init(byref(self._lb))
        self._lines = []
        self._cb = _cb_on_line_fn(self._on_line)

    def _on_line(self, c_line: bytes, user: c_void_p):
        try:
            self._lines.append(c_line.decode("ascii", errors="ignore"))
        except Exception:
            self._lines.append("")

    def feed(self, data: bytes):
        # alloue un buffer C pour les octets
        arr = (c_uint8 * len(data)).from_buffer_copy(data)
        _lib.cb_linebuf_feed(byref(self._lb), arr, c_size_t(len(data)), self._cb, None)

    def pop_all(self):
        out, self._lines = self._lines, []
        return out

def _strip0(b: bytes) -> str:
    return b.split(b"\x00", 1)[0].decode(errors="ignore")

def parse_line(line: str) -> Optional[dict]:
    m = cb_msg_t()
    ok = _lib.cb_parse_line(line.encode("ascii"), byref(m))
    if not ok:
        return None
    t = m.type
    d: dict = {"type": t}
    if t == CB_EVT_BOOT:
        d |= {"fw": _strip0(m.u.boot.fw), "hw": _strip0(m.u.boot.hw), "t_ms": m.u.boot.t_ms}
    elif t in (CB_EVT_LIFT, CB_EVT_PLACE, CB_EVT_LIFT_CANCEL, CB_EVT_BTN):
        d |= {"idx": getattr(m.u.sq, "idx", 0), "t_ms": getattr(m.u.sq, "t_ms", 0)}
    elif t == CB_EVT_MOVE:
        d |= {"from_idx": m.u.move.from_idx, "to_idx": m.u.move.to_idx, "t_ms": m.u.move.t_ms}
    elif t == CB_EVT_TIMEOUT:
        d |= {"state": _strip0(m.u.timeout.state), "t_ms": m.u.timeout.t_ms}
    elif t == CB_EVT_ERROR:
        d |= {"code": _strip0(m.u.err_evt.code), "details": _strip0(m.u.err_evt.details), "t_ms": m.u.err_evt.t_ms}
    elif t == CB_RSP_OK:
        d |= {"ok": _strip0(m.u.ok.text)}
    elif t == CB_RSP_ERR:
        d |= {"code": _strip0(m.u.err.code), "details": _strip0(m.u.err.details)}
    return d
