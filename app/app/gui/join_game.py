from tkinter import messagebox
import ctypes
import chess
from game_logic.game_loop import local_game_loop
from game_logic.game_loop import online_game_loop

#mylib = ctypes.CDLL("./libc/test.so")
#mylib.get_msg.restype = ctypes.c_char_p

def join_online_game(board_container,game_name):
 #   test = mylib.get_msg(game_name.encode("utf-8")).decode("utf-8")
    messagebox.showinfo(
        "Rejoindre une partie en ligne",
        f"Vous avez rejoint la partie : {game_name}\n"
        #f"msg: {test}"
    )
    board = chess.Board()
    online_game_loop(board_container,board,player_color)

def create_online_game(board_container):
    messagebox.showinfo(
        "Créer une partie en ligne",
    )
    # recuperer la couleur, par défaut BLACK
    player_color = chess.BLACK
    board = chess.Board()
    online_game_loop(board_container,board,player_color)

def create_local_game(board_container):
    player_color = chess.WHITE
   ## messagebox.showinfo(
   ##     "Création de la partie local"
   ## )
    board = chess.Board()
    local_game_loop(board_container,board,player_color)


