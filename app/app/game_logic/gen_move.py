import chess

def get_legal_moves_for_piece(board: chess.Board, square: chess.Square):
    moves = []
    for move in board.legal_moves:
        if move.from_square == square:
            moves.append(move)
    return moves


def get_legal_squares_for_piece(board: chess.Board, square: chess.Square):
    squares = [square]
    for move in board.legal_moves:
        if move.from_square == square:
            squares.append(move.to_square)
    return squares


def get_matrix_of_legal_move(board: chess.Board, square: chess.Square):
    matrix = [["." for _ in range(8)] for _ in range(8)]

    row, col = divmod(square, 8)
    matrix[7 - row][col] = "O"

    legal_moves = get_legal_moves_for_piece(board, square)
    #print(legal_moves)
    for move in legal_moves:
        row, col = divmod(move.to_square, 8)
        if board.piece_at(move.to_square):
            matrix[7 - row][col] = "X"
        else:
            matrix[7 - row][col] = "P"

    return matrix

