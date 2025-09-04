from tkinter import messagebox
import chess
from ..game_logic.game_loop import start_local_game, start_online_game
import berserk
from ..networks.lichess_api import fetch_game, load_token, verify_token
from ..networks.chess_anywhere_api import create_game, join_game

def join_online_game(board_container, game_id):
    """
    Join a Lichess game, whether private or open.
    Waits for open challenges to start.
    """
    token = load_token()
    if not token:
        #print("No token found")
        return
        
    user_id = verify_token(token)
    #print(user_id)
    if not user_id:
        #print("Invalid token")
        return

    session = berserk.TokenSession(token)
    client = berserk.Client(session=session)

    # Private challenge: accept if directed to you
    if game_id:
        try:
            client.challenges.accept(game_id)
            #print(f"Accepted private challenge {game_id}")
        except berserk.exceptions.ResponseError:
            print("Challenge cannot be accepted (maybe already started)")
    
    join_game(game_id, user_id)

    # Listen for gameStart events
    #print("Waiting for the game to start...")

    game_info = fetch_game(client, game_id)

    if not game_info:
        board_container.after(1000, lambda: join_online_game(board_container, game_id))
        return

    #print("Game started")

    # Determine player color
    player_id = client.account.get()['id']
    players = game_info['players']
    white_id = players.get('white', {}).get('user', {}).get('id')
    black_id = players.get('black', {}).get('user', {}).get('id')

    if white_id == player_id:
        player_color = chess.WHITE
        #print("You are playing as White")
    elif black_id == player_id:
        player_color = chess.BLACK
        #print("You are playing as Black")
    else:
        #print("You are not part of this game")
        return
    
    messagebox.showinfo(
        "Rejoindre une partie en ligne",
        f"Vous avez rejoint la partie : {game_id}\n"
    )
    #print("Rejoint la partie:", game_info['id'])
    
    board = chess.Board()
    start_online_game(board_container,board, player_color, client, game_id)

def create_online_game(board_container):
    game_id = create_game().get('response', None)
    if game_id == None:
        return

    join_online_game(board_container, game_id)

def create_local_game(board_container):
    player_color = chess.WHITE
   ## messagebox.showinfo(
   ##     "Création de la partie local"
   ## )
    board = chess.Board()
    start_local_game(board_container,board,player_color)


