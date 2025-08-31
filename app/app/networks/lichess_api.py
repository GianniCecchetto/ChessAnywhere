import os
import berserk
import customtkinter as ctk
import json

from .chess_anywhere_api import fetch_games, create_game

from platformdirs import user_config_dir


APP_NAME = "ChessAnywhere"
APP_AUTHOR = None  # optional, can be left None
CONFIG_DIR = user_config_dir(APP_NAME, APP_AUTHOR)
TOKEN_FILE = os.path.join(CONFIG_DIR, "token.json")

def save_token(token_data: str):
    os.makedirs(CONFIG_DIR, exist_ok=True)
    with open(TOKEN_FILE, "w", encoding="utf-8") as f:
        json.dump(token_data, f)
        print("Saved token at {CONFIG_DIR}")

def load_token() -> str | None:
    if os.path.exists(TOKEN_FILE):
        with open(TOKEN_FILE, "r", encoding="utf-8") as f:
            print("Load token from {CONFIG_DIR}")
            return json.load(f)
    return None

def fetch_game(board_container, client: berserk.Client, game_id):
    try:
        game_info = client.games.export(game_id)
        if game_info.get('status') != 'started':
            raise berserk.exceptions.ResponseError("Game not started yet")
        return game_info
    except berserk.exceptions.ResponseError as e:
        board_container.after(1000, lambda: fetch_game(board_container, client, game_id))

def refresh_games(parent, games_list_frame, board_container):
    from gui.join_game import join_online_game
    
    try:
        # Fetch JSON from your API
        games: dict = fetch_games()
        
    except Exception as e:
        print("Error fetching games:", e)
        games = ["❌ Error fetching data"]

    # Clear old buttons
    for widget in games_list_frame.winfo_children():
        widget.destroy()

    # Rebuild buttons
    if len(games) > 0:
        for game in games:
            if not isinstance(game, dict):
                print("Réponse inattendue:", game)
                continue
            btn = ctk.CTkButton(games_list_frame,
                                text=f"▶️ {game['id']} 📶",
                                corner_radius=15,
                                height=40,
                                fg_color="#333333",   # replace with c.GAME_LIST_BTN
                                text_color="white",
                                hover_color="#555555", # replace with c.DARK_BTN_HOVER
                                anchor="w",
                                command=lambda game_id=game['id']: join_online_game(board_container, game_id))
            btn.pack(fill="x", pady=5)

    # Re-run this function every 5s
    parent.after(5000, lambda: refresh_games(parent, games_list_frame, board_container))
