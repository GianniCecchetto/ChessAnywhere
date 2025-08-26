import chess
from .gen_move import get_legal_moves_for_piece
from .gen_move import get_matrix_of_legal_move
from .gen_move import get_legal_squares_for_piece
from  gui.draw_board import draw_chessboard

def online_game_loop(board_container,board,player_color) :
    print("starting online game \n", board)


def local_game_loop(board_container, board, player_color):
    print("===== starting local game =====")
    #board = chess.Board("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 4")
    draw_chessboard(board_container, board=board, player_color=player_color)
    
    while not board.is_checkmate() and not board.is_stalemate():
        draw_chessboard(board_container, board=board, player_color=player_color)
        print(board, "\n")

        valid_move_found = False
        while not valid_move_found:
            try:
                # recevoire la piece jouer par uart
                # chess.A1 = 0 chess.H7 =7
                # ...
                # 8  9  10 11 12 13 14 15 
                # 0  1  2  3  4  5  6  7  
                #valeur recu
                start_square_input = input("Pièce à déplacer: ")# UART
                start_square = int(start_square_input)
                
                piece = board.piece_at(start_square)
                if not piece or piece.color != board.turn:
                    print("Erreur : La pièce choisie n'est pas de votre couleur ou la case est vide.")
                    #si piece de la mauvaise couleur = soulever effet led 
                    continue
                
                playable_square = get_matrix_of_legal_move(board, start_square)
                print(playable_square)
                draw_chessboard(board_container, board=board, playable_square=playable_square, player_color=player_color)
                legal_squares = get_legal_squares_for_piece(board,start_square)
                for row in range(8):
                    print(" ".join(playable_square[row]))
                dest_square_input = input("case de destination : ") # UART
                dest_square = int(dest_square_input)
                
                if dest_square == start_square :
                    draw_chessboard(board_container, board=board, player_color=player_color)
                    continue
                current_fen = board.fen()
                
                move_uci = f"{chess.square_name(start_square)}{chess.square_name(dest_square)}"
                move = chess.Move.from_uci(move_uci)

                temp_board = chess.Board(current_fen)
                turn = board.turn

                while 1:
                    if dest_square in legal_squares:
                        board.push(move)
                        valid_move_found = True
                        draw_chessboard(board_container, board=board, player_color=player_color)
                        break
                    else:
                        valid_move_found = False

                        print("Coup illégal. Veuillez remettre la pièce à sa case de départ ou jouer un coup valid.")
                        
                        temp_board.push(move)
                        temp_board.turn = turn
                        row, col = divmod(dest_square, 8)
                        
                        playable_square[7-row][col] = "W"
                        draw_chessboard(board_container, board=temp_board,playable_square=playable_square,player_color=player_color)
                        playable_square[7-row][col] = "."
                        #ingnorer le soulever de piece, il est nécessaire de rejouer la derniere piece deplacer

                        new_dest_square_input = input("nouvelle destination : ")# UART
                        new_dest_square = int(new_dest_square_input)
               

                        move_uci = f"{chess.square_name(dest_square)}{chess.square_name(new_dest_square)}"
                        move = chess.Move.from_uci(move_uci)
                        print(legal_squares)
                        print(new_dest_square)
                        if new_dest_square in legal_squares:
                            temp_board.push(move)
                            board = temp_board
                            print("coup ok")
                            valid_move_found = True
                            break
                        else:
                            dest_square = new_dest_square
                            continue
                        



                    
            except ValueError:
                print("Entrée invalide. Veuillez entrer un nombre (0-63).")
            except Exception as e:
                print(f"Une erreur s'est produite : {e}")
        
    if board.is_checkmate():
        print("Échec et mat ! Le joueur", "Blanc" if board.turn == chess.WHITE else "Noir", "a gagné.")
    elif board.is_stalemate():
        print("Partie nulle par pat.")
    elif board.is_insufficient_material():
        print("Partie nulle par matériel insuffisant.")
    elif board.is_fivefold_repetition():
        print("Partie nulle par répétition quintuple.")
    elif board.is_seventyfive_moves():
        print("Partie nulle par la règle des 75 coups.")