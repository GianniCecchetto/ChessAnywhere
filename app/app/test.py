import chess

def get_legal_moves_for_piece(board: chess.Board, square: chess.Square):
    """
    Retourne une liste de coups légaux pour une pièce sur une case donnée.
    
    Args:
        board: L'objet chess.Board.
        square: La case (par exemple, chess.A2) de la pièce.
        
    Returns:
        Une liste d'objets chess.Move.
    """
    moves = []
    for move in board.legal_moves:
        if move.from_square == square:
            moves.append(move)
    return moves

# Exemple d'utilisation
board = chess.Board()
# Récupérer les coups légaux pour le pion en e2
e2_square = chess.E2
e2_moves = get_legal_moves_for_piece(board, e2_square)
print(f"Coups possibles pour la pièce en {chess.square_name(e2_square)} : {e2_moves}")

# Jouer un coup pour changer la position
board.push_san("e4")
# Récupérer les coups pour la Reine blanche
queen_square = chess.D1
queen_moves = get_legal_moves_for_piece(board, queen_square)
print(f"Coups possibles pour la Reine en {chess.square_name(queen_square)} : {queen_moves}")