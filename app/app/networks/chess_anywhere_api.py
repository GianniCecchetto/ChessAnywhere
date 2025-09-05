import requests

SERVER_URL = "https://chess-anywhere-server-cwhed8f0d7fsdba5.westeurope-01.azurewebsites.net"

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
    return fetch_data(SERVER_URL, "/api/games")

def create_game():
    return send_data(SERVER_URL, resource="/api/games/create")
    
def join_game(game_id, user_name):
    return send_data(SERVER_URL, resource=f"/api/games/join/{game_id}/{user_name}")
