import chess
from .gen_move import get_legal_moves_for_piece
from .gen_move import get_matrix_of_legal_move
from .gen_move import get_legal_squares_for_piece
from gui.draw_board import draw_chessboard
from uart.uart_com import get_next_event, send_command
from game_logic.local_func import process_game_events
from game_logic.online_func import process_online_game_events
import berserk
import os
import sys

BASE_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
print(BASE_DIR)
UART_PATH = os.path.join(BASE_DIR, "lib", "uart_fmt", "python_doc")
print(UART_PATH)
sys.path.append(UART_PATH)

import board_com_ctypes as cb

# État de jeu partagé
game_state = {
    'board': None,
    'container': None,
    'player_color': None,
    'start_square': None,
    'end_square': None,
    'current_turn': None,
    'client': None,
    'game_id': None,
    'online_move': None
}

def online_game_loop(board_container, board, player_color, client, game_id):
    print("===== starting online game =====")

    game_state['board'] = board
    game_state['container'] = board_container
    game_state['player_color'] = player_color
    game_state['start_square'] = None
    game_state['current_turn'] = 'local' if player_color == chess.WHITE else 'online'
    game_state['client'] = client
    game_state['game_id'] = game_id
    
    draw_chessboard(board_container, board=board, player_color=player_color)
    
    process_online_game_events(game_state)


def local_game_loop(board_container, board, player_color):
    print("===== starting local game =====")
    
    game_state['board'] = board
    game_state['container'] = board_container
    game_state['player_color'] = player_color
    game_state['start_square'] = None
    
    draw_chessboard(board_container, board=board, player_color=player_color)
    
    process_game_events(game_state)

def handle_event(event):
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    start_square = game_state['start_square']
    
    event_type = event.get('type')
    square = event.get('idx')

    if event_type == cb.CB_EVT_LIFT:
        handle_lift_event(square)
    elif event_type == cb.CB_EVT_PLACE:
        handle_place_event(square)

def handle_lift_event(square):
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    
    piece = board.piece_at(square)
    if not piece or piece.color != board.turn:
        print("Erreur : La pièce choisie n'est pas de votre couleur ou la case est vide.")
        send_command("LED_ERROR")
        return
        
    game_state['start_square'] = square
    game_state['end_square'] = None
    
    playable_matrix = get_matrix_of_legal_move(board, square)
    draw_chessboard(board_container, board=board, playable_square=playable_matrix, player_color=player_color)
    print("Pièce soulevée, en attente de la destination.")

def handle_place_event(square):
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    start_square = game_state['start_square']
    
    dest_square = square
    
    if start_square is None:
        print("Erreur : Pièce posée sans en avoir soulevé une au préalable.")
        send_command("LED_ERROR")
        return

    if start_square == dest_square:
        print("Coup annulé. Pièce reposée à la même place.")
        draw_chessboard(board_container, board=board, player_color=player_color)
        game_state['start_square'] = None
        return

    try:
        move = chess.Move(start_square, dest_square)
        
        if move in board.legal_moves:
            board.push(move)
            print(f"Coup légal joué : {move.uci()}")
            draw_chessboard(board_container, board=board, player_color=player_color)
            game_state['end_square'] = move.uci()
        else:
            print("Coup illégal. Veuillez remettre la pièce à sa case de départ ou jouer un coup valide.")
            send_command("LED_ERROR")
            draw_chessboard(board_container, board=board, player_color=player_color) # Redessiner le plateau pour effacer les indicateurs
        
        game_state['start_square'] = None
            
    except ValueError:
        print("Entrée invalide.")
        game_state['start_square'] = None
        send_command("LED_ERROR")
    except Exception as e:
        print(f"Une erreur s'est produite : {e}")
        game_state['start_square'] = None
        send_command("LED_ERROR")
