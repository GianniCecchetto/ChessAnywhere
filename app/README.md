# Application ChessAnywhere

package a installer (a completer)
- python-chess
- tkinter et customtkinter
- pip install Pillow
- pip install pyserial

### IMPORTANT la compilation se fait sur WSL 2 
il est nécessaire de forwarde les ports COM de windows vers wsl

- dans la powershell windows :
    - winget install usbipd
    - usbipd list
exemple de sortie
```
Connected:
BUSID  VID:PID    DEVICE                                                        STATE
1-2    046d:c52f  Périphérique d’entrée USB                                     Not shared
1-4    0483:374b  ST-Link Debug, Dispositif de stockage de masse USB, STMic...  Not shared
1-6    0bda:5676  Integrated Webcam                                             Not shared
1-10   8087:0aaa  Intel(R) Wireless Bluetooth(R)                                Not shared

Persisted:
GUID                                  DEVICE
```
dans notre cas \<BUSID> vaut 1-4

ensuite dans la powershell windows en tant qu'administrateur
- usbipd bind --busid 1-4
- usbipd attach --wsl --busid 1-4

