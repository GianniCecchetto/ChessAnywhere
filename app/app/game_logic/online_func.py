from game_logic.game_loop import *
import chess
import berserk
from uart.uart_com import get_next_event, send_command
from .local_func import handle_local_move_event

def process_online_game_events(game_state):
    """
    Boucle de jeu principale pour la partie en ligne. Gère les tours de manière non bloquante.
    """
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    
    # Vérification de la fin de partie
    if board.is_checkmate() or board.is_stalemate():
        print("Partie terminée.")
        print(board, "\n")
        if board.is_checkmate():
            print("Échec et mat ! Le joueur", "Blanc" if board.turn == chess.WHITE else "Noir", "a gagné.")
        elif board.is_stalemate():
            print("Partie nulle par pat.")
        return

    if game_state['current_turn'] == 'local':
        print("C'est à votre tour. Attente d'un coup via l'UART.")
        handle_local_move_event()
        if game_state['end_square']:
            game_state['client'].board.make_move(game_state['end_square'])
    elif game_state['current_turn'] == 'online':
        print("C'est au tour de l'adversaire en ligne. Récupération du coup via l'API.")
        handle_online_move_event(game_state)

    board_container.after(50, lambda: process_online_game_events(game_state))

def handle_online_move_event(game_state):
    """
    Simule la récupération d'un coup d'un adversaire en ligne et l'applique au plateau.
    """
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    
    print("Tentative de récupérer un coup de l'API...")

    poll_game_state(game_state, game_state['client'].board.stream_game_state(game_state['game_id']))

    online_move = game_state['online_move']

    if online_move:
        print(f"Coup de l'adversaire reçu : {online_move}")
        
        try:
            move = chess.Move.from_uci(online_move)
            board.push(move)
            print(f"Coup légal joué par l'adversaire : {online_move}")
            draw_chessboard(board_container, board=board, player_color=player_color)
            game_state['current_turn'] = 'local' # Passer le tour au joueur local
            print("C'est à votre tour de jouer.")
        except Exception as e:
            print(f"Erreur lors de l'application du coup de l'adversaire : {e}")
            # Gérer le cas où le coup de l'adversaire est invalide
            game_state['current_turn'] = 'online' # Revenir au tour du joueur local pour correction
    else:
        print("Pas de coup de l'adversaire disponible. Attente du prochain tour.")

def poll_game_state(game_state, game_stream):
    try:
        online_game_state = next(game_stream)  # get next update
        # process the game_state here
        game_state['online_move'] = online_game_state.get("state", "").get("moves", "")
        print("Moves:", game_state['online_move'])
    except StopIteration:
        print("Game stream ended")
        return

    # Schedule next poll in 1 second
    game_state['container'].after(1000, lambda: poll_game_state(game_state, game_stream))
