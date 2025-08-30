import os
import berserk
import customtkinter as ctk

from .chess_anywhere_api import *

SERVER_URL = "http://localhost:7000"

def connect(token: str):
    session = berserk.TokenSession(token)
    return berserk.Client(session=session)

def fetch_game(board_container, client: berserk.Client, game_id):
    try:
        return client.games.export(game_id)
    except berserk.exceptions.ResponseError:
        board_container.after(500, lambda: fetch_game(board_container, client, game_id))  # wait half a second

def refresh_games(parent, games_list_frame, board_container):
    from gui.join_game import join_online_game
    # Example: this should call Lichess API later
    games = ["Ali's game", "Nathan's game", "Gianni's game", "Thomas's game"]
    
    try:
        # Fetch JSON from your API
        games = fetch_data(SERVER_URL, "api/games")
        
    except Exception as e:
        print("Error fetching games:", e)
        games = ["❌ Error fetching data"]

    # Clear old buttons
    for widget in games_list_frame.winfo_children():
        widget.destroy()

    # Rebuild buttons
    for game in games:
        try:
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
        except:
            print("No valid game id found")

    # Re-run this function every 5s
    parent.after(5000, lambda: refresh_games(parent, games_list_frame, board_container))
