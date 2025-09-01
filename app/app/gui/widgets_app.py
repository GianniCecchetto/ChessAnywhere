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

BASE_DIR = os.path.dirname(os.path.dirname(os.path.abspath(__file__)))
LOGO_PATH = os.path.join(BASE_DIR, "assets", "logo", "Logo_ChessAnywhere_cropped.png")

def create_widgets(app):
    """
    Crée tous les widgets de l'interface utilisateur principale.
    """
    # ==== HEADER ====
    header = ctk.CTkFrame(app, height=60, fg_color=c.HEADER_BG, corner_radius=0)
    header.grid(row=0, column=0, columnspan=3, sticky="ew")
    logo_image = ctk.CTkImage(
        light_image=Image.open(LOGO_PATH), 
        dark_image=Image.open(LOGO_PATH), 
        size=(193, 103) 
    )

    logo_label = ctk.CTkLabel(header, image=logo_image, text="")
    logo_label.image = logo_image
    logo_label.pack(side="left", padx=20, pady=5)

    # Bouton Paramètres
    settings_btn = ctk.CTkButton(header, text="⚙️",font=("Arial", 40) ,width=80, height=80, corner_radius=20,
                                 fg_color=c.SETTINGS_BTN, text_color="white",
                                 hover_color=c.DARK_BTN_HOVER,
                                 command=lambda :toggle_settings_menu(app))
    settings_btn.pack(side="right", padx=10, pady=2)

    # ==== CONTENU PRINCIPAL ====
    # Frame principale pour le contenu (gauche et droite)
    main_content_frame = ctk.CTkFrame(app, fg_color=c.BG_COLOR)
    main_content_frame.grid(row=1, column=0, columnspan=2, sticky="nsew", padx=20, pady=20)
    
    main_content_frame.grid_rowconfigure(0, weight=1) 
    main_content_frame.grid_rowconfigure(1, weight=0) 
    main_content_frame.grid_columnconfigure(0, weight=0, minsize=350) 
    main_content_frame.grid_columnconfigure(1, weight=1) 
    
    # ==== PANNEAU DE GAUCHE PRINCIPAL (ARRONDI GRIS) ====
    left_panel = ctk.CTkFrame(main_content_frame, fg_color=c.LEFT_PANEL_BG, corner_radius=15)
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
                                 fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER, state="enable")
    connect_btn.grid(row=0, column=2, padx=(5, 0))

    # Section "Joinable games"
    games_label = ctk.CTkLabel(left_panel, text="Joinable games", font=("Arial", 13, "bold"))
    games_label.grid(row=2, column=0, sticky="w", padx=20, pady=(5, 5))

    games_list_frame = ctk.CTkScrollableFrame(left_panel, fg_color="transparent", corner_radius=15)
    games_list_frame.grid(row=2, column=0, sticky="nsew", padx=20, pady=(0, 10))
    
    # Liste pour stocker les boutons de jeux en ligne
    online_game_buttons = []

    # ==== ZONE DES BOUTONS DE JEU EN BAS ====
    game_buttons_frame = ctk.CTkFrame(main_content_frame, fg_color="transparent")
    game_buttons_frame.grid(row=1, column=0, sticky="ew", padx=10, pady=(10, 5))
    game_buttons_frame.columnconfigure(0, weight=1)
    game_buttons_frame.columnconfigure(1, weight=1)
    
    # Bouton Local Game
    local_btn = ctk.CTkButton(game_buttons_frame, text="👥 Local Game", corner_radius=15,
                             fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER,
                             command=lambda: create_local_game(board_container), state="disabled")
    local_btn.grid(row=0, column=0, sticky="ew", padx=(10, 5))

    # bouton Host Game
    host_btn = ctk.CTkButton(game_buttons_frame, text="🔗 Host Game", corner_radius=15,
                             fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER,
                             command=lambda: create_online_game(board_container), state="disabled")
    host_btn.grid(row=0, column=1, sticky="ew", padx=(5, 10))

    # Section Token
    token_frame = ctk.CTkFrame(main_content_frame, fg_color="transparent")
    token_frame.grid(row=2, column=0, sticky="ew", padx=20, pady=(5, 20))
    token_frame.columnconfigure(0, weight=1)
    token_frame.columnconfigure(1, weight=0)

    token_entry = ctk.CTkEntry(token_frame, placeholder_text="Token", corner_radius=15)
    token_entry.grid(row=0, column=0, sticky="ew", padx=(0, 5))
    
    save_btn = ctk.CTkButton(token_frame, text="💾 Save", corner_radius=15,
                             fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER,
                             command=lambda: save_token(token_entry.get()))
    save_btn.grid(row=0, column=1)

    # ==== PANNEAU DE DROITE (Échiquier) ====
    app.right_panel = ctk.CTkFrame(main_content_frame, fg_color=c.BG_COLOR, corner_radius=15)
    app.right_panel.grid(row=0, column=1, rowspan=3, sticky="nsew")
    app.right_panel.pack_propagate(False)
    app.right_panel.grid_rowconfigure(0, weight=0)
    app.right_panel.grid_rowconfigure(1, weight=1)
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
    
    port_frame = ctk.CTkFrame(status_frame, fg_color="transparent")
    port_frame.pack(side="left", padx=(10, 5))
    
    # Créer le ComboBox
    port_combo = ctk.CTkComboBox(port_frame, values=[], width=120)
    port_combo.pack(side="left", fill="x", expand=True)

    # bouton pour la connexion COM
    com_connect_btn = ctk.CTkButton(port_frame, text="Connect COM", corner_radius=15,
                                    fg_color=c.DARK_BTN_BG, text_color="white", hover_color=c.DARK_BTN_HOVER, state="disabled", width=120)
    com_connect_btn.pack(side="left", padx=(5, 0))
    
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
        try:
            games: dict = fetch_games()
        except Exception as e:
            print("Error fetching games:", e)
            games = []

        # Schedule the UI update back on the Tkinter main thread
        app.after(0, lambda: update_game_buttons(games))

    def update_game_buttons(games):
        """Met à jour l'UI avec les boutons de parties."""
        # Clear old buttons
        for btn in online_game_buttons:
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

                btn = ctk.CTkButton(
                    games_list_frame,
                    text=f"▶️ {game['id']} {challenger_name} vs {dest_user_name}",
                    corner_radius=15,
                    height=40,
                    fg_color="#333333",   # replace with c.GAME_LIST_BTN
                    text_color="white",
                    hover_color="#555555", # replace with c.DARK_BTN_HOVER
                    anchor="w",
                    command=lambda game_id=game['id']: join_online_game(board_container, game_id)
                )
                btn.pack(fill="x", pady=5)
                online_game_buttons.append(btn)

        # Reschedule the next scan
        app.after(1000, scan_and_update_games)
    
    scan_and_update_games()

    def update_connection_status(is_connected):
        """Met à jour l'état de la connexion et l'état des boutons."""
        if is_connected:
            connection_state.configure(text="Connected", text_color="green")
            connection_state_frame.configure(border_color="green", fg_color="#e3f9e3")
            local_btn.configure(state="normal")
            host_btn.configure(state="normal")
            for btn in online_game_buttons:
                btn.configure(state="normal")
        else:
            connection_state.configure(text="Not connected", text_color="red")
            connection_state_frame.configure(border_color="red", fg_color="#ffecec")
            local_btn.configure(state="disabled")
            host_btn.configure(state="disabled")
            for btn in online_game_buttons:
                btn.configure(state="disabled")

    def connect_to_port():
        """Lance la connexion au port sélectionné."""
        selected_port = port_combo.get()
        if selected_port not in ["Select a COM port...", "No port found"]:
            print(f"Tentative de connexion au port: {selected_port}")
            com_connect_btn.configure(state="disabled")
            uart_com.set_serial_port(selected_port)
            update_connection_status(True)

    def on_port_selected(choice):
        if choice not in ["Select a COM port...", "No port found"]:
            com_connect_btn.configure(state="normal")
        else:
            com_connect_btn.configure(state="disabled")

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

    com_connect_btn.configure(command=connect_to_port)
    port_combo.bind("<<ComboboxSelected>>", lambda e: on_port_selected(port_combo.get()))
    update_connection_status(False)
    scan_and_update_ports()
