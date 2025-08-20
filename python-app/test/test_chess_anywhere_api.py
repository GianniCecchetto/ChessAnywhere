import pytest
import requests
from unittest.mock import patch
from app.chess_anywhere_api import fetch_data

def test_fetch_data_success():
    mock_response = {"message": "Hello Client"}

    with patch("app.api.requests.get") as mock_get:
        mock_get.return_value.json.return_value = mock_response
        mock_get.return_value.status_code = 200
        result = fetch_data("http://fake-server.com", "status")

    assert result == {"message": "Hello Client"}

def test_fetch_data_error():
    with patch("app.api.requests.get") as mock_get:
        mock_get.side_effect = requests.exceptions.ConnectionError

        with pytest.raises(requests.exceptions.ConnectionError):
            fetch_data("http://fake-server.com", "status")
