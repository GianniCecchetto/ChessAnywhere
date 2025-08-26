# Documentation de l'API

Le serveur met à disposition les routes suivantes :

## Challenges

* **GET** ```/api/challenges``` : Retourne la liste des challenges
* **POST** ```/api/challenge``` : Crée un nouveau challenge avec couleur aléatoire
* **POST** ```/api/challenge/{color}``` : Crée un nouveau challenge en ayant {color} comme couleur
* **DELETE** ```/api/challenge``` : Annule (efface) le challenge lancé par le joueur
