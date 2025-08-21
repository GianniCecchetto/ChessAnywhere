# Description des choix techniques

## Choix d'architecture
 
L’architecture retenue est découper en quatre éléments (plateau connecté, application desktop, serveur MQTT et Lichess). Cela nous permet de séparer clairement les responsabilités :

* le **plateau** reste focalisé sur la détection physique des pièces et l’assistance au joueur via les LED, sans complexité logicielle (algorithmique) inutile,
* l’**application desktop** joue un rôle central en tant que passerelle : elle assure la cohérence de l’état du jeu, facilite la communication avec les serveurs et propose une interface utilisateur,
* le **serveur MQTT** introduit une couche communautaire pour la mise en relation entre joueurs, ce qui évite de surcharger l’application,
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

### Microcontrôleur

Ali ...
Nous avons décider d'utiliser un STM32, ...
- Logiciel professionnel utilisé dans l'industrie
- Configuration des périphériques avancées
- Language C

### PCB

Ali ...
- Dimension / Taille du PCB
- Placement des composants

### Composants

Ali ...
- Choix du microcontrôleur
- Choix du FT231
- Choix des leds
- Choix de l'alimentation
- Choix 

## Choix Application

Gianni ...
- Language
- Fonctionnalité
- API
- Librairies 

## Choix Serveur

Gianni ...
Le serveur fera une partie simple

- Language
- Fonctionnalité
- API
- Librairies