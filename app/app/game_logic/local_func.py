from .game_loop import *
import chess
from .gen_move import get_matrix_of_legal_move
from ..uart.uart_com import get_next_event, send_command

from lib.uart_fmt.python_doc import board_com_ctypes as cb

def process_game_events(game_state):
    board = game_state['board']
    board_container = game_state['container']
    
    if board.is_checkmate():
        #print("Partie terminée.")
        #print(board, "\n")
        #print("Échec et mat ! Le joueur", "Blanc" if board.turn == chess.WHITE else "Noir", "a gagné.")
        
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
        handle_event(game_state, event)

    if not game_state['client']:
        board_container.after(50, lambda: process_game_events(game_state))

def handle_local_move_event(game_state):
    """
    Gère la logique de lecture des événements UART pour un coup local.
    """
    event = get_next_event()
    if event:
        handle_event(game_state, event)

def handle_event(game_state, event):    
    event_type = event.get('type')
    square = event.get('idx')

    if event_type == cb.CB_EVT_LIFT:
        handle_lift_event(game_state, square)
    elif event_type == cb.CB_EVT_PLACE:
        handle_place_event(game_state, square)

def handle_lift_event(game_state, square):
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    
    if game_state['illegal_move_pending']:
        if square != game_state['start_square']:
            #print("Erreur : veuillez replacer la pièce mal jouée avant de soulever une nouvelle pièce.")
            return
    else:
        piece = board.piece_at(square)
        if not piece or piece.color != board.turn:
            #print("Erreur : La pièce choisie n'est pas de votre couleur ou la case est vide.")
            return

    game_state['start_square'] = square
    game_state['end_square'] = None
    
    playable_matrix = get_matrix_of_legal_move(board, square)
    game_state['legal_moves_matrix'] = playable_matrix # Stocker la matrice pour une utilisation ultérieure
    draw_chessboard(board_container, board=board, playable_square=playable_matrix, player_color=player_color)
    #print("Pièce soulevée, en attente de la destination.")

def handle_place_event(game_state, square):
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    start_square = game_state['start_square']
    
    dest_square = square
    
    if start_square is None:
        #print("Erreur : Pièce posée sans en avoir soulevé une au préalable.")
        return

    if start_square == dest_square:
        #print("Coup annulé. Pièce reposée à la même place.")
        draw_chessboard(board_container, board=board, player_color=player_color)
        game_state['start_square'] = None
        game_state['illegal_move_pending'] = False
        game_state['legal_moves_matrix'] = None
        return

    try:
        move = chess.Move(start_square, dest_square)
        
        try:
            move = board.find_move(start_square, dest_square) # Promote automatically to queen
        except:
            move = chess.Move(start_square, dest_square)

        if move in board.legal_moves:
            board.push(move)
            #print(f"Coup légal joué : {move.uci()}")
            draw_chessboard(board_container, board=board, player_color=player_color)
            game_state['end_square'] = move.uci()
            game_state['local_move'] = move.uci()
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
            
            #print("Coup illégal. Veuillez remettre la pièce à sa case de départ ou jouer un coup valide.")
            draw_chessboard(board_container, board=illegale_board, playable_square=playable_matrix, player_color=player_color)
            
            game_state['illegal_move_pending'] = True
            
    except ValueError:
        #print("Entrée invalide.")
        game_state['start_square'] = None
    except Exception as e:
        #print(f"Une erreur s'est produite : {e}")
        game_state['start_square'] = None
