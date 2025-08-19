# ChessAnywhere

# Spécification projet – Échiquier connecté

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



## Requirements non-fonctionnelles

