from game_logic.game_loop import *

def process_online_game_events():
    """
    Boucle de jeu principale pour la partie en ligne. Gère les tours de manière non bloquante.
    """
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']
    
    # Vérification de la fin de partie
    if board.is_checkmate() or board.is_stalemate():
        print("Partie terminée.")
        print(board, "\n")
        if board.is_checkmate():
            print("Échec et mat ! Le joueur", "Blanc" if board.turn == chess.WHITE else "Noir", "a gagné.")
        elif board.is_stalemate():
            print("Partie nulle par pat.")
        return

    if game_state['current_turn'] == 'local':
        print("C'est à votre tour. Attente d'un coup via l'UART.")
        handle_local_move_event()
    elif game_state['current_turn'] == 'online':
        print("C'est au tour de l'adversaire en ligne. Récupération du coup via l'API.")
        handle_online_move_event()

    board_container.after(50, process_online_game_events)

def handle_online_move_event():
    """
    Simule la récupération d'un coup d'un adversaire en ligne et l'applique au plateau.
    """
    board = game_state['board']
    board_container = game_state['container']
    player_color = game_state['player_color']

    # --- SIMULATION DE L'APPEL API ---
    # move_from_api = api_client.get_opponent_move()

    
    # Placeholder pour récupérer un coup de l'API
    online_move = None
    
    print("Tentative de récupérer un coup de l'API...")
    
    if True: # Remplacez par une condition réelle
        # Simuler un coup de l'adversaire pour le test
        legal_moves = list(board.legal_moves)
        if legal_moves:
            online_move = legal_moves[0] # Ou choisissez un coup de manière plus intelligente

    if online_move:
        print(f"Coup de l'adversaire reçu : {online_move.uci()}")
        
        try:
            board.push(online_move)
            print(f"Coup légal joué par l'adversaire : {online_move.uci()}")
            draw_chessboard(board_container, board=board, player_color=player_color)
            game_state['current_turn'] = 'local' # Passer le tour au joueur local
            print("C'est à votre tour de jouer.")
        except Exception as e:
            print(f"Erreur lors de l'application du coup de l'adversaire : {e}")
            # Gérer le cas où le coup de l'adversaire est invalide
            game_state['current_turn'] = 'local' # Revenir au tour du joueur local pour correction
    else:
        print("Pas de coup de l'adversaire disponible. Attente du prochain tour.")
