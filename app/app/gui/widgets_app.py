import customtkinter as ctk
import tkinter as tk
from . import app_color as c    
from PIL import Image
from .draw_board import draw_chessboard
from .settings_menu import toggle_settings_menu
from .join_game import join_online_game, create_online_game, create_local_game
from ..networks.lichess_api import save_token
from ..uart import uart_com
import threading
import os
import time

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOGO_PATH = os.path.join(BASE_DIR, "assets", "logo", "Logo_ChessAnywhere_cropped.png")

def create_widgets(app):
    """
    Crée tous les widgets de l'interface utilisateur principale.
    """
    # ==== HEADER ====
    app.header = ctk.CTkFrame(app, height=60, fg_color=c.HEADER_BG, corner_radius=0)
    app.header.grid(row=0, column=0, columnspan=3, sticky="ew")
    logo_image = ctk.CTkImage(
        light_image=Image.open(LOGO_PATH), 
        dark_image=Image.open(LOGO_PATH), 
        size=(193, 103) 
    )

    logo_label = ctk.CTkLabel(app.header, image=logo_image, text="")
    logo_label.image = logo_image
    logo_label.pack(side="left", padx=20, pady=5)

    # Bouton Paramètres
    app.settings_btn = ctk.CTkButton(app.header, text="⚙️",font=("Arial", 40) ,width=80, height=80, corner_radius=20,
                                     fg_color=c.SETTINGS_BTN, text_color="white",
                                     hover_color=c.DARK_BTN_HOVER,
                                     command=lambda :toggle_settings_menu(app))
    app.settings_btn.pack(side="right", padx=10, pady=2)

    # ==== CONTENU PRINCIPAL ====
    # Frame principale pour le contenu (gauche et droite)
    app.main_content_frame = ctk.CTkFrame(app, fg_color=c.BG_COLOR)
    app.main_content_frame.grid(row=1, column=0, columnspan=2, sticky="nsew", padx=20, pady=20)
    
    app.main_content_frame.grid_rowconfigure(0, weight=1) 
    app.main_content_frame.grid_rowconfigure(1, weight=0) 
    app.main_content_frame.grid_columnconfigure(0, weight=0, minsize=350) 
    app.main_content_frame.grid_columnconfigure(1, weight=1) 
    
    # ==== PANNEAU DE GAUCHE PRINCIPAL (ARRONDI GRIS) ====
    app.left_panel = ctk.CTkFrame(app.main_content_frame, fg_color=c.LEFT_PANEL_BG, corner_radius=15)
    app.left_panel.grid(row=0, column=0, sticky="nsew", padx=(0, 10), pady=(0, 10))
    app.left_panel.grid_rowconfigure(0, weight=0)
    app.left_panel.grid_rowconfigure(1, weight=0)
    app.left_panel.grid_rowconfigure(2, weight=1) 
    app.left_panel.grid_columnconfigure(0, weight=1)
    app.left_panel.pack_propagate(False)

    # Section "Connecting via Lichess Link"
    connect_label_frame = ctk.CTkFrame(app.left_panel, fg_color="transparent")
    connect_label_frame.grid(row=0, column=0, sticky="ew", padx=20, pady=(20, 5))
    connect_label_frame.columnconfigure(0, weight=0)
    connect_label_frame.columnconfigure(1, weight=1)
    
    app.arrow_label = ctk.CTkLabel(connect_label_frame, text="➔", font=("Arial", 20, "bold"))
    app.arrow_label.grid(row=0, column=0, padx=(0, 5))
    
    app.connect_label = ctk.CTkLabel(connect_label_frame, text="Connecting via Lichess Link", 
                                 font=("Arial", 14, "bold"), justify="left", anchor="w")
    app.connect_label.grid(row=0, column=1, sticky="ew")
    
    connect_frame = ctk.CTkFrame(app.left_panel, fg_color="transparent")
    connect_frame.grid(row=1, column=0, sticky="ew", padx=20, pady=(5, 10))
    connect_frame.columnconfigure(0, weight=1)
    connect_frame.columnconfigure(1, weight=0)
    connect_frame.columnconfigure(2, weight=0)

    app.link_entry = ctk.CTkEntry(connect_frame, placeholder_text="Link", corner_radius=15)
    app.link_entry.grid(row=0, column=0, sticky="ew", padx=(0, 5))
    
    app.clear_btn = ctk.CTkButton(connect_frame, text="X", width=30, height=30, corner_radius=15, 
                                 fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER,
                                 command=lambda: app.link_entry.delete(0, "end"))
    app.clear_btn.grid(row=0, column=1)

    app.connect_btn = ctk.CTkButton(connect_frame, text="Connect", width=80, corner_radius=15,
                                     fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER, state="enable")
    app.connect_btn.grid(row=0, column=2, padx=(5, 0))

    # Section "Joinable games"
    app.games_label = ctk.CTkLabel(app.left_panel, text="Joinable games", font=("Arial", 13, "bold"))
    app.games_label.grid(row=2, column=0, sticky="w", padx=20, pady=(5, 5))

    app.games_list_frame = ctk.CTkScrollableFrame(app.left_panel, fg_color="transparent", corner_radius=15)
    app.games_list_frame.grid(row=2, column=0, sticky="nsew", padx=20, pady=(0, 10))
    
    # Liste pour stocker les boutons de jeux en ligne
    app.online_game_buttons = []

    # ==== ZONE DES BOUTONS DE JEU EN BAS ====
    game_buttons_frame = ctk.CTkFrame(app.main_content_frame, fg_color="transparent")
    game_buttons_frame.grid(row=1, column=0, sticky="ew", padx=10, pady=(10, 5))
    game_buttons_frame.columnconfigure(0, weight=1)
    game_buttons_frame.columnconfigure(1, weight=1)
    
    # Bouton Local Game
    app.local_btn = ctk.CTkButton(game_buttons_frame, text="👥 Local Game", corner_radius=15,
                                 fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER,
                                 command=lambda: create_local_game(board_container), state="disabled")
    app.local_btn.grid(row=0, column=0, sticky="ew", padx=(10, 5))

    # bouton Host Game
    app.host_btn = ctk.CTkButton(game_buttons_frame, text="🔗 Host Game", corner_radius=15,
                                 fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER,
                                 command=lambda: create_online_game(board_container), state="disabled")
    app.host_btn.grid(row=0, column=1, sticky="ew", padx=(5, 10))

    # Section Token
    token_frame = ctk.CTkFrame(app.main_content_frame, fg_color="transparent")
    token_frame.grid(row=2, column=0, sticky="ew", padx=20, pady=(5, 20))
    token_frame.columnconfigure(0, weight=1)
    token_frame.columnconfigure(1, weight=0)

    app.token_entry = ctk.CTkEntry(token_frame, placeholder_text="Token", corner_radius=15)
    app.token_entry.grid(row=0, column=0, sticky="ew", padx=(0, 5))
    
    app.save_btn = ctk.CTkButton(token_frame, text="💾 Save", corner_radius=15,
                                 fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER,
                                 command=lambda: save_token(app.token_entry.get()))
    app.save_btn.grid(row=0, column=1)

    # ==== PANNEAU DE DROITE (Échiquier) ====
    app.right_panel = ctk.CTkFrame(app.main_content_frame, fg_color=c.BG_COLOR, corner_radius=15)
    app.right_panel.grid(row=0, column=1, rowspan=3, sticky="nsew")
    app.right_panel.pack_propagate(False)
    app.right_panel.grid_rowconfigure(0, weight=0)
    app.right_panel.grid_rowconfigure(1, weight=1)
    app.right_panel.grid_columnconfigure(0, weight=0)
    app.right_panel.grid_columnconfigure(1, weight=1)
    
    status_frame = ctk.CTkFrame(app.right_panel, fg_color="transparent")
    status_frame.grid(row=0, column=0, columnspan=2, pady=(10, 0), sticky="n")
    
    app.status_label = ctk.CTkLabel(status_frame, text="Connection Status :", font=("Arial", 14))
    app.status_label.pack(side="left", padx=5)
    
    app.connection_state_frame = ctk.CTkFrame(status_frame, corner_radius=5, border_width=2, border_color="red", fg_color="#ffecec")
    app.connection_state_frame.pack(side="left", padx=5)
    
    app.connection_state = ctk.CTkLabel(app.connection_state_frame, text="Not connected", text_color="red", 
                                     font=("Arial", 14, "bold"), fg_color="transparent")
    app.connection_state.pack(padx=5, pady=2)
    
    port_frame = ctk.CTkFrame(status_frame, fg_color="transparent")
    port_frame.pack(side="left", padx=(10, 5))
    
    # Créer le ComboBox
    port_combo = ctk.CTkComboBox(port_frame, values=[], width=120)
    port_combo.pack(side="left", fill="x", expand=True)

    # bouton pour la connexion COM
    app.com_connect_btn = ctk.CTkButton(port_frame, text="Connect COM", corner_radius=15,
                                         fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER, state="disabled", width=120)
    app.com_connect_btn.pack(side="left", padx=(5, 0))
    
    board_container = ctk.CTkFrame(app.right_panel, fg_color="transparent")
    board_container.grid(row=1, column=1, sticky="")
    board_container.grid_rowconfigure(0, weight=1)
    board_container.grid_columnconfigure(0, weight=1)
    
    draw_chessboard(board_container)

    def scan_and_update_games():
        """Démarre un thread pour fetch les games/challengs en cours."""
        threading.Thread(target=_fetch_games_and_update, daemon=True).start()

    def _fetch_games_and_update():
        from ..networks.chess_anywhere_api import fetch_games
        while True:
            try:
                games: dict = fetch_games()
            except Exception as e:
                games = {}
                print("Error fetching games")
            
            app.after(0, lambda: update_game_buttons(games))
            time.sleep(1)

    def update_game_buttons(games):
        """Met à jour l'UI avec les boutons de parties."""
        # Clear old buttons
        for btn in app.online_game_buttons:
            btn.destroy()

        if len(games) > 0:
            for game in games:
                if not isinstance(game, dict):
                    print("Réponse inattendue:", game)
                    continue

                challenger = game.get('challenger', {})
                if challenger is None:
                    challenger_name = "Unknown"
                else:
                    challenger_name = challenger.get('name', "Unknown")
                
                dest_user = game.get('destUser', {})
                if dest_user is None:
                    dest_user_name = "Unknown"
                else:
                    dest_user_name = dest_user.get('name', "Unknown")

                app.btn = ctk.CTkButton(
                    app.games_list_frame,
                    text=f"▶️ {game['id']} {challenger_name} vs {dest_user_name}",
                    corner_radius=15,
                    height=40,
                    fg_color="#628092",
                    text_color="white",
                    hover_color="#555555",
                    anchor="w",
                    command=lambda game_id=game['id']: join_online_game(board_container, game_id)
                )
                app.btn.pack(fill="x", pady=5)
                app.online_game_buttons.append(app.btn)

    scan_and_update_games()

    def update_connection_status(is_connected):
        """Met à jour l'état de la connexion et l'état des boutons."""
        if is_connected:
            app.connection_state.configure(text="Connected", text_color="green")
            app.connection_state_frame.configure(border_color="green", fg_color="#e3f9e3")
            app.local_btn.configure(state="normal")
            app.host_btn.configure(state="normal")
            for btn in app.online_game_buttons:
                if btn:
                    btn.configure(state="disabled")
        else:
            app.connection_state.configure(text="Not connected", text_color="red")
            app.connection_state_frame.configure(border_color="red", fg_color="#ffecec")
            app.local_btn.configure(state="disabled")
            app.host_btn.configure(state="disabled")
            for btn in app.online_game_buttons:
                if btn:
                    btn.configure(state="disabled")

    def connect_to_port():
        """Lance la connexion au port sélectionné."""
        selected_port = port_combo.get()
        if selected_port not in ["Select a COM port...", "No port found"]:
            print(f"Tentative de connexion au port: {selected_port}")
            app.com_connect_btn.configure(state="disabled")
            uart_com.set_serial_port(selected_port)
            update_connection_status(True)

    def on_port_selected(choice):
        if choice not in ["Select a COM port...", "No port found"]:
            app.com_connect_btn.configure(state="normal")
        else:
            app.com_connect_btn.configure(state="disabled")

    def scan_and_update_ports():
        full_ports = uart_com.get_available_ports()
        
        if not full_ports:
            port_combo.configure(values=["No port found"])
            port_combo.set("No port found")
            on_port_selected("No port found")
        else:
            formatted_ports = [p.split(' ')[0] for p in full_ports]
            port_combo.configure(values=formatted_ports)
            if port_combo.get() not in formatted_ports:
                port_combo.set("Select a COM port...")
            on_port_selected(port_combo.get())
        
        app.after(1000, scan_and_update_ports)

    app.com_connect_btn.configure(command=connect_to_port)
    port_combo.bind("<<ComboboxSelected>>", lambda e: on_port_selected(port_combo.get()))
    update_connection_status(False)
    scan_and_update_ports()
