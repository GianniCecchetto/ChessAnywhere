#!/usr/bin/env python3
# pytest -q -k test_uart -s
import time
import pytest

try:
    import serial
except Exception as e:
    pytest.skip(f"pyserial introuvable: {e}", allow_module_level=True)

PORT = "/dev/ttyACM0"
BAUDRATE = 115200
TARGET = "Hello World !"        # attendu
READ_CHUNK = 256
SER_TIMEOUT = 0.2
GLOBAL_TIMEOUT = 10.0           # délai max en secondes

def test_uart():
    """Teste la réception exacte de 'Hello World !' sur l'UART avec un timeout global."""
    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=SER_TIMEOUT)
    except serial.SerialException as e:
        pytest.skip(f"Port série indisponible ({PORT} @ {BAUDRATE}): {e}")

    ser.reset_input_buffer()
    ser.reset_output_buffer()

    buf = bytearray()
    raw_log = bytearray()
    seen_lines = []

    start_time = time.monotonic()
    ok = False

    try:
        while (time.monotonic() - start_time) < GLOBAL_TIMEOUT:
            chunk = ser.read(READ_CHUNK)
            if chunk:
                raw_log.extend(chunk)
                buf.extend(chunk)

                *complete, remainder = bytes(buf).split(b"\n")
                buf = bytearray(remainder)

                for raw in complete:
                    line = raw.rstrip(b"\r").decode(errors="ignore").strip()
                    if line:
                        seen_lines.append(line)
                        if line == TARGET:
                            ok = True
                            print("[INFO] Reçu:", line)
                            break
            if ok:
                break
    finally:
        try:
            ser.close()
        except Exception:
            pass

    if not ok:
        hex_preview = raw_log[:256].hex(" ")
        pytest.fail(
            f"Message '{TARGET}' non reçu dans les {GLOBAL_TIMEOUT}s.\n"
            f"PORT       : {PORT}\n"
            f"BAUDRATE   : {BAUDRATE}\n"
            f"LINES SEEN : {seen_lines}\n"
            f"RAW BYTES  : {len(raw_log)}\n"
            f"HEX PREVIEW: {hex_preview}"
        )
