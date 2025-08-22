# Description du processus de travail (Approche Agile)

Le projet adopte une organisation agile et collaborative, reposant sur **GitHub** et **Discord**, afin de garantir une bonne coordination, une intégration continue et une visibilité claire de l’avancement.

---

## Gestion du code source et des branches

* Le dépôt principal est hébergé sur **GitHub**.
* Nous suivons une organisation inspirée de **Git Flow** :

  * La branche **`main`** contient uniquement les versions stables.
  * Chaque fonctionnalité est développée dans une **feature branch** dédiée.
  * L’intégration se fait via des **pull requests**, soumises à relecture et validation avant fusion.

Cette approche garantit la stabilité du code et favorise la qualité grâce aux revues croisées.

---

## Organisation du projet

* Nous utilisons l’onglet **Projects** de GitHub (Kanban / roadmap) pour planifier et suivre les tâches :

  * **To Do** : tâches à réaliser,
  * **In Progress** : tâches en cours,
  * **Need Review** : en attente de relecture,
  * **Done** : tâches finalisées.

Cet outil permet :

* une visibilité claire sur l’état du projet,
* une répartition équitable du travail,
* la conservation d’un historique grâce aux backlogs.

---

## Intégration continue (CI)

* L’onglet **Actions** de GitHub est configuré avec des **workflows en YAML** permettant :

  * l’exécution automatique des tests d’intégration,
  * la vérification du code à chaque *push* ou *pull request*,
  * la mise en place progressive d’étapes supplémentaires (build, déploiement futur).

Ainsi, chaque contribution est testée et validée avant son intégration.

---

## Communication et coordination

* La communication de l’équipe est centralisée sur **Discord**.
* Plusieurs salons sont organisés pour une efficacité optimale :

  * **Salons textuels** : organisation, suivi des tâches, entraide technique.
  * **Salons vocaux** : réunions rapides, sessions collaboratives.

Cela assure une communication fluide, réactive et structurée.

---

## Bénéfices de notre organisation

* Qualité renforcée grâce aux tests automatisés et aux revues de code.
* Suivi clair de l’avancement via GitHub Projects.
* Collaboration efficace grâce à Discord.

---