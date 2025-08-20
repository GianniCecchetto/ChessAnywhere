import requests

def fetch_data(server_url: str, resource: str) -> dict:
    response = requests.get(f"{server_url}/{resource}", timeout=5)
    response.raise_for_status()  # raises error on 4xx/5xx
    return response.json()

def send_data(server_url: str, resource: str, payload: dict) -> dict:
    response = requests.post(f"{server_url}/{resource}", json=payload, timeout=5)
    response.raise_for_status()
    return 0 #response.json()
    
