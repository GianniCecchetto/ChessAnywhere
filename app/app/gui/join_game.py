from tkinter import messagebox
import chess
from ..game_logic.game_loop import start_local_game, start_online_game
import berserk
from ..networks.lichess_api import fetch_game, load_token, verify_token
from ..networks.chess_anywhere_api import create_game

def join_online_game(board_container, challenge_id):
    """
    Join a Lichess game, whether private or open.
    Waits for open challenges to start.
    """
    token = load_token()
    if not token:
        print("No token found")
        return

    session = berserk.TokenSession(token)
    client = berserk.Client(session=session)

    # Private challenge: accept if directed to you
    if challenge_id:
        try:
            client.challenges.accept(challenge_id)
            print(f"Accepted private challenge {challenge_id}")
        except berserk.exceptions.ResponseError:
            print("Challenge cannot be accepted (maybe already started)")

    # Listen for gameStart events
    print("Waiting for the game to start...")
    game_id = None
    for event in client.board.stream_incoming_events():
        if event['type'] == "gameStart":
            # Only continue if it's your challenge or a random open challenge
            if challenge_id is None or event['game']['id'] == challenge_id:
                game_id = event['game']['id']
                print("Game started! ID:", game_id)
                break

    if not game_id:
        print("No game started for this challenge")
        return

    # Export the game info
    game_info = fetch_game(board_container, client, game_id)

    # Determine player color
    player_id = client.account.get()['id']
    players = game_info['players']
    white_id = players.get('white', {}).get('user', {}).get('id')
    black_id = players.get('black', {}).get('user', {}).get('id')

    if white_id == player_id:
        player_color = chess.WHITE
        print("You are playing as White")
    elif black_id == player_id:
        player_color = chess.BLACK
        print("You are playing as Black")
    else:
        print("You are not part of this game")
        return
    
    messagebox.showinfo(
        "Rejoindre une partie en ligne",
        f"Vous avez rejoint la partie : {game_id}\n"
    )
    print("Rejoint la partie:", game_info['id'])
    
    board = chess.Board()
    start_online_game(board_container,board, player_color, client, game_id)

def create_online_game(board_container):
    game_id = create_game()

    join_online_game(board_container, game_id)

def create_local_game(board_container):
    player_color = chess.WHITE
   ## messagebox.showinfo(
   ##     "Création de la partie local"
   ## )
    board = chess.Board()
    start_local_game(board_container,board,player_color)


