import customtkinter as ctk
import tkinter as tk
from . import app_color as c
from  .widgets_app import create_widgets
from  .settings_menu import create_settings_menu

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