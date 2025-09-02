![Logo](docs/img/Logo_small.png)

# Spécification projet – Échiquier connecté

## Objectif du projet

Permettre à un joueur d’utiliser un vrai plateau d’échecs et de vraies pièces pour jouer des parties connectées.
Les positions réelles des pièces sont lues par le plateau (capteurs reed) et transmises via USB à une application sur PC.
L’application permet :

- de rejoindre une partie Lichess via un URL,

- de créer/rejoindre des parties communautaires via un serveur central listant les parties en attente.

Le système valide les mouvements (localement et/ou via Lichess), affiche l’état du jeu et gère la communication entre le plateau, l'application, l'API Lichess et le serveur.

## Kick project presentation

- [Landing Page](docs/mockups_app_and_landing_page)
- [mockups](docs/mockups_app_and_landing_page)

## Structure de fichier

- [.github/workflows](.github/workflows) : Test d'intégration continu
- [app](app) : Application Python
- [docs](docs) : Documentation générale du projet
- [firmware](firmware) : Firmware du microcontrôleur STM32
- [hardware](hardware) : Schéma et PCB du board
- [lib](lib) : Librairie créer pour le projet
- [livrable_sem_1](livrable_sem_1) : Rendu de la semaine 1
- [server](server) : Serveur Azure

# Mise en place du déploiement automatique

Afin de ne pas surcharger ce `README.md`, nous avons séparer la documentation du pipeline de déploiement en différentes sections :

- [/docs/stm32_self_deployment](/docs/stm32_self_deployment) : Pipeline Firmware
- [/docs/app_self_deployement](/docs/app_self_deployement) : Pipeline Application *(TODO)*
- [/docs/server_self_deployement](/docs/server_self_deployement) : Pipeline Serveur *(TODO)*

# Mise en place du projet 

Voici la démarche à suivre, étape par étape, afin de pouvoir reproduire ce projet :

## 1 - Plateau Board

Dans cette section, nous allons voir comment mettre en place le plateau, soit l'échiquier

### 1.1 - PCB

- Monter les PCB
- Indiquer l'endroit de la BOM
- Commande de composants
- Tests effectués
- Spécifications par rapport au schéma
- Modification apportée pour le bon fonctionnement (diode)
- Alimentation (tension + courant)
- Montage des reeds
- ATTENTION les reeds sont fragiles

### 1.2 - Firmware

Une fois que la carte est montée et 

- Code
- IDE
- Optimisation de compilation
- Fichier .ioc
- Flasher

### 1.3 - Mécanique

- Grille 3D
- Impression des pièces 3D
- Montage des puèces 3D avec les aimants

## 2 - Application python

Gianni Nathan

- Packages
- Commandes Pythons
- Fonctionnement
- Fonctionnalités
- Lancement de l'appli

## 3 - Serveur

Gianni

- Packages
- Commandes
- Fonctionnement
- Fonctionnalités

## 4 - Tests de bon fonctionnements



# Authors

- Gianni Cecchetto - [Github](https://github.com/GianniCecchetto)
- Nathan Tschantz - [Github](https://github.com/TschantzN)
- Ali Zoubir - [Github](https://github.com/Ali-Z0)
- Thomas Stäheli - [Github](https://github.com/thomasstaheli)
