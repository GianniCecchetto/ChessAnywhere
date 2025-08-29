import os
import time
import glob
import serial.tools.list_ports

def list_serial_ports():
    """Retourne la liste des ports série ttyS* sous WSL."""
    return sorted(glob.glob("/dev/ttyS*"))

def get_port_info(port):
    """Récupère les infos détaillées si possible via pyserial."""
    for p in serial.tools.list_ports.comports():
        if p.device == port:
            return {
                "device": p.device,
                "description": p.description,
                "hwid": p.hwid
            }

    return {"device": port, "description": "N/A", "hwid": "N/A"}

def monitor_serial_ports(scan_interval=2):
    """Surveille périodiquement les ports série et affiche les changements."""
    print("Scanning des ports série toutes les", scan_interval, "secondes...")
    previous_ports = set(list_serial_ports())

    while True:
        current_ports = set(list_serial_ports())

        new_ports = current_ports - previous_ports
        for port in new_ports:
            info = get_port_info(port)
            print(f"[+] Nouveau port détecté : {info['device']}")
            print(f"    ↳ Description : {info['description']}")
            print(f"    ↳ HWID        : {info['hwid']}")

        removed_ports = previous_ports - current_ports
        for port in removed_ports:
            print(f"[-] Port déconnecté : {port}")

        previous_ports = current_ports
        time.sleep(scan_interval)


if __name__ == "__main__":
    monitor_serial_ports(scan_interval=1)
