import chess
from .gen_move import get_legal_moves_for_piece
from .gen_move import get_matrix_of_legal_move
from .gen_move import get_legal_squares_for_piece
from gui.draw_board import draw_chessboard
from uart.uart_com import get_next_event, send_command
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
    'illegal_move_pending': False, 
    'legal_moves_matrix': None,   
}

def online_game_loop(board_container, board, player_color):
    print("===== starting online game =====")
    
    game_state['board'] = board
    game_state['container'] = board_container
    game_state['player_color'] = player_color
    game_state['start_square'] = None
    game_state['current_turn'] = 'local' if player_color == chess.WHITE else 'online'
    
    draw_chessboard(board_container, board=board, player_color=player_color)
    
    process_online_game_events()


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
        elif board.is_stalemate():
            print("Partie nulle par pat.")
        # Gérer d'autres fins de partie si nécessaire
        return

    event = get_next_event()
    if event:
        handle_event(event)

    board_container.after(50, process_game_events)


def process_online_game_events():
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
    elif game_state['current_turn'] == 'online':
        print("C'est au tour de l'adversaire en ligne. Récupération du coup via l'API.")
        handle_online_move_event()

    board_container.after(50, process_online_game_events)

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
    
    # Si un coup illégal est en attente, une pièce doit être déplacée
    if game_state['illegal_move_pending']:
        piece = board.piece_at(game_state['start_square'])
        if square != game_state['start_square']:
            print("Erreur : veuillez replacer la pièce mal jouée avant de soulever une nouvelle pièce.")
            send_command("LED_ERROR")
            return
    else:
        piece = board.piece_at(square)
        if not piece or piece.color != board.turn:
            print("Erreur : La pièce choisie n'est pas de votre couleur ou la case est vide.")
            send_command("LED_ERROR")
            return

    game_state['start_square'] = square
    
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
        send_command("LED_ERROR")
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
            send_command("LED_ERROR")
            draw_chessboard(board_container, board=illegale_board, playable_square=playable_matrix, player_color=player_color)
            
            game_state['illegal_move_pending'] = True
            
    except ValueError:
        print("Entrée invalide.")
        game_state['start_square'] = None
        send_command("LED_ERROR")
    except Exception as e:
        print(f"Une erreur s'est produite : {e}")
        game_state['start_square'] = None
        send_command("LED_ERROR")

def handle_local_move_event():
    """
    Gère la logique de lecture des événements UART pour un coup local.
    """
    event = get_next_event()
    if event:
        handle_event(event)

def handle_online_move_event():
    """
    Simule la récupération d'un coup d'un adversaire en ligne et l'applique au plateau.
    """
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    
    # --- SIMULATION DE L'APPEL API ---
    # move_from_api = api_client.get_opponent_move()
    
    # Placeholder pour récupérer un coup de l'API
    online_move = None
    
    print("Tentative de récupérer un coup de l'API...")
    
    if True: # Remplacez par une condition réelle
        # Simuler un coup de l'adversaire pour le test
        legal_moves = list(board.legal_moves)
        if legal_moves:
            online_move = legal_moves[0] # Ou choisissez un coup de manière plus intelligente

    if online_move:
        print(f"Coup de l'adversaire reçu : {online_move.uci()}")
        
        try:
            board.push(online_move)
            print(f"Coup légal joué par l'adversaire : {online_move.uci()}")
            draw_chessboard(board_container, board=board, player_color=player_color)
            game_state['current_turn'] = 'local' # Passer le tour au joueur local
            print("C'est à votre tour de jouer.")
        except Exception as e:
            print(f"Erreur lors de l'application du coup de l'adversaire : {e}")
            # Gérer le cas où le coup de l'adversaire est invalide
            game_state['current_turn'] = 'local' # Revenir au tour du joueur local pour correction
    else:
        print("Pas de coup de l'adversaire disponible. Attente du prochain tour.")
