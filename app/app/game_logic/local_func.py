from .game_loop import *
import chess
from ..uart.uart_com import get_next_event, send_command

def process_game_events(game_state):
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    
    if board.is_checkmate() or board.is_stalemate():
        print("Partie terminée.")
        print(board, "\n")
        if board.is_checkmate():
            print("Échec et mat ! Le joueur", "Blanc" if board.turn == chess.WHITE else "Noir", "a gagné.")
        elif board.is_stalemate():
            print("Partie nulle par pat.")
        # Gérer d'autres fins de partie si nécessaire
        return

    handle_local_move_event()

    board_container.after(50, lambda: process_game_events(game_state))

def handle_local_move_event():
    """
    Gère la logique de lecture des événements UART pour un coup local.
    """
    from .game_loop import handle_event
    event = get_next_event()
    if event:
        handle_event(event)
