from .game_loop import *
import chess
import berserk
from ..uart.uart_com import get_next_event, send_command
from .local_func import handle_local_move_event
from .gen_move import get_matrix_from_squares
import time
import threading

from lib.uart_fmt.python_doc import board_com_ctypes as cb

def process_online_game_events(game_state):
    """
    Boucle de jeu principale pour la partie en ligne. Gère les tours de manière non bloquante.
    """
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    client: berserk.Client = game_state['client']
    game_id = game_state['game_id']
    game_state['local_move'] = "start"

    # Vérification de la fin de partie
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

    if board.turn == player_color:
        #print("C'est à votre tour. Attente d'un coup via l'UART.")
        handle_local_move_event(game_state)
        if game_state['end_square']:
            try:
                client.board.make_move(game_id, game_state['end_square'])
                #print("Lichess accepted the move")
            except berserk.exceptions.ResponseError as e:
                print("Move rejected by Lichess:", e)

    if game_state['client']:
        board_container.after(50, lambda: process_online_game_events(game_state))

def show_online_move(game_state):
    """Boucle non bloquante comme dédiée à l’attente UART."""
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']

    move = chess.Move.from_uci(game_state['online_move'])
    squares = [move.from_square, move.to_square]
    
    move_matrix = get_matrix_from_squares(board, board.piece_at(move.from_square), squares)
    draw_chessboard(board_container, board=board, playable_square=move_matrix, player_color=player_color)
    wait_for_uart_confirmation(game_state)

def wait_for_uart_confirmation(game_state):
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    # Check if UART has sent an event
    event = get_next_event()
    if event:
        handle_online_event(game_state, event)

        # After handling event, check if full move confirmed
        if game_state['end_square'] == game_state["online_move"]:
            #print("Coup confirmé par l'échiquier.")
            
            game_state['online_move'] = None
            game_state['end_square'] = None
            
            draw_chessboard(board_container, board=board, player_color=player_color)
            return

    # Try again later (non-blocking)
    if game_state['client']:
        board_container.after(100, lambda: wait_for_uart_confirmation(game_state))

def handle_online_event(game_state, event):    
    event_type = event.get('type')
    square = event.get('idx')

    #print(event_type)
    if event_type == cb.CB_EVT_LIFT:
        #print("LEVER")
        handle_online_lift_event(game_state, square)
    elif event_type == cb.CB_EVT_PLACE:
        #print("POSER")
        handle_online_place_event(game_state, square)

def handle_online_lift_event(game_state, square):
    board = game_state['board']
    
    piece = board.piece_at(square)
    #print(piece)
    #print(board.turn)
    if not piece or piece.color != board.turn:
        #print("Erreur : La pièce choisie n'est pas de votre couleur ou la case est vide.")
        send_command("LED_ERROR")
        return
        
    game_state['start_square'] = square
    game_state['end_square'] = None

    #print("Pièce soulevée, en attente de la destination.")

def handle_online_place_event(game_state, square):
    board: chess.Board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    start_square = game_state['start_square']
    
    dest_square = square
    
    if game_state['online_move'] == None:
        return

    if game_state['local_move'] == game_state['online_move']:
        return

    online_move = chess.Move.from_uci(game_state['online_move'])

    if start_square == dest_square:
        #print("Coup annulé. Pièce reposée à la même place.")
        squares = [online_move.from_square, online_move.to_square]
    
        move_matrix = get_matrix_from_squares(board, board.piece_at(start_square), squares)
        draw_chessboard(board_container, board=board, playable_square=move_matrix, player_color=player_color)
        return
    
    try:
        move = chess.Move(start_square, dest_square)
        try:
            move = board.find_move(start_square, dest_square) # Promote automatically to queen
        except:
            move = chess.Move(start_square, dest_square)

        #print(move)
        
        if start_square == online_move.from_square and move.to_square == online_move.to_square and board.is_legal(move):
            board.push(online_move)
            game_state['last_online_move_confirmed'] = game_state['online_move']
            game_state['online_move'] = None
            online_move = None
            
            #print(f"Coup légal joué : {move.uci()}")
            draw_chessboard(board_container, board=board, player_color=player_color)        
        else:
            illegale_board = board.copy()
            illegale_board.push(move)
            
            squares = [online_move.from_square, online_move.to_square]
        
            move_matrix = get_matrix_from_squares(board, board.piece_at(start_square), squares)
            
            y, x = divmod(square, 8)
            move_matrix[7-y][x] = "W"
            
            #print("Coup illégal. Veuillez remettre la pièce à sa case de départ ou jouer un coup valide.")
            draw_chessboard(board_container, board=illegale_board, playable_square=move_matrix, player_color=player_color)
            
    except ValueError:
        #print("Entrée invalide.")
        send_command("LED_ERROR")
    except Exception as e:
        #print(f"Une erreur s'est produite : {e}")
        send_command("LED_ERROR")


def start_polling(game_state):
    """Boucle indépendante qui lit l'API Lichess"""
    board = game_state['board']
    player_color = game_state['player_color']
    client = game_state['client']
    game_id = game_state['game_id']
    container = game_state['container']

    def poll_stream():
        while game_state['client']:
            if game_state['online_move']:
                if board.turn == player_color:
                    time.sleep(3)
                    continue
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
                        if moves_list:
                            newest_move = moves_list[-1]

                            # Only consider opponent moves
                            if newest_move != game_state['last_move'] and newest_move != game_state['local_move']:
                                game_state['online_move'] = newest_move
                                #print(f"New move from opponent: {newest_move}")
                                container.after(0, lambda: show_online_move(game_state))

            except Exception as e:
                #print(f"Stream interrompu: {e}, reconnexion dans 3s...")
                time.sleep(3)

    threading.Thread(target=poll_stream, daemon=True).start()
    #print("Started polling")
