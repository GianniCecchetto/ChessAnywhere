# Application ChessAnywhere

## Ajouter tous les packages

Pour installer les packages nécessaires, il suffit de lancer la commande suivante depuis la racine.

```bash
pip install -r app/requirements.txt
```

## Lancer le projet

Pour lancer l'application Python il vous faudra ouvrir le chemin du répértoire WSL dans une ligne de commande PowerShell.

Il faudra ensuite utilisez la commande suivante à la racine du répértoire:

```bash
python3 -m app.app.main
```

## Structure des fichiers

- **assets**  
Le chemin ou les images (logo, pièces) sont stockées.
- **game_logic**  
Dans ce dossier, nous trouvons les fichiers Python qui gère la logique du jeu d'échec.
- **gui**  
Dans ce dossier, nous avons tous les fichiers qui ont un lien avec la GUI.
- **lib**  
Dans lib, nous retrouvons les scripts qui nous permettent d'utiliser la librairie uart écrit en `C`.
- **networks**  
Dans ce dossier, nous avons les scripts qui s'occupent de faire des requêtes en ligne.
- **uart***  
Nous avons ici les fichiers qui gèrent les communications en uart.

