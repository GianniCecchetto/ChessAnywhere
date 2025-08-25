#!/usr/bin/env python3
# Test unitaire pour STM32
# Attend une chaîne : Hello world ! avant le timeout de 10 sec
import pytest
import serial
import sys
import time

PORT = "/dev/ttyACM0"
BAUDRATE = 115200

def test_uart():

    timeout = 0
    try:
        ser = serial.Serial(PORT, BAUDRATE, timeout=1)
        print(f"[OK] Port {PORT} ouvert à {BAUDRATE} bauds")

        #ser.reset_input_buffer()
        #ser.reset_output_buffer()

        while True:
            line = ser.readline().decode(errors='ignore').strip()
            if line:
                print(f"[STM32] {line}")
                # Vérifie si la chaîne correspond
                if line == "Hello World !":
                    print("[INFO] Message attendu reçu, arrêt du programme.")
                    break
            # timeout de 10 secondes max
            if timeout >= 0.1 * 100:
                break
            else:
                timeout += 1
                time.sleep(0.1)

    except serial.SerialException as e:
        print(f"[ERREUR] Impossible d’ouvrir {PORT} : {e}")
        sys.exit(1)
    except KeyboardInterrupt:
        print("\n[STOP] Arrêt demandé par l’utilisateur.")
    finally:
        if 'ser' in locals() and ser.is_open:
            ser.close()
            print("[OK] Port série fermé.")

    if timeout >= 0.1 * 100:
        assert False
    else:
        assert True
