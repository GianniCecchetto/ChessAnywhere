import os
import berserk
import customtkinter as ctk

from gui.join_game import join_online_game
from .chess_anywhere_api import *

SERVER_URL = "http://localhost:8000"

def connect(token: str):
    session = berserk.TokenSession(token)
    return berserk.Client(session=session)

def fetch_games():
    

def refresh_games(parent, games_list_frame, board_container):
    # Example: this should call Lichess API later
    games = ["Ali's game", "Nathan's game", "Gianni's game", "Thomas's game"]
    
    try:
        # Fetch JSON from your API
        data = fetch_data(SERVER_URL, "api/challenges")

        # Suppose your API returns something like:
        # { "games": ["Ali's game", "Nathan's game", ...] }
        games = data.get("games", [])

    except Exception as e:
        print("Error fetching games:", e)
        games = ["❌ Error fetching data"]

    # Clear old buttons
    for widget in games_list_frame.winfo_children():
        widget.destroy()

    # Rebuild buttons
    for game in games:
        btn = ctk.CTkButton(games_list_frame,
                            text=f"▶️ {game} 📶",
                            corner_radius=15,
                            height=40,
                            fg_color="#333333",   # replace with c.GAME_LIST_BTN
                            text_color="white",
                            hover_color="#555555", # replace with c.DARK_BTN_HOVER
                            anchor="w",
                            command=lambda g=game: join_online_game(g, board_container))
        btn.pack(fill="x", pady=5)

    # Re-run this function every 5s
    parent.after(5000, lambda: refresh_games(parent, games_list_frame, board_container))
