import serial
import sys
import os

BASE_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
print(BASE_DIR)
UART_PATH = os.path.join(BASE_DIR, "lib", "uart_fmt", "python_doc")
print(UART_PATH)
sys.path.append(UART_PATH)

import board_com_ctypes as cb

# === Configuration du port COM ===
"""ser = serial.Serial(
    port="COM3",  # sous Linux typiquement "/dev/ttyUSB0" ou "/dev/ttyAMA0"
    baudrate=115200,
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    timeout=1
)
"""
def send_command(cmd: str):
    print(f"[TX] {cmd}")
    """
    if ser.is_open:
        ser.write((cmd + "\n").encode("ascii"))
        print(f"[TX] {cmd}")
    else:
        print("Port série non ouvert")"""

def read_responses():
    """
    if not ser.is_open:
        print("Port série non ouvert")
        return []

    responses = []
    while True:
        line = ser.readline()
        if not line:
            break
        line = line.decode("ascii", errors="ignore").strip()
        if line:
            parsed = cb.parse_line(line)
            print(f"[RX] {line} → {parsed}")
            responses.append(parsed or {"raw": line})
    return responses"""
