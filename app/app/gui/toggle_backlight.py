from uart.uart_com  import send_command

def toggle_backlight(button):
    """
    Active ou désactive le rétroéclairage et met à jour le texte du bouton.
    """
    if "ON" in button.cget("text"):
        button.configure(text="🔆 OFF")
        send_command(":BACKLIGHT 0")
    else:
        button.configure(text="🔆 ON")
        send_command(":BACKLIGHT 1")