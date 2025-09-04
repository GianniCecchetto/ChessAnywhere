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
- [/docs/app_self_deployement](/docs/app_self_deployment) : Pipeline Application *(TODO)*
- [/docs/server_self_deployement](/docs/server_self_deployment) : Pipeline Serveur *(TODO)*

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

| Broche | Signal | Description                  |
|--------|---------|------------------------------|
| 1      | SWCLK   | Horloge du débogueur SWD     |
| 2      | SWIO    | Données du débogueur SWD     |
| 3      | 3V3     | Alimentation 3,3 V           |
| 4      | NRST    | Reset du microcontrôleur     |
| 5      | GND     | Masse                        |

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
