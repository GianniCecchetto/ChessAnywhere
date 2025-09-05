# API de Jeu d’Échecs (intégrée à Lichess)

Ce projet fournit une **API REST** construite avec [Javalin](https://javalin.io/) permettant de créer, rejoindre et gérer des parties d’échecs via l’[API Lichess](https://lichess.org/api).  
L’application garde également une trace des parties actives en mémoire et annule automatiquement les défis non rejoints après un délai défini.

## Fonctionnalités

- **Vérification du service**
  - `GET /ping` → Retourne `200 OK`.

- **Gestion des parties**
  - `GET /api/games` → Liste toutes les parties actives (synchronisées avec Lichess).
  - `GET /api/games/{gameId}` → Récupère les détails d’une partie.
  - `POST /api/games/create` → Crée une nouvelle partie (couleur aléatoire).
  - `POST /api/games/create/{color}` → Crée une partie avec une couleur choisie (pas encore implémenté).
  - `POST /api/games/join/{gameId}` → Rejoint une partie existante.

- **Nettoyage automatique**
  - Les parties créées mais non rejointes sont **annulées après 30 secondes**.
  
## Composants

### `Api`
- Configure le serveur Javalin.
- Enregistre les routes et les gestionnaires d’exceptions.
- Délègue les opérations liées aux parties au `GameController`.

### `GameController`
- Maintient une **map concurrente** des parties actives.
- Délègue les appels externes à `LichessClient`.
- Fournit des endpoints pour :
  - Lister les parties (`getAll`)
  - Récupérer une partie (`getOne`)
  - Créer une partie (`create`, `createRandom`)
  - Rejoindre une partie (`join`)

### `LichessClient`
- Gère la communication avec **l’API Lichess** via `HttpClient`.
- Endpoints utilisés :
  - `/api/challenge` → Liste des défis.
  - `/api/challenge/open` → Crée un nouveau défi.
  - `/api/challenge/{id}/show` → Détails d’un défi.
  - `/api/challenge/{id}/cancel` → Annule un défi.
- Utilise **Jackson** (`ObjectMapper`) pour le parsing JSON.
- Programme l’annulation automatique des parties non rejointes après un délai.

## Variables d’environnement

L’application a besoin d’un **token Lichess** pour s’authentifier :  

```bash
export LICHESS_TOKEN=ton_token_lichess
```


## Exemple d’utilisation

1. Créer une partie
```bash
curl -X POST http://localhost:8080/api/games/create
```
2. Rejoindre une partie
```bash
curl -X POST http://localhost:8080/api/games/join/{gameId}
```
3. Obtenir les détails d'une partie
```bash
curl http://localhost:8080/api/games/{gameId}
```

## Build le serveur

Utilisez la commande suivant afin de créer un .jar du serveur:

```bash
mvnw package
```

Ceci va vous créer l'archive suivante: `ChessAnywhreServer-1.0-SNAPSHOT.jar`, que nous utilisons ensuite dans le Docker.
