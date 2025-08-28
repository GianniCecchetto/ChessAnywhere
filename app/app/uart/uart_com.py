import serial

# === Configuration du port COM ===
ser = serial.Serial(
    port="COM3",# Remplacer "COM3" par le port correct (sous Linux : "/dev/ttyUSB0" ou "/dev/ttyAMA0") wsl pas possible je crois... a valider
    baudrate=115200,
    bytesize=serial.EIGHTBITS,
    parity=serial.PARITY_NONE,
    stopbits=serial.STOPBITS_ONE,
    timeout=1
)

def send_string(message: str):
    if ser.is_open:
        ser.write(message.encode("utf-8"))
        print(f"Envoyé: {message}")
    else:
        print("Port série non ouvert")

def receive_string():
    if ser.is_open:
        data = ser.readline().decode("utf-8").strip()
        if data:
            print(f"Reçu: {data}")
        return data
    else:
        print("Port série non ouvert")
        return None