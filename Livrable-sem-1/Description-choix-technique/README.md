# Description des choix techniques

## Choix d'architecture
 
L’architecture retenue est découper en quatre éléments (plateau connecté, application desktop, serveur ChessAnywhere et Lichess). Cela nous permet de séparer clairement les responsabilités :

* le **plateau** reste focalisé sur la détection physique des pièces et l’assistance au joueur via les LED, sans complexité logicielle (algorithmique) inutile,
* l’**application desktop** joue un rôle central en tant que passerelle : elle assure la cohérence de l’état du jeu, facilite la communication avec les serveurs et propose une interface utilisateur,
* le **serveur ChessAnywhere** introduit une couche communautaire pour la mise en relation entre joueurs, ce qui évite de surcharger l’application,
* enfin, l’intégration avec **Lichess** exploite une API existante et fiable, ce qui permet de bénéficier immédiatement d’une gestion robuste des règles d’échecs et d’une redistribution des coups joué au joueur.

Ce découpage rend le système plus maintenable (chaque bloc pouvant évoluer indépendamment), plus flexible (possibilité de remplacer ou d’améliorer un élément sans impacter l’ensemble), et plus robuste (en cas de problème sur un serveur, les autres fonctionnalités peuvent continuer à fonctionner).

## Choix Plateau / Board

### Détection des pièces

Pour la détection des pièces sur l'échiquier, nous allons utiliser des reeds switchs. Pour connaître l'état de la partie, nous allons scanner tout le board, afin de récupérer l'état de tous les reeds switchs. Si le switch est ouvert, cela signifie que la case est libre (aucune pièce s'y trouve), sinon, si le switch est fermé, cela signifie qu'une pièce se trouve sur la case. Il n'est donc pas possible de déterminé quelle pièce se trouve sur la case, mais on peut simplement savoir si la case est occupée ou non. 

Le scan des reeds swicths s'effectuent sous forme de matrice 8x8. Via le microcontrôleur, on peut sélectionner la ligne ainsi que la colonne du switch dont on souhaite connaître la valeur.

### Affichage des mouvements possibles

Lorsque l'utilisateur soulèvera une pièce, il aura la possibilité de connaître les mouvements possibles grâce aux LED's RGB se trouvant sous la grille. Les LED's RGB sont connectées en série ce qui permet de les commander facilement avec une trame.

### Communication du PCB (board) au PC

Pour la communication du board au PC, nous avons décider d'utiliser de l'UART pour la partie microcontrôleur. Ce protocole permet un échange simplifié entre deux machines à l'aide de 2 fils. Après que le microcontrôleur ait envoyé une trame, celle-ci passera par un FT231 ou FT232 pour la convertir en format USB. Ainsi le PC sera capable de lire la trame envoyée par le microcontrôleur.

## Choix des composants

### Microcontrôleur – STM32G030F6P6

Le microcontrôleur principal est un **STM32G030F6P6** (ARM Cortex-M0+) :

* **Package simple à braser (TSSOP-20)**, adapté à un montage manuel et à des prototypes.
* **Nombre de broches suffisant** : fournit les UART, I²C, GPIO nécessaires à la gestion de la matrice de détection (reeds), de l’interface LED et de la communication.
* **Complexité réduite** : permet de limiter la charge logicielle tout en bénéficiant de l’écosystème STM32.
* **Outils de développement** : configuration et génération de code simplifiées via STM32CubeMX/IDE.

Ce choix équilibre **facilité d’intégration**, **coût** et **capacité technique** pour la gestion en temps réel du plateau.


### Interface USB/UART – FT231XS-U

La communication avec l’ordinateur est assurée par le **FT231XS-U** :

* **Conversion USB ↔ UART** fiable, évitant d’avoir à implémenter une pile USB sur le STM32.
* **Compatibilité multiplateforme** : drivers FTDI natifs sous Windows, Linux et macOS.
* **Utilisation principale** : transfert des états du plateau et réception des commandes depuis le PC.
* **Connecteur associé** : USB-C (USB4105-GF-A), standard moderne et robuste.
* **Disponibilité** : le **FT232**, initialement préféré quant a notre connaissance du composant et à sa popularité, n’était plus disponible chez Digikey au moment de la commande. Le choix s’est donc porté sur le **FT231XS-U**, offrant une compatibilité équivalente pour notre usage.


### LEDs intelligentes – ADA-RGB-1655

Chaque case de l’échiquier est équipée d’une LED RGB adressable type **1655** (similaire WS2812B) :

* **Chaînage série (DIN/DOUT)** : simplifie fortement le routage, une seule ligne de données pour 64 LEDs.
* **Consommation** : une LED peut consommer jusqu’à **20 mA par couleur**, soit **60 mA par LED RGB**.

  * Pour 64 LEDs : 64 × 60 mA = **3,84 A**, arrondi à **4 A** maximum en pleine intensité blanche.
* **Alimentation dédiée 5 V** : nécessaire pour supporter ce courant sans perturber le microcontrôleur.
* **Condensateurs de découplage 100 nF** : placés au plus près de chaque LED pour stabiliser l’alimentation, conformément aux recommandations du constructeur.

### Alimentation et distribution de puissance

* **Alimentation LEDs** : via un **bornier à vis (Phoenix 1729128)** et un **jack d’alimentation**, capables de délivrer le courant nécessaire aux 64 LEDs (jusqu’à 4 A sous 5 V).
* **Alimentation logique** : régulateur linéaire **LD1117S33** produisant du 3,3 V à partir du 5 V.
* **Protection et filtrage** :

  * **Condensateurs de découplage (100 nF, 10 µF)** proches du microcontrôleur et des circuits sensibles.

Cette séparation entre **alimentation logique (3,3 V, faible courant)** et **alimentation LEDs (5 V, fort courant)** garantit la robustesse du système.

#### Régulateur 3,3 V – LD1117S33CTR

Le **LD1117S33CTR** a été retenu pour générer le rail logique 3,3 V à partir du 5 V d’entrée. Ce régulateur linéaire, capable de fournir jusqu’à 800 mA, offre une solution simple et peu coûteuse pour alimenter le microcontrôleur, le FTDI et la logique associée. Sa faible complexité, son faible bruit de sortie et sa large disponibilité en boîtier SOT-223 en font un choix adapté aux besoins du projet, la consommation de la logique restant très inférieure à sa capacité maximale.

### Logique de lecture matrice – clarification Schottky

* **Lignes** : un **CD74HC238** sélectionne **une** ligne active.
* **Colonnes** : un second **CD74HC238** pilote des **BSS138** ; la colonne sélectionnée est **mise à la masse** via le MOSFET.
* **Lecture** : un **SN74HC251** multiplexe les retours de colonnes vers **une seule entrée MCU**.
* **Schottky (BAT54)** : elles sont placées **sur les sorties du décodeur de lignes** pour **empêcher les retours de courant entre ses sorties** lorsque **plusieurs reeds sont fermés**. Sans ces diodes, une sortie « L » pourrait tirer une autre sortie via des chemins parasites (sneak paths) à travers la matrice, provoquant des contentions ou des détections erronées.


### Connecteurs

* **USB-C (USB4105-GF-A)** : communication avec avec l'échiquier via FTDI.
* **Bornier 2 broches Phoenix** : alimentation 5 V externe pour LEDs.
* **Jack d’alimentation** : alternative pour l’injection du 5 V (LEDs).
* **Headers 2–5 broches** :

  * Programmation et debug (SWD : SWCLK/SWDIO).
  * Accès UART, I²C et signaux internes pour test ou extension.
* **Jumpers** : configuration flexible (sélection source d’alimentation, test de signaux).


## Justification des choix techniques du PCB

### Dimensions et grille

* **Format 254×254 mm** avec **marges 11 mm** : laisse un bord mécanique pour la fixation, la protection des pistes périphériques et l’espace pour sérigraphie/références.
* **Pas 29 mm par case** : compromis entre densité des composants (LED + reed + découplage) et lisibilité/ergonomie du plateau.
* **Reeds centrés + LED RGB adjacente + C\_découplage local** : centrage = détection symétrique des pièces ; LED adjacente = trajets courts pour VBUS/5 V et DIN/DOUT, ce qui réduit la chute IR et le couplage EMI.

### Répartition fonctionnelle (placement)

* **Haut-centre : MCU (STM32G030)** : point d’intégration des bus (UART, I²C, GPIO matrice) → longueurs de pistes équilibrées vers le reste.
* **Haut-droite : USB-C + FTDI** : éloigné du plan LED fort courant, proche du bord pour accès utilisateur, minimise la longueur des différentielles **D+/D-**.
* **Bas-centre : décodeur colonnes + multiplexeur lecture** : placement au barycentre des colonnes pour égaliser retards/impédances de retour.
* **Sous chaque colonne** : **BSS138** + réseau R associé : masse « localisée » des colonnes activées, boucles courtes, limitation des chemins parasites.
* **Gauche-centre : décodeur lignes + **Schottky** (BAT54)** : diodes côte sorties pour barrer les retours inter-sorties lors de multiples reeds fermés.
* **Haut-gauche : jack + bornier 5 V** : injection puissance éloignée de l’USB pour éviter back-feeding et bruit sur l’interface.
* **Connecteurs « berg »** en haut (accès câbles aisé) ; **SWD** proche MCU (intégrité et confort de debug).

### Stratégie de couches et de routage

* **2 couches** (TOP/BOTTOM) : optimisation coût/fabrication, suffisante avec une discipline de plans et de retours.
* **TOP : priorisation horizontale**, **BOTTOM : priorisation verticale** : tissage orthogonal → moins de vias, diaphonie maîtrisée.
* **USB D+/D- différentielles** : routées groupées, isochrones, couplées, retour GND continu ; longueur courte → marge sur la cible 90 Ω diff malgré 2 couches.

### Distribution de puissance et masses

* **Plan 5 V étendu, centré sous la grille LED** : réduction de la densité de courant par piste, chute IR homogène quand plusieurs LEDs tirent en blanc (≈ 4 A max).
* **Injection étoile depuis jack/bornier** : point d’entrée unique, évite boucles et retours par l’USB.
* **Îlots GND de sortie LED** avec **5–6 vias de stitching (Ø 0,4/0,8 mm)** par îlot : chemin de retour court au plus près des drivers internes LED, diminution des boucles HF.
* **BOTTOM : plan de masse uniforme** (référence de retour globale).
* **TOP : anneau de masse périphérique** (« guard ring ») : confinement EMI, continuité de blindage autour du plateau.
* **Stitching GND tous \~8 mm** : maille suffisante pour réduire l’inductance de surface et maintenir la continuité RF du plan avec un coût perçage raisonnable.

### Intégrité du signal

* **Chemins courts** entre reed ↔ décodeur/mux ↔ MCU : minimise la susceptibilité au rebond/contact et couplage.
* **Schottky sur sorties du décodeur de lignes** : supprime les chemins de retour involontaires (sneak paths) entre sorties lors de multiples fermetures simultanées.
* **MOSFET colonnes commandés par décodeur** : mise à la masse contrôlée, évite contentions et fixe l’état repos des colonnes.
* **Découplage systématique** : 100 nF au pied de chaque LED et de chaque CI, + réservoirs (1–10 µF) par sous-ensemble (MCU/FTDI/matrice).
* **Séparation logique/power** : zones 3,3 V (MCU/FTDI/HC) tenues à l’écart du flux 5 V fort courant des LEDs.

### Fabrication, test et maintenance

* **2 couches, TSSOP-20, vias standard** : assemblage manuel possible, DFM simple.
* **Points de test** **Vin/3V3** accessibles en haut : validation alimentation et mise au point.
* **Regroupement des connecteurs utilisateurs** en bord supérieur : ergonomie de câblage, contraintes mécaniques simplifiées.


## Choix Application

L’application desktop sera développée en **Python 3.10.13**, en utilisant **Tkinter** pour l’interface graphique. Ce choix permet de bénéficier d’un langage largement documenté, multiplateforme et simple à maintenir, tout en offrant une interface utilisateur suffisante pour nos besoins. Cela nous évitera d'avoir a prendre en main un framework plus complexe comme Qt ou javaFx.

L’application communiquera à la fois avec :

* **l’API officielle de Lichess**, afin de gérer les parties en ligne, valider les coups et synchroniser l’état de la partie,
* **une API ChessAnywhere**, qui permettra la mise en relation entre utilisateurs et la gestion des rooms.

Les principales fonctionnalités prévues sont :

* **Visualisation des rooms ChessAnywhere** créées par les utilisateurs,
* **Lancer un challenge à un utilisateur Lichess en ligne** à l'aide d’un lien,
* **Lancement d’une partie locale (versus en personne)** sans passer par Lichess,
* **Saisie du token API Lichess** pour l’authentification et l’accès aux fonctionnalités avancées,
* **Suivi en temps réel du statut de connexion au plateau (board)**,
* **Affichage de l’état courant du plateau** (position des pièces, coups possibles, etc.).

Cette approche garantit une application capable de servir de point central entre le matériel physique et les serveurs distants, tout en offrant à l’utilisateur une interface claire.

## Choix Serveur

Le serveur communautaire **ChessAnywhere**:

* le serveur agit comme **broker** (intermediaire),
* les clients (l’application desktop) peuvent créer des rooms,
* d’autres clients peuvent se connecter a la room pour recevoir le lien correspondants a la partie.

Le serveur sera développé en **Java** et déployé sur une **Web App Azure**. Ce choix est motivé par :

* la connaissance préalable de java,
* l’intégration facilitée avec l’environnement **Azure** que nous avons également déjà utilisé, sauf que nous avions utilisé une VM.

Cette approche permet de garder une architecture avec la quelle nous avons déjà interagit, tout en développant notre propre API.
