# Pipeline

Pour l'application Python, nous avons une action qui installe les dépendances et qui lance les pytest du projet à chaque push où il y a eu une modification dans le dossier app/ sur le repo GitHub ainsi que lorsqu'il y a une PR sur la branch main. 

Nous avons une deuxième action pour déployer un exécutable Windows dans une release GitHub qui, elle, s'exécute uniquement lorsqu'il y a un tag de version.

Vous trouverez les deux fichiers `python-ci.yml` et `python-release.yml` dans le dossier `.github/workflows/`.
