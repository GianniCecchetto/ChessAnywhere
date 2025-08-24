# Description du pipeline DevOps

Le projet utilise les GitHub Actions pour automatiser les tests sur les différents projets. Ils servent aussi à déployer automatiquement notre server dockerisé sur une **Web App Azure**. Le firmware du STM32 est aussi automatiquement déployer sur le microcontrôleur afin de tester le programme.

---

## App Python

Pour l'application Python, nous avons une action qui installe les dépendances et qui lance les pytest du projet à chaque push où il y a eu une modification dans le dossier **python-app** sur le repo GitHub ainsi que lorsqu'il y a une **PR** sur la branch main. Nous avons une deuxième action pour créer un exécutable Windows qui, elle, s'exécute uniquement lorsqu'il y a un tag de version.

---

## Server Java

Les actions pour le serveur Java sont presque identiques à l'application Python. Une va réaliser des tests ainsi que créer le docker pour ensuite tester si le serveur est vivant. Et nous en avons une deuxième qui va build le docker avant de le déployer automatiquement sur une **Web App Azure** lorsqu'un tag de version est push.

---

## Firmware STM32

Pour le STM32, nous avons qu'une seule action qui s'exécute sur un **Self Hosted Runner** qui va compiler le code, flasher le code sur le STM32, et qui va ensuite lancer un script python pour tester certaines fonctionnalitées du Firmware.

---
