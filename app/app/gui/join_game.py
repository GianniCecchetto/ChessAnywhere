from tkinter import messagebox
import ctypes
import chess
from game_logic.game_loop import local_game_loop
from game_logic.game_loop import online_game_loop
import berserk
import time
from networks.lichess_api import fetch_game

#mylib = ctypes.CDLL("./libc/test.so")
#mylib.get_msg.restype = ctypes.c_char_p



def join_online_game(board_container,game_id):
 #   test = mylib.get_msg(game_name.encode("utf-8")).decode("utf-8")
    session = berserk.TokenSession("lip_Y2gBhf5qbnqLQrDInoHN")
    client = berserk.Client(session=session)

    try:
        client.challenges.accept(game_id)

    except berserk.exceptions.ResponseError as e:
        print("Erreur", f"Impossible d'accepter le challenge : {e}")

    data = client.challenges.get_mine()

    # Safely get incoming and outgoing
    incoming = data.get('in', []) if isinstance(data, dict) else []
    outgoing = data.get('out', []) if isinstance(data, dict) else []

    for challenge in outgoing:
        print("Challenge ID:", challenge['id'])
        print("Variant:", challenge['variant']['name'])
        print("Status:", challenge['status'])
        print("Color:", challenge['color'])

    
    # Export game info
    game_info = fetch_game(board_container, client, game_id)

    player_color = None
    if game_info:
        print("Game ID:", game_info['id'])
        player_id = client.account.get()['id']      
        players = game_info['players']
        print(players)
        print(player_id)

        if 'user' in players['white'] and players['white']['user']['id'] == player_id:
            player_color = chess.WHITE
        elif 'user' in players['black'] and players['black']['user']['id'] == player_id:
            player_color = chess.BLACK
        else:
            player_color = None  # maybe you are not in the game
        
        messagebox.showinfo(
            "Rejoindre une partie en ligne",
            f"Vous avez rejoint la partie : {game_id}\n"
        )
    else:
        print("Game not found yet.")

    board = chess.Board()
    online_game_loop(board_container,board, player_color, client, game_id)

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


