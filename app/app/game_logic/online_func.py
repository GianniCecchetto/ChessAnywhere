from game_logic.game_loop import *
import chess
import berserk
from uart.uart_com import get_next_event, send_command
from .local_func import handle_local_move_event
from .gen_move import get_matrix_from_squares
import time
import threading

def process_online_game_events(game_state):
    """
    Boucle de jeu principale pour la partie en ligne. Gère les tours de manière non bloquante.
    """
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    client = game_state['client']
    game_id = game_state['game_id']
    
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
        
    board_container.after(500, lambda: process_online_game_events(game_state))

def handle_online_move_event(game_state):
    board = game_state['board']
    online_move = game_state['online_move']

    if not online_move:
        print("Pas de coup de l'adversaire disponible. Attente du prochain tour.")
        return

    print(f"Coup de l'adversaire reçu : {online_move}")

    wait_for_uart_confirmation(game_state)

def wait_for_uart_confirmation(game_state):
    """Boucle non bloquante comme process_game_events, mais dédiée à l’attente UART."""
    container = game_state['container']
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']

    move = chess.Move.from_uci(game_state['online_move'])
    squares = [move.from_square, move.to_square]
    
    move_matrix = get_matrix_from_squares(squares)
    draw_chessboard(board_container, board=board, playable_square=move_matrix, player_color=player_color)

    # Check if UART has sent an event
    event = get_next_event()
    if event:
        handle_online_event(event)

        # ✅ After handling event, check if full move confirmed
        if game_state.get("end_square") == game_state["online_move"]:
            print("Coup confirmé par l'échiquier.")
            board.push(move)  # Apply the move officially
            game_state['online_move'] = None
            game_state["current_turn"] = "local"
            draw_chessboard(board_container, board=board, player_color=player_color)
            return True

    # Try again later (non-blocking)
    container.after(100, lambda: wait_for_uart_confirmation(game_state))
    return False

def start_polling(game_state):
    """Boucle indépendante qui lit l'API Lichess"""
    client = game_state['client']
    game_id = game_state['game_id']

    def poll_stream():
        while True:
            try:
                stream = client.board.stream_game_state(game_id)
                for event in stream:
                    moves = None
                    if event.get("type") == "gameState":
                        moves = event.get("moves", "")
                    elif event.get("type") == "gameFull":
                        moves = event.get("state", {}).get("moves", "")

                    if moves:
                        moves_list = moves.split()
                        game_state['online_move'] = moves_list[-1] if moves_list else None

            except Exception as e:
                print(f"Stream interrompu: {e}, reconnexion dans 3s...")
                time.sleep(3)

    threading.Thread(target=poll_stream, daemon=True).start()
