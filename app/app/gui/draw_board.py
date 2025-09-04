import tkinter as tk
import chess
from . import app_color as c 
from ..uart.uart_com  import send_command
import time
import threading

from lib.uart_fmt.python_doc import board_com_ctypes as cb

OUTLINE_WIDTH = 0

# Mapping des symboles vers couleurs LED (R, G, B)
LED_COLORS = {
    "X": (255, 0, 0),      # rouge
    "O": (0, 0, 255),      # bleu
    "P": (0, 255, 255),    # cyan
    "W": (255, 174, 0),    # orange
    "M": (0, 182, 0)
}

def square_to_idx(row: int, col: int) -> int:
    return row * 8 + (7-col)

def draw_chessboard(parent, size=8, square_size=70, board=None, playable_square=None, player_color=True):
    app = parent.winfo_toplevel()  # gets the root window (your chess_anywhere_app)
    preloaded_pieces = getattr(app, "PRELOADED_PIECES", {})

    canvas = tk.Canvas(parent, 
                       width=size * square_size, 
                       height=size * square_size, 
                       highlightthickness=0,
                       bg=c.BG_COLOR)
    canvas.grid(row=0, column=0)
    
    colors = [c.WHITE_SQUARE, c.BLACK_SQUARE]

    cmds = []

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
                symbol = playable_square[row_index][col_index]
                if symbol in LED_COLORS:
                    r, g, b = LED_COLORS[symbol]
                    idx = square_to_idx(row_index, col_index)

                    # === UART : envoyer commande LED ===
                    cmd = cb.fmt_led_set(63-idx, r, g, b)
                    cmds.append(cmd)

                    # === Graphique : colorier la case ===
                    if symbol == "X":
                        fill_color = "#FF0000"
                    elif symbol == "O":
                        fill_color = "#0026FF"
                    elif symbol == "P":
                        fill_color = "#00FBFF"
                    elif symbol == "W":
                        fill_color = "#FFAE00"
                    elif symbol == "M":
                        fill_color = "#00B600"

            x1 = col * square_size
            y1 = row * square_size
            x2 = x1 + square_size
            y2 = y1 + square_size

            canvas.create_rectangle(x1, y1, x2, y2, fill=square_color, outline="", width=OUTLINE_WIDTH)

            if playable_square and playable_square[row_index][col_index] != ".":
                canvas.create_rectangle(x1 + 6, y1 + 6, x2 - 6, y2 - 6, 
                                        fill=fill_color, width=OUTLINE_WIDTH, outline="gray")

    threading.Thread(target=send_command_with_delay, args=(cmds, 0.007), daemon=True).start()

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

                piece_name = piece.symbol()
                if piece_name in preloaded_pieces:
                    img = preloaded_pieces[piece_name]
                    canvas.create_image(x_coord + square_size // 2, y_coord + square_size // 2, image=img)

    canvas.create_rectangle(0, 0, size * square_size, size * square_size, 
                            outline=c.DARK_BTN_BG, width=3)

def send_command_with_delay(cmds, delay):
    for cmd in cmds:
        send_command(cmd)
        time.sleep(delay)
