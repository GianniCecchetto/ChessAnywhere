#!/usr/bin/env python3
# tests/test_uart_hello.py
# pytest -q -k test_uart -s
import time
import pytest

try:
    import serial
except Exception as e:
    pytest.skip(f"pyserial introuvable: {e}", allow_module_level=True)

PORT = "/dev/ttyACM0"
BAUDRATE = 115200
TARGET = "Hello World !"        # attendu côté STM32
IDLE_TIMEOUT_S = 10.0           # délai d'inactivité max
READ_CHUNK = 256
SER_TIMEOUT = 0.2               # lecture non bloquante

def test_uart():
    """Teste la réception exacte de 'Hello World !' sur une ligne UART."""
    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=SER_TIMEOUT)
    except serial.SerialException as e:
        pytest.skip(f"Port série indisponible ({PORT} @ {BAUDRATE}): {e}")

    # Purge pour éviter les anciennes trames
    ser.reset_input_buffer()
    ser.reset_output_buffer()

    buf = bytearray()
    raw_log = bytearray()
    last_rx = time.monotonic()
    ok = False
    seen_lines = []

    try:
        while True:
            chunk = ser.read(READ_CHUNK)
            now = time.monotonic()

            if chunk:
                last_rx = now
                raw_log.extend(chunk)
                buf.extend(chunk)

                # Split sur LF, garder le reste partiel
                *complete, remainder = bytes(buf).split(b"\n")
                buf = bytearray(remainder)

                for raw in complete:
                    # Tolère CRLF, ignore erreurs UTF-8 et espaces périphériques
                    line = raw.rstrip(b"\r").decode(errors="ignore").strip()
                    if line:
                        seen_lines.append(line)
                        if line == TARGET:
                            ok = True
                            print("[INFO] Reçu:", line)
                            break

            if ok:
                break

            if (now - last_rx) > IDLE_TIMEOUT_S:
                break
    finally:
        try:
            ser.close()
        except Exception:
            pass

    if not ok:
        hex_preview = raw_log[:512].hex(" ")
        pytest.fail(
            "Message attendu non reçu avant le timeout d'inactivité.\n"
            f"  PORT       : {PORT}\n"
            f"  BAUDRATE   : {BAUDRATE}\n"
            f"  TARGET     : {TARGET!r}\n"
            f"  LINES SEEN : {seen_lines}\n"
            f"  RAW BYTES  : {len(raw_log)} (aperçu 512B)\n"
            f"  HEX PREVIEW: {hex_preview}"
        )
