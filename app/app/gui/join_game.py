from tkinter import messagebox
import ctypes
import chess
from ..game_logic.game_loop import local_game_loop
from ..game_logic.game_loop import online_game_loop
import berserk
import time
from ..networks.lichess_api import fetch_game, load_token, verify_token
from ..networks.chess_anywhere_api import create_game

def join_online_game(board_container,game_id):
    token = load_token()
    session = berserk.TokenSession(token)
    client: berserk.Client = berserk.Client(session=session)
    
    if not verify_token(token):
        print("Token non valide")
        return

    try:
        client.challenges.accept(game_id)

    except berserk.exceptions.ResponseError as e:
        print("Impossible d'accepter le challenge (la partie a peut-être déjà commencée)")
    
    # Export game info
    game_info = fetch_game(board_container, client, game_id)

    player_color = None
    if game_info:
        player_id = client.account.get()['id']      
        players = game_info['players']

        print(game_info)

        white_id = players.get('white', {}).get('user', {}).get('id')
        black_id = players.get('black', {}).get('user', {}).get('id')

        if white_id == player_id:
            player_color = chess.WHITE
            print("Vous êtes les blancs")
        elif black_id == player_id:
            player_color = chess.BLACK
            print("Vous êtes les noirs")
        else:
            player_color = None  # maybe you are not in the game
        
    else:
        print("Partie non trouvée")
        board_container.after(5000, lambda: join_online_game(board_container, game_id))
        return

    if player_color == None:
        print("Cette partie n'est pas pour vous")
        board_container.after(5000, lambda: join_online_game(board_container, game_id))
        return
    
    messagebox.showinfo(
        "Rejoindre une partie en ligne",
        f"Vous avez rejoint la partie : {game_id}\n"
    )
    print("Rejoint la partie:", game_info['id'])
    
    board = chess.Board()
    online_game_loop(board_container,board, player_color, client, game_id)

def create_online_game(board_container):
    game_id = create_game()

    join_online_game(board_container, game_id)

def create_local_game(board_container):
    player_color = chess.WHITE
   ## messagebox.showinfo(
   ##     "Création de la partie local"
   ## )
    board = chess.Board()
    local_game_loop(board_container,board,player_color)


