# Description du processus de travail

Le projet suit une organisation collaborative basée sur **GitHub** et **Discord**, afin de garantir une bonne coordination et une intégration continue.

### Gestion du code source et des branches

* Le dépôt principal est hébergé sur **GitHub**.
* Nous suivons une approche proche de **Git Flow** :

  * la branche **`main`** contient les versions stables,
  * chaque fonctionnalité est développée dans une **feature branch** dédiée, puis intégrée via *pull request*.
* Les *pull requests* sont relues et validées avant d’être fusionnées, afin d’assurer la qualité du code.

### Organisation du projet

* Nous utilisons l’onglet **Projects** de GitHub (Kanban / roadmap) pour planifier et suivre l’avancement :

  * **To Do** : tâches à réaliser,
  * **In Progress** : tâches en cours,
  * **Done** : tâches terminées.
* Cela permet une visibilité claire sur l’état du projet et la répartition du travail.
* De plus cela permet de maintenir un historique des tâches via les backlogs

### Intégration continue

* L’onglet **Actions** de GitHub est utilisé pour configurer des workflows en **YAML**.
* Ces workflows permettent :

  * le lancement automatique de tests d’intégration,
  * la vérification du code lors de chaque push ou pull request,
  * éventuellement des étapes supplémentaires (build, déploiement futur).

### Communication et coordination

* La communication d’équipe est centralisée sur un **serveur Discord**.
* Plusieurs salons sont mis en place pour une gestion efficace :

  * **salons textuels** (organisation, suivi des tâches, entraide technique),
  * **salons vocaux** (réunions rapides, sessions de travail collaboratives).

Ce processus de travail nous permet d’assurer :

* une meilleure qualité du code grâce aux tests et aux revues,
* un bon suivis de l'avancement du développement via GitHub Projects,
* une communication simple et structuré grâce à Discord.