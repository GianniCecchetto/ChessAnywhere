# ChessAnywhere

# Spécification projet – Échiquier connecté

## Problématique

Comment permettre à un joueur d’utiliser un véritable échiquier physique pour disputer des parties d’échecs en ligne (via Lichess ou un serveur communautaire), en assurant une détection fiable et en temps réel des mouvements, une synchronisation fluide avec l’application PC et les services distants, tout en garantissant robustesse, faible latence et bonne expérience utilisateur ?

## Objectif du projet

Permettre à un joueur d’utiliser un vrai plateau d’échecs et de vraies pièces pour jouer des parties connectées.
Les positions réelles des pièces sont lues par le plateau (capteurs reed) et transmises via USB à une application sur PC.
L’application permet :

- de rejoindre une partie Lichess via un URL,

- de créer/rejoindre des parties communautaires via un serveur central listant les parties en attente.

Le système valide les mouvements (localement et/ou via Lichess), affiche l’état du jeu et gère la communication entre le plateau, l'application, l'API Lichess et le serveur.

## Portée

Inclus :

- Firmware du plateau : lecture capteurs, protocole USB ou UART.
- Application PC : UI, intégration Lichess, interface USB.
- Serveur communautaire : liste des parties en attente, accès au partie.

## Acteurs

- <b>Utilisateur</b> : joueur local.
- <b>Opposant distant</b> : joueur via Lichess ou autre utilisateur connecté.
- <b>Serveur communautaire</b> : gère la liste des parties.
- <b>API Lichess</b> : fournit l’état et valide les coups.
- <b>Firmware plateau</b> : capteurs + microcontrôleur

## Scénarios d'utilisation

Scénarios d’utilisation :

- Rejoindre une partie Lichess via URL.
- Créer ou rejoindre une partie communautaire via serveur.

## Requirements fonctionnelles

### A. Plateau / Board

- **RF-A1** : Le plateau récupère les cases occupées et les envoie à l'application.  
- **RF-A2** : Le plateau attend une trame de la part de l'application pour modifier l'état des LED's.  
- **RF-A3** : Le plateau envoie une trame lorsque l'utilisateur a appuyé sur le bouton pour indiquer qu'il a effectué le mouvement.  

### B. Communication plateau ↔ PC

- **RF-B1** : Le plateau envoie les positions des pièces via USB. Lorsque l'utilisateur aura terminé son mouvement, il appuie sur un bouton.  
- **RF-B2** : Protocole USB et les datas seront récupérés sous forme d'une trame.  
- **RF-B3** : L'application peut récupérer toutes les cases occupées par le board.  

### C. Application PC

- **RF-C1** : Détection de connexion USB.  
- **RF-C2** : L’utilisateur saisit une URL Lichess ou se connecte via le mode communautaire.  
- **RF-C3** : Connexion à l’API Lichess pour récupérer l’état de la partie.  
- **RF-C4** : Les coups sont envoyés à l'API Lichess, si le coup est invalide Lichess nous renvoie une erreur.  
- **RF-C5** : L'application enverra les différentes commandes pour contrôler l'état des LED's.  
- **RF-C6** : Affichage échiquier, coups, horloge, état de partie.  
- **RF-C7** : Gestion des erreurs (USB, API) avec messages clairs.  
- **RF-C8** : Gestion de déconnexion de l'USB → reprise de la partie (optionnel à cause du temps à disposition).  

### D. Intégration Lichess

- **RF-D1** : Utilisation du compte Lichess du joueur.  
- **RF-D2** : Écoute des événements de partie.  
- **RF-D3** : Validation des coups via Lichess.  

### E. Serveur communautaire

- **RF-E1** : API REST simple : création de partie, liste des parties en attente, rejoindre une partie.  
- **RF-E2** : Métadonnées stockées : id, créateur, couleur demandée, statut (en attente / en cours), timestamp.  
- **RF-E3** : La liste des parties est mise à jour grâce à la communication de l'application au serveur.  
- **RF-E4** : Rafraîchissement de la liste dans l’application (pull périodique si pas d'informations des applications).  

### F. Journalisation & diagnostic

- **RF-F1** : Affichage et logs locaux des événements (USB, API, erreurs), exportables.  


## Requirements non-fonctionnelles

### Performance & latence

- **NRF-1** : Latence plateau → UI < 300 ms idéalement.    
- **NRF-2** : Affichage d’un snapshot complet en < 1 s.  

### Fiabilité & robustesse

- **NRF-4** : Précision de détection des pièces.  
- **NRF-5** : Résilience aux déconnexions USB : reconnexion automatique et reprise d’état.   

### Scalabilité

- **NRF-6** : Supporte une dizaine d'utilisateurs concurrents à l'aide du serveur communautaire.  
- **NRF-7** : Architecture modulaire pour extensions futures.  

### Maintenabilité & extensibilité

- **NRF-8** : Code documenté + tests unitaires sur la logique critique (USB, validation coups).  
- **NRF-9** : API documentée.  

### Portabilité & compatibilité

- **NRF-10** : Application PC compatible Windows et Linux (macOS optionnel).

### UX / accessibilité

- **NRF-11** : UI claire : état connexion plateau, statut Lichess, statut partie, erreurs.  
- **NRF-12** : Accessibilité minimale : navigation clavier + souris.  

### Testabilité

- **NRF-13** : Simulateur de plateau (émetteur USB virtuel) pour tests automatisés.  


