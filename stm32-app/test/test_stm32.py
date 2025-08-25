#!/usr/bin/env python3
# pytest -q tests/test_uart_hello.py -s
import os
import time
import pytest

try:
    import serial
except Exception as e:
    pytest.skip(f"pyserial introuvable: {e}", allow_module_level=True)

PORT = os.getenv("UART_PORT", "/dev/ttyACM0")
BAUDRATE = int(os.getenv("UART_BAUD", "115200"))
TARGET = os.getenv("UART_TARGET", "Hello World !")
IDLE_TIMEOUT_S = float(os.getenv("UART_IDLE_TIMEOUT", "10.0"))  # délai d'inactivité
READ_CHUNK = 256
SER_TIMEOUT = 0.2  # timeout non-bloquant pour ser.read()

@pytest.fixture(scope="function")
def ser():
    """Ouvre le port série et purge les buffers. Skip si non disponible."""
    try:
        s = serial.Serial(PORT, BAUDRATE, timeout=SER_TIMEOUT)
    except serial.SerialException as e:
        pytest.skip(f"Port série indisponible ({PORT} @ {BAUDRATE}): {e}")
    # purge pour éviter l'accumulation d'anciennes trames
    s.reset_input_buffer()
    s.reset_output_buffer()
    yield s
    try:
        s.close()
    except Exception:
        pass

def _read_until_line(ser, target: str, idle_timeout_s: float):
    """
    Lit en flux, découpe sur LF, tolère CR, et renvoie True si `target` est reçu
    avant `idle_timeout_s` d'inactivité. Retourne (ok, log_bytes, seen_lines)
    pour aider au debug en cas d'échec.
    """
    buf = bytearray()
    last_rx = time.monotonic()
    log_bytes = bytearray()
    seen_lines = []

    while True:
        chunk = ser.read(READ_CHUNK)
        now = time.monotonic()
        if chunk:
            last_rx = now
            log_bytes.extend(chunk)
            buf.extend(chunk)

            # split sur '\n' et conserver le morceau partiel en fin
            *complete, remainder = bytes(buf).split(b"\n")
            buf = bytearray(remainder)

            for raw in complete:
                # tolère CRLF/espaces; ignore erreurs d'UTF-8
                line = raw.rstrip(b"\r").decode(errors="ignore").strip()
                if line:
                    seen_lines.append(line)
                    if line == target:
                        return True, bytes(log_bytes), seen_lines

        if (now - last_rx) > idle_timeout_s:
            return False, bytes(log_bytes), seen_lines

@pytest.mark.serial
def test_uart_hello_world(ser):
    """
    Vérifie que le firmware émet exactement `TARGET` (par défaut 'Hello World !')
    sur une ligne (idéalement terminée par \\r\\n) dans la fenêtre d'inactivité
    `IDLE_TIMEOUT_S`. Le test est robuste aux accumulations et aux CR/LF.
    """
    ok, raw, lines = _read_until_line(ser, TARGET, IDLE_TIMEOUT_S)

    if not ok:
        # Aide au diagnostic : dump des octets reçus et des lignes décodées
        hex_preview = raw[:512].hex(" ")
        pytest.fail(
            "Message attendu non reçu dans la fenêtre d'inactivité.\n"
            f"  PORT       : {PORT}\n"
            f"  BAUDRATE   : {BAUDRATE}\n"
            f"  TARGET     : {TARGET!r}\n"
            f"  LINES SEEN : {lines}\n"
            f"  RAW BYTES  : {len(raw)} octets (aperçu 512B hex)\n"
            f"  HEX PREVIEW: {hex_preview}"
        )

    # Si on arrive ici, c'est reçu
    assert ok
