import chess
from .gen_move import get_legal_moves_for_piece
from .gen_move import get_matrix_of_legal_move
from  gui.draw_board import draw_chessboard

def online_game_loop(board_container,board,player_color) :
    print("starting online game \n", board)


def local_game_loop(board_container,board,player_color) :
    print("starting local game \n", board)
    # attendre sur un mouvement des blancs...

    draw_chessboard(board_container,board=board,player_color=player_color)
    # recevoire la piece jouer par uart
    #     # chess.A1 = 0 chess.H7 =7
    # ...
    # 8  9  10 11 12 13 14 15 
    # 0  1  2  3  4  5  6  7  
    #valeur recu
    piece = input("Piece soulevee en : ")
    piece = int(piece)

    playable_square = get_matrix_of_legal_move(board,piece)
    draw_chessboard(board_container,board=board,playable_square=playable_square,player_color=player_color)
    # TO DO
    piece = input("Piece reposer en : ")
    piece = int(piece)

    
    player_color = True
    #board = chess.Board("rnbqkbnr/p1pp1ppp/4p3/1p6/2B3Q1/4P3/PPPP1PPP/RNB1K1NR w KQkq - 0 6")

    #board = chess.Board("r1bqkb1r/pppp1ppp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNBQK1NR w KQkq - 0 4")

    #legal_moves = get_legal_moves_for_piece(board,piece)
    #playable_square = get_matrix_of_legal_move(board,piece)

    for row in range(8):
            print(" ".join(playable_square[row]))


    #board.push_san("e4")
    
    #board.push_san("e5")
    #board.push_san("Qg4")
    print(legal_moves,"\n")
    print(board,"\n")
    