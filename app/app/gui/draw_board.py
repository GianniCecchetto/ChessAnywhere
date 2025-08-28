import customtkinter as ctk
import tkinter as tk
from PIL import Image, ImageTk 
import chess
from . import app_color as c 

OUTLINE_WIDTH = 0
PIECES_PATH = "assets/" 

# mappage des noms des pièces de python-chess aux noms des fichiers
PIECES_MAP = {
    'p': 'bP', 'n': 'bN', 'b': 'bB', 'r': 'bR', 'q': 'bQ', 'k': 'bK',
    'P': 'wP', 'N': 'wN', 'B': 'wB', 'R': 'wR', 'Q': 'wQ', 'K': 'wK'
}

def draw_chessboard(parent, size=8, square_size=70, board=None, playable_square=None,player_color = True):
    """
    Dessine un échiquier avec les pièces.
    """
    canvas = tk.Canvas(parent, 
                       width=size * square_size, 
                       height=size * square_size, 
                       highlightthickness=0,
                       bg=c.BG_COLOR)
    canvas.grid(row=0, column=0)
    
    colors = [c.WHITE_SQUARE, c.BLACK_SQUARE]
    
    # Dictionnaire pour stocker les références aux images
    piece_images = {}

    for row in range(size):
        for col in range(size):
            if player_color:
                row_index = row
                col_index = col
            else:
                row_index = 7 - row
                col_index = 7 - col

            square_color = colors[(row + col) % 2]
            fill_color = square_color

            if playable_square:
                if playable_square[row_index][col_index] == "X":
                    fill_color = "#FF0000"
                elif playable_square[row_index][col_index] == "O":
                    fill_color = "#0026FF"
                elif playable_square[row_index][col_index] == "P":
                    fill_color = "#00FBFF"
                elif playable_square[row_index][col_index] == "W":
                    fill_color = "#FFAE00"

            x1 = col * square_size
            y1 = row * square_size
            x2 = x1 + square_size
            y2 = y1 + square_size

            canvas.create_rectangle(x1, y1, x2, y2, fill=square_color, outline="", width=OUTLINE_WIDTH)

            if playable_square and playable_square[row_index][col_index] != ".":
                canvas.create_rectangle(x1 + 6, y1 + 6, x2 - 6, y2 - 6, 
                                        fill=fill_color, width=OUTLINE_WIDTH, outline="gray")
                # uart envoyer la couleur via fmt_led_set

    # ----- dessiner les pièces -----
    if board:
        for square in chess.SQUARES:
            piece = board.piece_at(square)
            if piece:
                row, col = divmod(square, 8)
                
                if player_color :
                    y_coord = (7 - row) * square_size
                    x_coord = col * square_size
                else :
                    y_coord = (row) * square_size
                    x_coord = (7-col) * square_size

                piece_name = PIECES_MAP[piece.symbol()]
                image_path = f"{PIECES_PATH}{piece_name}.png"

                try:
                    img = Image.open(image_path)
                    img = img.resize((square_size, square_size), Image.LANCZOS)
                    piece_images[square] = ImageTk.PhotoImage(img)
                    
                    canvas.create_image(x_coord + square_size // 2, y_coord + square_size // 2, 
                                        image=piece_images[square])
                except FileNotFoundError:
                    print(f"Erreur : le fichier image {image_path} n'a pas été trouvé.")

    canvas.create_rectangle(0, 0, size * square_size, size * square_size, 
                            outline=c.DARK_BTN_BG, width=3)
    canvas.piece_images = piece_images