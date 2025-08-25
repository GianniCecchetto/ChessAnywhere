![Logo](Documentation/img/Logo_small.png)

# Spécification projet – Échiquier connecté

## Objectif du projet

Permettre à un joueur d’utiliser un vrai plateau d’échecs et de vraies pièces pour jouer des parties connectées.
Les positions réelles des pièces sont lues par le plateau (capteurs reed) et transmises via USB à une application sur PC.
L’application permet :

- de rejoindre une partie Lichess via un URL,

- de créer/rejoindre des parties communautaires via un serveur central listant les parties en attente.

Le système valide les mouvements (localement et/ou via Lichess), affiche l’état du jeu et gère la communication entre le plateau, l'application, l'API Lichess et le serveur.

## Kick project presentation

- [Landing Page](Documentation/LandingPage)
- [Mockups](Documentation/Mockups)

## Structure de fichier

- [.github/workflows](.github/workflows) : Test d'intégration continu
- [server](server) : Code et API du serveur
- [docs](docs) : Toute la documentation du projet
- [livrable_sem_1](livrable_sem_1) : Contient tous les fichiers à livrer pour la semaine 1
- [hardware](hardware) : Schéma et PCB
- [app](app) : Application graphique en python
- [firmware](firmware) : Firmware STM32 pour le board
