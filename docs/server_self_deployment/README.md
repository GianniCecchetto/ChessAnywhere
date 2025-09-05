# Pipeline

Les actions pour le serveur Java sont presque identiques à l'application Python. Une va réaliser des tests ainsi que créer le docker pour ensuite tester si le serveur est vivant.

Nous avons une deuxième actions qui va build et push le docker sur `ghcr.io/giannicecchetto/chess-anywhere-server:{latest/v*.*.*}` avant de le déployer automatiquement sur une **Web App Azure** lorsqu'un tag de version (v*.*.*) est push sur le repo.

Vous trouverez les deux fichiers `java-ci.yml` et `java-release.yml` dans le dossier `.github/workflows/`.
