import requests

SERVER_URL = "https://chess-anywhere-server-cwhed8f0d7fsdba5.westeurope-01.azurewebsites.net"

def fetch_data(server_url: str, resource: str) -> dict:
    response = requests.get(f"{server_url}/{resource}", timeout=5)
    response.raise_for_status()  # raises error on 4xx/5xx
    return response.json()

def send_data(server_url: str, resource: str, payload: dict) -> dict:
    response = requests.post(f"{server_url}/{resource}", json=payload, timeout=5)
    response.raise_for_status()
    
    text = response.text.strip()
    if text.startswith("{") or text.startswith("["):  # likely JSON
        return response.json()
    else:
        # Wrap the string in a dict if you want consistent return type
        return text
    
def fetch_games():
    return fetch_data(SERVER_URL, "/api/games")

def create_game():
    return send_data(SERVER_URL, resource="/api/games/create", payload=None)
    