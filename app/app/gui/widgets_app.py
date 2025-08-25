import customtkinter as ctk
import tkinter as tk
from . import app_color as c    
from .draw_board import draw_chessboard
from .settings_menu import toggle_settings_menu
from .join_game import join_online_game
from .join_game import create_local_game


def create_widgets(app):
        """
        Crée tous les widgets de l'interface utilisateur principale.
        """
        # ==== HEADER ====
        header = ctk.CTkFrame(app, height=60, fg_color=c.HEADER_BG, corner_radius=0)
        header.grid(row=0, column=0, columnspan=3, sticky="ew")
        
        logo = ctk.CTkLabel(header, text="  ♞ Chess Anywhere", 
                             font=("Arial", 26, "bold"), text_color="white")
        logo.pack(side="left", padx=20, pady=10)

        # Bouton Paramètres
        settings_btn = ctk.CTkButton(header, text="⚙️", width=40, height=40, corner_radius=20,
                                     fg_color=c.SETTINGS_BTN, text_color="white",
                                     hover_color=c.DARK_BTN_HOVER,
                                     command=lambda :toggle_settings_menu(app))
        settings_btn.pack(side="right", padx=20, pady=10)

        # ==== CONTENU PRINCIPAL ====
        # Frame principale pour le contenu (gauche et droite)
        main_content_frame = ctk.CTkFrame(app, fg_color=c.BG_COLOR)
        main_content_frame.grid(row=1, column=0, columnspan=2, sticky="nsew", padx=20, pady=20)
        
        # Le panneau principal de gauche et le panneau du bas sont maintenant dans des lignes séparées
        main_content_frame.grid_rowconfigure(0, weight=1) # Panneau de gauche principal
        main_content_frame.grid_rowconfigure(1, weight=0) # Zone pour le bouton et le token
        main_content_frame.grid_columnconfigure(0, weight=0, minsize=350) # Colonne de gauche
        main_content_frame.grid_columnconfigure(1, weight=1) # Colonne de droite (Échiquier)
        
        # ==== PANNEAU DE GAUCHE PRINCIPAL (ARRONDI GRIS) ====
        left_panel = ctk.CTkFrame(main_content_frame, fg_color=c.LEFT_PANEL_BG, corner_radius=15)
        # Ajout de padding pour le séparer du bouton du bas
        left_panel.grid(row=0, column=0, sticky="nsew", padx=(0, 10), pady=(0, 10))
        left_panel.grid_rowconfigure(0, weight=0)
        left_panel.grid_rowconfigure(1, weight=0)
        left_panel.grid_rowconfigure(2, weight=1) 
        left_panel.grid_columnconfigure(0, weight=1)
        left_panel.pack_propagate(False)

        # Section "Connecting via Lichess Link"
        connect_label_frame = ctk.CTkFrame(left_panel, fg_color="transparent")
        connect_label_frame.grid(row=0, column=0, sticky="ew", padx=20, pady=(20, 5))
        connect_label_frame.columnconfigure(0, weight=0)
        connect_label_frame.columnconfigure(1, weight=1)
        
        arrow_label = ctk.CTkLabel(connect_label_frame, text="➔", font=("Arial", 20, "bold"))
        arrow_label.grid(row=0, column=0, padx=(0, 5))
        
        connect_label = ctk.CTkLabel(connect_label_frame, text="Connecting via Lichess Link", 
                                     font=("Arial", 14, "bold"), justify="left", anchor="w")
        connect_label.grid(row=0, column=1, sticky="ew")
        
        connect_frame = ctk.CTkFrame(left_panel, fg_color="transparent")
        connect_frame.grid(row=1, column=0, sticky="ew", padx=20, pady=(5, 10))
        connect_frame.columnconfigure(0, weight=1)
        connect_frame.columnconfigure(1, weight=0)
        connect_frame.columnconfigure(2, weight=0)

        link_entry = ctk.CTkEntry(connect_frame, placeholder_text="Link", corner_radius=15)
        link_entry.grid(row=0, column=0, sticky="ew", padx=(0, 5))
        
        clear_btn = ctk.CTkButton(connect_frame, text="X", width=30, height=30, corner_radius=15, 
                                  fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER,
                                  command=lambda: link_entry.delete(0, "end"))
        clear_btn.grid(row=0, column=1)

        connect_btn = ctk.CTkButton(connect_frame, text="Connect", width=80, corner_radius=15,
                                    fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER)
        connect_btn.grid(row=0, column=2, padx=(5, 0))

        # Section "Joinable games"
        games_label = ctk.CTkLabel(left_panel, text="Joinable games", font=("Arial", 13, "bold"))
        games_label.grid(row=2, column=0, sticky="w", padx=20, pady=(5, 5))

        games_list_frame = ctk.CTkScrollableFrame(left_panel, fg_color="transparent", corner_radius=15)
        games_list_frame.grid(row=2, column=0, sticky="nsew", padx=20, pady=(0, 10))
            
        # ==== ZONE DU BOUTON ET DU TOKEN EN BAS ====

        # Section Token
        token_frame = ctk.CTkFrame(main_content_frame, fg_color="transparent")
        token_frame.grid(row=2, column=0, sticky="ew", padx=20, pady=(5, 20))
        token_frame.columnconfigure(0, weight=1)
        token_frame.columnconfigure(1, weight=0)

        token_entry = ctk.CTkEntry(token_frame, placeholder_text="Token", corner_radius=15)
        token_entry.grid(row=0, column=0, sticky="ew", padx=(0, 5))
        
        save_btn = ctk.CTkButton(token_frame, text="💾 Save", corner_radius=15,
                                 fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER)
        save_btn.grid(row=0, column=1)

        # ==== PANNEAU DE DROITE (Échiquier) ====
        app.right_panel = ctk.CTkFrame(main_content_frame, fg_color=c.BG_COLOR, corner_radius=15)
        app.right_panel.grid(row=0, column=1, rowspan=3, sticky="nsew") # Le panneau de droite s'étend sur plusieurs lignes
        app.right_panel.pack_propagate(False)
        app.right_panel.grid_rowconfigure(0, weight=0)
        app.right_panel.grid_rowconfigure(1, weight=1)
        # Ajout d'une colonne avec un poids pour décaler l'échiquier vers la droite
        app.right_panel.grid_columnconfigure(0, weight=0)
        app.right_panel.grid_columnconfigure(1, weight=1)
        
        status_frame = ctk.CTkFrame(app.right_panel, fg_color="transparent")
        status_frame.grid(row=0, column=0, columnspan=2, pady=(10, 0), sticky="n")
        
        status_label = ctk.CTkLabel(status_frame, text="Connection Status :", font=("Arial", 14))
        status_label.pack(side="left", padx=5)
        
        connection_state_frame = ctk.CTkFrame(status_frame, corner_radius=5, border_width=2, border_color="red", fg_color="#ffecec")
        connection_state_frame.pack(side="left", padx=5)

        connection_state = ctk.CTkLabel(connection_state_frame, text="Not connected", text_color="red", 
                                        font=("Arial", 14, "bold"), fg_color="transparent")
        connection_state.pack(padx=5, pady=2)
        
        board_container = ctk.CTkFrame(app.right_panel, fg_color="transparent")
        # Le conteneur du plateau est maintenant dans la deuxième colonne pour le décaler à droite
        board_container.grid(row=1, column=1, sticky="") # Pas de sticky pour ne pas étirer
        board_container.grid_rowconfigure(0, weight=1)
        board_container.grid_columnconfigure(0, weight=1)
        
        # ==== Liste des parties en ligne ====
        games = ["Ali's game", "Nathan's game", "Gianni's game", "Thomas's game"]
        for i, game in enumerate(games):
            btn = ctk.CTkButton(games_list_frame, 
                                text=f"▶️ {game} 📶", 
                                corner_radius=15, 
                                height=40, 
                                fg_color=c.GAME_LIST_BTN, 
                                text_color="white", 
                                hover_color=c.DARK_BTN_HOVER,
                                anchor="w",
                                command=lambda g=game: join_online_game(g,board_container))
            btn.pack(fill="x", pady=5)

        # Bouton Local Game
        local_btn = ctk.CTkButton(main_content_frame, text="👥 Local Game", corner_radius=15,
                                  fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER,command=lambda :create_local_game(board_container))

        local_btn.grid(row=1, column=0, sticky="ew", padx=(10, 20), pady=(10, 5))

        draw_chessboard(board_container)