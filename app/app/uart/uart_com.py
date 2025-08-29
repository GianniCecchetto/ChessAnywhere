import serial
import sys
import os
import threading
import queue
import time

BASE_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
UART_PATH = os.path.join(BASE_DIR, "lib", "uart_fmt", "python_doc")
sys.path.append(UART_PATH)

import board_com_ctypes as cb

# --- Configuration du port COM ---
try:
    ser = serial.Serial(
        port="COM7",
        baudrate=115200,
        bytesize=serial.EIGHTBITS,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        timeout=0.1
    )
except serial.SerialException as e:
    print(f"Erreur de connexion au port série: {e}. Le programme continuera sans UART.")
    ser = None

# Queue pour stocker les événements reçus
uart_queue = queue.Queue()

# Thread de lecture UART
def uart_reader():
    if not ser:
        return
    while True:
        try:
            if ser.is_open:
                line = ser.readline()
                if line:
                    line = line.decode("ascii", errors="ignore").strip()
                    parsed = cb.parse_line(line)
                    if parsed:
                        uart_queue.put(parsed)
        except Exception as e:
            print(f"Erreur de lecture UART: {e}")
        time.sleep(0.01)

threading.Thread(target=uart_reader, daemon=True).start()

# Envoi de commande
def send_command(cmd: str):
    print(f"[TX] {cmd}")
    if ser and ser.is_open:
        try:
            ser.write((cmd + "\n").encode("ascii"))
        except Exception as e:
            print(f"Erreur d'écriture UART: {e}")
    else:
        print("Port série non ouvert ou déconnecté. Commande non envoyée.")

# Lecture non bloquante des événements
def get_next_event(event_type=None):
    try:
        while True:
            parsed = uart_queue.get_nowait()
            print(f"[RX] {parsed}")
            if event_type is None or parsed.get("type") == event_type:
                return parsed
    except queue.Empty:
        return None
