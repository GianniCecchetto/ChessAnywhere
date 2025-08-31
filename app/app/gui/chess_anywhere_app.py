import customtkinter as ctk
import tkinter as tk
from PIL import Image, ImageTk 
from . import app_color as c
from  .widgets_app import create_widgets
from  .settings_menu import create_settings_menu

PIECES_PATH = "assets/" 

PIECES_MAP = {
    'p': 'bP', 'n': 'bN', 'b': 'bB', 'r': 'bR', 'q': 'bQ', 'k': 'bK',
    'P': 'wP', 'N': 'wN', 'B': 'wB', 'R': 'wR', 'Q': 'wQ', 'K': 'wK'
}

class chess_anywhere_app(ctk.CTk):
    """
    Classe principale pour l'application Chess Anywhere,
    basée sur customtkinter.
    """
    def __init__(self):
        super().__init__()

        self.title("Chess Anywhere")
        self.geometry("1000x710")
        self.resizable(True, True)
    
        # Configure le fond de la fenêtre principale en blanc pour le padding.
        self.configure(fg_color=c.BG_COLOR)
        
        ctk.set_appearance_mode("light")
        ctk.set_default_color_theme("dark-blue")

        # Variables de l'application
        self.settings_menu_is_open = False
        self.settings_menu_width = 300
        
        # Configurer le poids des lignes et colonnes pour le redimensionnement
        self.grid_rowconfigure(0, weight=0) # Ligne du header
        self.grid_rowconfigure(1, weight=1) # Ligne du contenu principal
        self.grid_columnconfigure(0, weight=0, minsize=350) # Panneau de gauche
        self.grid_columnconfigure(1, weight=1) # Panneau de droite (Échiquier)
        self.grid_columnconfigure(2, weight=0, minsize=0) # Panneau des paramètres (initialement caché)

        create_widgets(self)
        create_settings_menu(self)

        self.PRELOADED_PIECES = {}

        # Preload images safely after root exists
        self.after(50, lambda: self.preload_piece_images(square_size=70))

    def preload_piece_images(self, square_size=70):
        for symbol, name in PIECES_MAP.items():
            try:
                img = Image.open(f"{PIECES_PATH}{name}.png")
                img = img.resize((square_size, square_size), Image.LANCZOS)
                self.PRELOADED_PIECES[symbol] = ImageTk.PhotoImage(img, master=self)
            except FileNotFoundError:
                print(f"Erreur : {PIECES_PATH}{name}.png non trouvé")
        print("✅ Pièces préchargées")
