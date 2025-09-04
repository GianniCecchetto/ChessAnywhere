import serial
import serial.tools.list_ports
import sys
import os
import threading
import queue
import time

BASE_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
UART_PATH = os.path.join(BASE_DIR, "lib", "uart_fmt", "python_doc")
sys.path.append(UART_PATH)

from lib.uart_fmt.python_doc import board_com_ctypes as cb

# Variables de l'état de la connexion série
ser = None
uart_reader_thread = None
stop_thread_event = threading.Event()

# Queue pour stocker les événements reçus
uart_queue = queue.Queue()

# Thread de lecture UART
def uart_reader():
    global ser
    while not stop_thread_event.is_set():
        if ser and ser.is_open:
            try:
                line = ser.readline()
                if line:
                    line = line.decode("ascii", errors="ignore").strip()
                    #print(f"[UART RAW] {line}")
                    parsed = cb.parse_line(line)
                    if parsed:
                        #print(f"[UART PARSED] {parsed}") 
                        uart_queue.put(parsed)
            except serial.SerialException as e:
                #print(f"Erreur de lecture UART: {e}")
                ser = None
            except Exception as e:
                print(f"Erreur inattendue dans le thread de lecture: {e}")
        time.sleep(0.01)

def set_serial_port(port_name):
    global ser, uart_reader_thread
    
    # Arrêter le thread précédent s'il existe
    if uart_reader_thread and uart_reader_thread.is_alive():
        stop_thread_event.set()
        uart_reader_thread.join()
        stop_thread_event.clear()
        
    # Fermer le port précédent s'il est ouvert
    if ser and ser.is_open:
        ser.close()
        #print(f"Closed previous port.")

    if port_name:
        try:
            ser = serial.Serial(
                port=port_name,
                baudrate=115200,
                bytesize=serial.EIGHTBITS,
                parity=serial.PARITY_NONE,
                stopbits=serial.STOPBITS_ONE,
                timeout=0.1
            )
            #print(f"Port {port_name} is now open.")
            
            # Démarrer un nouveau thread de lecture
            uart_reader_thread = threading.Thread(target=uart_reader, daemon=True)
            uart_reader_thread.start()
        except serial.SerialException as e:
            #print(f"Failed to open port {port_name}: {e}")
            ser = None
        except Exception as e:
            #print(f"Failed to set serial port: {e}")
            ser = None
    else:
        ser = None
        #print("No port selected.")

# Fonction pour obtenir les ports disponibles
def get_available_ports():
    ports = serial.tools.list_ports.comports()
    return [str(p) for p in ports]

# Envoi de commande
def send_command(cmd: str):
    #print(f"[TX] {cmd}")
    if ser and ser.is_open:
        try:
            ser.write((cmd + "\n").encode("ascii"))
        except serial.SerialException as e:
            #print(f"Erreur d'écriture UART: {e}")
            set_serial_port(None) # Déconnecter en cas d'erreur
        except Exception as e:
            print(f"Erreur d'écriture inattendue: {e}")
    else:
        print("Port série non ouvert ou déconnecté. Commande non envoyée.")

# Lecture non bloquante des événements
def get_next_event(event_type=None):
    try:
        while True:
            parsed = uart_queue.get_nowait()
            #print(f"[RX] {parsed}")
            if event_type is None or parsed.get("type") == event_type:
                return parsed
    except queue.Empty:
        return None
