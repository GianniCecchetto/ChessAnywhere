import chess
from ..gui.draw_board import draw_chessboard
from .local_func import process_game_events
from .online_func import process_online_game_events, start_polling

# État de jeu partagé
game_state = {
    'board': None,
    'container': None,
    'player_color': None,
    'start_square': None,
    'end_square': None,
    'client': None,
    'game_id': None,
    'online_move': None,
    'local_move': None,
    'last_move': None,
    'illegal_move_pending': False, 
    'legal_moves_matrix': None,   
}

def start_online_game(board_container, board: chess.Board, player_color, client, game_id):
    print("===== starting online game =====")

    game_state['board'] = board
    game_state['container'] = board_container
    game_state['player_color'] = player_color
    game_state['start_square'] = None
    game_state['client'] = client
    game_state['game_id'] = game_id

    stream = client.board.stream_game_state(game_id)
    for event in stream:
        moves = event.get('state', {}).get('moves', '')
        if moves:
            for uci in moves.split():
                board.push_uci(uci)
                game_state['last_move'] = uci
        break
    
    draw_chessboard(board_container, board=board, player_color=player_color)

    start_polling(game_state)
    process_online_game_events(game_state)

def start_local_game(board_container, board, player_color):
    print("===== starting local game =====")
    
    game_state['board'] = board
    game_state['container'] = board_container
    game_state['player_color'] = player_color
    game_state['start_square'] = None
    game_state['illegal_move_pending'] = False
    game_state['legal_moves_matrix'] = None
    game_state['client'] = None
    game_state['game_id'] = None
    
    draw_chessboard(board_container, board=board, player_color=player_color)
    
    process_game_events(game_state)
