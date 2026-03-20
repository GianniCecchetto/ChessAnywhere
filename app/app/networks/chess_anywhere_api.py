import requests

import os
import json
from platformdirs import user_config_dir

SERVER_URL = "https://chess-anywhere-server-cwhed8f0d7fsdba5.westeurope-01.azurewebsites.net"
SERVER_URL = "http://localhost:7000"  # for local testing

APP_NAME = "ChessAnywhere"
APP_AUTHOR = None  # optional, can be left None
CONFIG_DIR = user_config_dir(APP_NAME, APP_AUTHOR)
SERVER_URL_FILE = os.path.join(CONFIG_DIR, "server_url.json")

def fetch_data(server_url: str, resource: str, headers: dict | None= None) -> dict:
    response = requests.get(f"{server_url}/{resource}", headers=headers, timeout=5)
    return response.json()

def send_data(server_url: str, resource: str, headers: dict | None= None, payload: dict | None = None) -> dict:
    response = requests.post(f"{server_url}/{resource}", headers=headers, json=payload, timeout=5)
    
    text = response.text.strip()
    if text.startswith("{") or text.startswith("["):  # likely JSON
        return response.json()
    else:
        # Wrap the string in a dict if you want consistent return type
        return {"response": text}
    
def fetch_games():
    server_url = SERVER_URL
    if os.path.exists(SERVER_URL_FILE):
        with open(SERVER_URL_FILE, "r", encoding="utf-8") as f:
            print("Load server URL from {CONFIG_DIR}")
            server_url = json.load(f)
    return fetch_data(server_url, "/api/games")

def create_game():
    server_url = SERVER_URL
    if os.path.exists(SERVER_URL_FILE):
        with open(SERVER_URL_FILE, "r", encoding="utf-8") as f:
            print("Load server URL from {CONFIG_DIR}")
            server_url = json.load(f)
    return send_data(server_url, resource="/api/games/create")
    
def join_game(game_id, user_name):
    server_url = SERVER_URL
    if os.path.exists(SERVER_URL_FILE):
        with open(SERVER_URL_FILE, "r", encoding="utf-8") as f:
            print("Load server URL from {CONFIG_DIR}")
            server_url = json.load(f)
    return send_data(server_url, resource=f"/api/games/join/{game_id}/{user_name}")
