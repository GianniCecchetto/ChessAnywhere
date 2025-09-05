![Logo](docs/img/Logo_small.png)

# Spécification projet – Échiquier connecté

![Logo](docs/img/board.jpg)

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
- [/docs/app_self_deployement](/docs/app_self_deployment) : Pipeline Application
- [/docs/server_self_deployement](/docs/server_self_deployment) : Pipeline Serveur

# Mise en place du projet 

Voici la démarche à suivre, étape par étape, afin de pouvoir reproduire ce projet :

## 1 - Plateau Board

Dans cette section, nous allons voir comment mettre en place le plateau, soit l'échiquier

### 1.1 - Schéma

Une documentation détaillant les choix d’implémentation et expliquant le schéma est disponible dans le README [/docs/pcb/README.md](/docs/pcb/README.md).


### 1.2 - PCB

1. Commander les composants indiqués dans la **BOM (Bill of Materials)** [/docs/pcb/bom.pdf](/docs/pcb/BOM.PDF), correspondant aux éléments nécessaires au montage de la carte.

2. Commander le PCB avec les fichier GERBER générés via [/hardware/altium/OutJobs.OutJob](/hardware/altium/OutJobs.OutJob), ou zip des gerber disponbile à [/hardware/altium/Outputs/EuroCircuits.zip](/hardware/altium/Outputs/EuroCircuits.zip).

2. Assembler le PCB en respectant la nomenclature des composants. **Attention : les contacts magnétiques REED en verre sont fragiles.**  
   Il est nécéssaire dans cette version d’utiliser des REED avec diode intégrée, ou d’ajouter manuellement des diodes Schottky rapides en série avec chaque REED.  
   *(Une version ultérieur du schéma/PCB intégrerait 64 diodes SMD, une sous chaque case de l’échiquier.)*

3. Effectuer des mesures de vérification, notamment les tests d’alimentation.  
   Pour ce faire, les jumpers **W1** et **W2** permettent de couper les pistes avant et après le régulateur de tension *LD1117S33CTR* qui génère le 3,3 V alimentant la carte.

4. L’alimentation de la carte peut se faire via deux connecteurs :  
   - un bornier à vis,  
   - ou un connecteur jack 2,1 mm.  
   La carte s’alimente en **5 V / 4 A** : cette tension alimente directement les LEDs, puis est régulée via un LDO pour fournir le **3,3 V** nécessaire aux autres circuits.

5. Les jumpers **W5** et **W8** permettent de connecter/déconnecter l’alimentation VCC/GND du microcontrôleur STM32.  
   Cela permet, si nécessaire, de déporter le contrôle des périphériques vers un contrôleur externe via les connecteurs **P2, P3, P4, P5 et P7**.  
   Dans la solution actuelle utilisant le STM32G030F6P6 intégré, il est recommandé de laisser ces jumpers en place et de poursuivre l’intégration telle que proposée.

6. Les autres jumpers sont optionnels et peuvent être utilisés pour certaines analyses spécifiques.  
   Se référer au schéma : [/docs/pcb/Schematic_Prints.PDF](/docs/pcb/Schematic_Prints.PDF).


### 1.3 - Firmware

Une documentation approfondie existe dans le `README.md` du le répertoire [/firmware](/firmware). Dans ce dossier vous aurez toutes les indications pour :
- Compiler / Flasher le firmware
- Modifier le code du firmware
- Modifier l'architecture du microcontrôleur

Une fois la carte montée et testée électriquement, il est possible de passer à la programmation du microcontrôleur.

- **Code source** : fourni dans le répo, dans [/firmware](/firmware).  
- **IDE recommandé** : STM32CubeIDE (intégrant CubeMX pour la configuration et la génération de code).  
- **Optimisations de compilation** : adaptées au microcontrôleur STM32G030F6P6 ayant peu de mémoire flah (32KB), configurées dans le projet (indication dans [/firmware](/firmware)).  
- **Fichier `.ioc`** : inclus pour permettre la reconfiguration matérielle via STM32CubeMX.

---

#### Instructions de flashage du microcontrôleur

Pour programmer le microcontrôleur *STM32G030F6P6*, il est nécessaire d’utiliser un programmateur **ST-LINK** ou tout autre outil compatible **SWD (Serial Wire Debug)**.  

Le programmateur doit être connecté au connecteur **P8** avec le pinout suivant :

| Broche | Signal | Description              |
| ------ | ------ | ------------------------ |
| 1      | SWCLK  | Horloge du débogueur SWD |
| 2      | SWIO   | Données du débogueur SWD |
| 3      | 3V3    | Alimentation 3,3 V       |
| 4      | NRST   | Reset du microcontrôleur |
| 5      | GND    | Masse                    |

![Schéma de connexion du programmateur](docs/img/pinning_programmer.jpg)

---

**Remarque importante** :  
- Vérifier que la carte est alimentée correctement (3,3 V stables).  
- Éviter toute inversion de câblage (aucune protection à été mise en place sur la carte), au risque d’endommager le microcontrôleur ou le programmateur.
- Aucun composant anti-static se trouve sur la carte, veillez à manipulez la carte avec un bracelet ESD.


### 1.4 - Mécanique

Matériel nécessaire :

- 8x Colonettes M3 15mm
- 4x Vis M3
- 1x Plexiglace, de même dimension que le board ou légèrement supérieur, ave 4 trous de perçage M3 à effectuer et si possible, un plexiglace de 4mm d'épaisseur maximum afin de garantir une bonne détéction des reeds
- 1x Grille 3D (modèle disponible [/docs/board_design/ChessAnywhereBoard.stl](/docs/board_design/ChessAnywhereBoard.stl) )
- 1x Papier filtre avec les mêmes dimensions que la grille 3D
- 2x Pièces d'échecs à imprimer avec des trous en-dessous, afin de pouvoir y insérer des aimants.



## 2 - Application python

### Packages requis

Cette application utilise les bibliothèques suivantes :  

- `requests` → Requêtes HTTP.  
- `pyserial` → Communication série (ex. Arduino, ports COM).  
- `Pillow` → Manipulation et affichage d’images.  
- `customtkinter` → Interface graphique moderne basée sur Tkinter.  
- `python-chess` → Gestion et logique du jeu d’échecs.  
- `berserk` → Client Python pour l’API [Lichess](https://lichess.org).  
- `platformdirs` → Gestion des répertoires spécifiques au système (config, cache, data).

Installation :  

```bash
pip install -r requirements.txt
```

ou directement :

```bash
pip install requests pyserial Pillow customtkinter python-chess berserk platformdirs
```

### Fonctionnement

- L’application est écrite en Python 3.10.13.
- Elle combine plusieurs briques logicielles :
  - Interface graphique → via customtkinter et Pillow.
  - Jeu d’échecs → via python-chess.
  - Connexion à Lichess → via berserk.
  - Communication externe → via requests et pyserial.
- Les données de configuration et de cache sont gérées proprement grâce à platformdirs.

### Fonctionnalités

- [x] Interface graphique utilisateur
- [x] Création et gestion de parties d’échecs locales ou en ligne.
- [x] Communication avec l’API Lichess (via berserk).
- [x] Connexion à du matériel externe via port série (pyserial).

### Lancement de l’application

1. Cloner le projet
```bash 
git clone git@github.com:GianniCecchetto/ChessAnywhere.git
cd ChessAnywhere
```
2. Installer les dépendances sur Windows
```bash 
pip install -r app/requirements.txt
```
3. Démarrer l'application depuis la racine
```bash
python3 -m app.app.main
```

## 3 - Serveur

### Package

Pour réalisé le serveur, nous avons utilisé les packages suivants:
- Maven shade plugin (v3.5.0)  
Permet de réalisé un .jar qui contient la solution de l'application.
- Javalin (v6.7.0)  
Permet de mettre en place l'API du serveur.
- rest-assured (v6.7.0)  
Permet de faire des requêtes HTTP.
- JUnit  
Permet de réaliser les tests unitaires du serveur.

### Docker

Pour build le docker du serveur, lancer la commande suivant à la racine du projet :  
```docker build -t chess-anywhere-server ./server```

Pour lancer ensuite le docker, utilisé la commande suivante :  
```docker run -p 7000:7000 -e LICHESS_TOKEN={lichess_token} chess-anywhere-server```  
**lichess_token:** Mettre un token d'un compte qui servira à créer les parties.

### Unit test

Les units tests sont effectués à chaque push lorsque ceux-ci affectent les fichiers dans le dossier server/. Ils ont été réalisé pour les premières versions du serveur. Ensuite, il y a eu des appelles à l'API Lichess, il était donc plus simple de les enlever ne sachant pas s'il fallait réaliser des tests dans ce cas.

### Déploiement

Le serveur est déployer automatiquement lorsqu'un tag de version (v*.*.*) est créé sur le git. Ceci nous évite de devoir le déployer à chaque nouvelle mise à jour.

# Authors

- Gianni Cecchetto - [Github](https://github.com/GianniCecchetto)
- Nathan Tschantz - [Github](https://github.com/TschantzN)
- Ali Zoubir - [Github](https://github.com/Ali-Z0)
- Thomas Stäheli - [Github](https://github.com/thomasstaheli)
