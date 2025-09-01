import os
import berserk
import customtkinter as ctk
import json

from .chess_anywhere_api import fetch_games, create_game, fetch_data

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

def load_token() -> str:
    if os.path.exists(TOKEN_FILE):
        with open(TOKEN_FILE, "r", encoding="utf-8") as f:
            print("Load token from {CONFIG_DIR}")
            return json.load(f)
    return ""

def verify_token(token: str) -> bool:
    headers = {
        "Authorization": f"Bearer {token}"
    }
    token_info = fetch_data("http://lichess.org", "/api/account", headers=headers)
    if token_info.get('id', {}):
        return True
    return False

def fetch_game(board_container, client: berserk.Client, game_id):
    try:
        game_info = client.games.export(game_id)
        if game_info.get('status') == 'created':
            raise RuntimeError("Game not started yet")
        return game_info
    except berserk.exceptions.ResponseError as e:
        board_container.after(1000, lambda: fetch_game(board_container, client, game_id))
