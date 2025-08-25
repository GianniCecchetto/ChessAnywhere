import customtkinter as ctk
import tkinter as tk
from . import app_color as c 

playable_square = [[".",".",".",".",".",".",".",".",],
                   [".",".",".",".",".","X",".",".",],
                   [".",".",".",".",".","P",".",".",],
                   [".",".",".",".",".","P",".",".",],
                   [".",".",".",".","P","O","P","P",],
                   [".",".",".",".",".","X",".",".",],
                   [".",".",".",".",".",".",".",".",],
                   [".",".",".",".",".",".",".",".",]]

def draw_chessboard(parent, size=8, square_size=70):
        """
        Dessine un échiquier sur un canevas.
        """
        canvas = tk.Canvas(parent, 
                           width=size * square_size, 
                           height=size * square_size, 
                           highlightthickness=0, # Retire la bordure par défaut
                           bg=c.BG_COLOR)
        canvas.grid(row=0, column=0)
        
        colors = [c.WHITE_SQUARE, c.BLACK_SQUARE]
        for row in range(size):
            for col in range(size):
                x1 = col * square_size
                y1 = row * square_size
                x2 = x1 + square_size
                y2 = y1 + square_size
                if playable_square[row][col] == "X":
                    canvas.create_rectangle(x1, y1, x2, y2, fill="#FF0000", width = 3, outline="gray")
                    canvas.create_rectangle(x1+6, y1+6, x2-6, y2-6, fill=colors[(row + col) % 2], outline="")
                if playable_square[row][col] == "O":
                    canvas.create_rectangle(x1, y1, x2, y2, fill="#0026FF", width = 3, outline="gray")
                    canvas.create_rectangle(x1+6, y1+6, x2-6, y2-6, fill=colors[(row + col) % 2], outline="")
                if playable_square[row][col] == "P":
                    canvas.create_rectangle(x1, y1, x2, y2, fill="#00FBFF", width = 3, outline="gray")
                    canvas.create_rectangle(x1+6, y1+6, x2-6, y2-6, fill=colors[(row + col) % 2], outline="")
                elif playable_square[row][col] == ".":
                    canvas.create_rectangle(x1, y1, x2, y2, fill=colors[(row + col) % 2],width = 3, outline="gray")
        
        # Dessine manuellement le contour pour un alignement parfait
        canvas.create_rectangle(0, 0, size * square_size, size * square_size, 
                                outline=c.DARK_BTN_BG, width=3)