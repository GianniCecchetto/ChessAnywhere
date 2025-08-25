from tkinter import messagebox
import ctypes

mylib = ctypes.CDLL("./libc/test.so")
mylib.get_msg.restype = ctypes.c_char_p  # si ta fonction C renvoie une string

def join_game(game_name):
    """
    Simule l'action de rejoindre une partie.
    """
    test = mylib.get_msg(game_name.encode("utf-8")).decode("utf-8")
    messagebox.showinfo(
        "Rejoindre une partie",
        f"Vous avez rejoint la partie : {game_name}\n"
        f"msg: {test}"
    )