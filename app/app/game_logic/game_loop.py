import chess
from .gen_move import get_legal_moves_for_piece
from .gen_move import get_matrix_of_legal_move, get_matrix_from_squares
from .gen_move import get_legal_squares_for_piece
from ..gui.draw_board import draw_chessboard
from ..uart.uart_com import get_next_event, send_command
from .local_func import process_game_events
from .online_func import process_online_game_events, start_polling
import berserk
import os
import sys

#BASE_DIR = os.path.dirname(os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__)))))
#UART_PATH = os.path.join(BASE_DIR, "lib", "uart_fmt", "python_doc")
#sys.path.append(UART_PATH)

#import board_com_ctypes as cb
from lib.uart_fmt.python_doc import board_com_ctypes as cb

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
    'online_move': None,
    'last_move': None,
    'illegal_move_pending': False, 
    'legal_moves_matrix': None,   
}

def online_game_loop(board_container, board: chess.Board, player_color, client, game_id):
    print("===== starting online game =====")

    game_state['board'] = board
    game_state['container'] = board_container
    game_state['player_color'] = player_color
    game_state['start_square'] = None
    game_state['client'] = client
    game_state['game_id'] = game_id

    stream = client.board.stream_game_state(game_id)
    for event in stream:
        moves = event.get('state', {}).get('moves', '')
        if moves:
            for uci in moves.split():
                board.push_uci(uci)
                game_state['last_move'] = uci
        break

    game_state['current_turn'] = 'local' if board.turn == player_color else 'online'
    
    draw_chessboard(board_container, board=board, player_color=player_color)

    start_polling(game_state)
    process_online_game_events(game_state)


def local_game_loop(board_container, board, player_color):
    print("===== starting local game =====")
    
    game_state['board'] = board
    game_state['container'] = board_container
    game_state['player_color'] = player_color
    game_state['start_square'] = None
    game_state['illegal_move_pending'] = False
    game_state['legal_moves_matrix'] = None
    
    draw_chessboard(board_container, board=board, player_color=player_color)
    
    process_game_events()

def process_game_events():
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    
    if board.is_checkmate() or board.is_stalemate():
        print("Partie terminée.")
        print(board, "\n")
        if board.is_checkmate():
            print("Échec et mat ! Le joueur", "Blanc" if board.turn == chess.WHITE else "Noir", "a gagné.")
        
        if board.turn == chess.WHITE:
            send_command(":WIN 0")# les noirs ont gagné
        else:
            send_command(":WIN 1")# les blancs ont gagné
        return
    elif board.is_stalemate():
        send_command(":DRAW STALE")
        return
    elif board.is_fifty_moves():
        send_command(":DRAW EXCEEDED")
        return


    event = get_next_event()
    if event:
        handle_event(event)

    board_container.after(50, process_game_events)

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
    
    if game_state['illegal_move_pending']:
        piece = board.piece_at(game_state['start_square'])
        if square != game_state['start_square']:
            print("Erreur : veuillez replacer la pièce mal jouée avant de soulever une nouvelle pièce.")
            return
    else:
        piece = board.piece_at(square)
        if not piece or piece.color != board.turn:
            print("Erreur : La pièce choisie n'est pas de votre couleur ou la case est vide.")
            return

    game_state['start_square'] = square
    game_state['end_square'] = None
    
    playable_matrix = get_matrix_of_legal_move(board, square)
    game_state['legal_moves_matrix'] = playable_matrix # Stocker la matrice pour une utilisation ultérieure
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
        return

    if start_square == dest_square:
        print("Coup annulé. Pièce reposée à la même place.")
        draw_chessboard(board_container, board=board, player_color=player_color)
        game_state['start_square'] = None
        game_state['illegal_move_pending'] = False
        game_state['legal_moves_matrix'] = None
        return

    try:
        move = chess.Move(start_square, dest_square)
        
        if move in board.legal_moves:
            board.push(move)
            print(f"Coup légal joué : {move.uci()}")
            draw_chessboard(board_container, board=board, player_color=player_color)
            game_state['end_square'] = move.uci()
            game_state['start_square'] = None
            game_state['illegal_move_pending'] = False
            game_state['legal_moves_matrix'] = None
        else:
            illegale_board = board.copy()
            illegale_board.push(move)
            
            playable_matrix = game_state['legal_moves_matrix']
            if playable_matrix is None:
                playable_matrix = [["." for _ in range(8)] for _ in range(8)]
            
            y, x = divmod(square, 8)
            playable_matrix[7-y][x] = "W"
            
            print("Coup illégal. Veuillez remettre la pièce à sa case de départ ou jouer un coup valide.")
            draw_chessboard(board_container, board=illegale_board, playable_square=playable_matrix, player_color=player_color)
            
            game_state['illegal_move_pending'] = True
            
    except ValueError:
        print("Entrée invalide.")
        game_state['start_square'] = None
    except Exception as e:
        print(f"Une erreur s'est produite : {e}")
        game_state['start_square'] = None

def handle_online_event(event):    
    event_type = event.get('type')
    square = event.get('idx')

    print(event_type)
    if event_type == cb.CB_EVT_LIFT:
        print("LEVER")
        handle_online_lift_event(square)
    elif event_type == cb.CB_EVT_PLACE:
        print("POSER")
        handle_online_place_event(square)

def handle_online_lift_event(square):
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    
    piece = board.piece_at(square)
    print(piece)
    print(board.turn)
    if not piece or piece.color != board.turn:
        print("Erreur : La pièce choisie n'est pas de votre couleur ou la case est vide.")
        send_command("LED_ERROR")
        return
        
    game_state['start_square'] = square
    game_state['end_square'] = None

    print("Pièce soulevée, en attente de la destination.")

def handle_online_place_event(square):
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
        
        if move.to_square == chess.Move.from_uci(game_state['online_move']).to_square:
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
