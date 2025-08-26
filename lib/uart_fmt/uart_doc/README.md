# Protocole UART pour Échiquier Intelligent (reeds + LEDs RGB)

Ce document décrit le protocole **ASCII** (lisible au terminal) permettant à une **application hôte** d’interagir avec un **PCB d’échiquier** doté de **contacteurs reed** (détection de pièces) et de **LEDs RGB** (une par case). Il couvre :

- Paramètres UART & conventions
- Événements **PCB → App** (asynchrones)
- Commandes **App → PCB** (avec configuration couleurs & debounce)
- Codes d’erreur, exemples, bonnes pratiques
- Annexes (mappage cases, bitboard, timing)

---

## 1) Paramètres & conventions

- **UART** : `115200 bauds`, `8N1`, aucun contrôle de flux.
- **Fin de ligne** : chaque message (commande ou événement) se termine par `\r\n`.
- **Réponses** :
  - Succès : `OK [DATA]`  
  - Erreur : `ERR <CODE> [DETAILS]`  
- **Temps** : `t=<ms>` est le temps en millisecondes depuis le boot.
- **Cases** : notation échiquéenne `A1..H8` (insensible à la casse côté parsing).
- **Bitboard 64 bits** : chaîne hex sur **16 caractères** (64 bits). Convention :
  - `bit0 = A1`, `bit1 = B1`, …, `bit7 = H1`, `bit8 = A2`, …, `bit63 = H8`.
  - `1` = reed **fermé** (pièce présente), `0` = reed **ouvert** (vide).
- **Remap d’orientation** : la carte applique `ORIENT` (0/90/180/270°) avant d’émettre ou d’interpréter l’index logique des cases.

---

## 2) Flux global & machine à états (côté PCB)

1. **IDLE** – plateau stable.  
   Événement : reed **ouvre** → `EVT LIFT FROM` → **LIFTED(FROM)**.
2. **LIFTED(FROM)** – une pièce est soulevée.  
   L’app envoie `:LED MOVES FROM …` pour afficher les destinations.  
   Reed **ferme** ailleurs → `EVT PLACE TO` → **PLACED(FROM,TO)**.  
   Repose sur FROM → `EVT LIFT_CANCEL FROM` → **IDLE**.
3. **PLACED(FROM,TO)** – pièce posée sur TO.  
   Appui bouton → `EVT MOVE FROM TO` → **WAIT_APP**.  
   Re-déplacement → nouveau `EVT PLACE TO'` (mise à jour).  
   Retour sur FROM → `EVT LIFT_CANCEL FROM` → **IDLE**.
4. **WAIT_APP** – la carte attend décision de l’app.  
   `:MOVE ACK FROM TO …` → animation OK → LEDs idle → **IDLE**.  
   `:MOVE NACK FROM TO …` → flash erreur → **LIFTED(FROM)** ou état réel selon reeds.

> **Timeouts recommandés** : LIFTED→15 s, PLACED→10 s ; sinon `EVT TIMEOUT <STATE>`.

---

## 3) Événements **PCB → App** (asynchrones)

> Les événements sont émis **uniquement** si `:STREAM ON` est actif.

| Événement | Syntaxe | Description |
|---|---|---|
| Boot prêt | `EVT BOOT FW=<v> HW=<id> t=<ms>` | La carte vient d’être initialisée et opérationnelle. |
| Reed levé | `EVT LIFT <SQ> t=<ms>` | Le reed de `<SQ>` **ouvre** → une pièce vient d’être soulevée. |
| Reed posé | `EVT PLACE <SQ> t=<ms>` | Le reed de `<SQ>` **ferme** → une pièce vient d’être posée. |
| Mouvement (bouton) | `EVT MOVE <FROM> <TO> t=<ms>` | Bouton validé : propose le coup courant. |
| Annulation | `EVT LIFT_CANCEL <SQ> t=<ms>` | La pièce est reposée sur sa case d’origine. |
| Bouton (brut) | `EVT BTN t=<ms>` | Appui bouton (si vous préférez gérer la logique côté app). |
| Inactivité | `EVT TIMEOUT <STATE> t=<ms>` | Aucune action durant la fenêtre autorisée. |
| Erreur | `EVT ERROR <CODE> [DETAILS] t=<ms>` | Anomalie firmware (diagnostic).

**États possibles dans TIMEOUT** : `IDLE`, `LIFTED`, `PLACED`, `WAIT_APP`.

---

## 4) Commandes **App → PCB**

### 4.1 Système & flux

| Commande | Syntaxe | Réponse | Rôle |
|---|---|---|---|
| Ping | `:PING` | `OK PONG` | Test de communication. |
| Version | `:VER?` | `OK VER FW=<v> HW=<id>` | Version firmware & identifiant matériel. |
| Uptime | `:TIME?` | `OK TIME t=<ms>` | Temps depuis boot. |
| Reset soft | `:RST` | `OK` | Redémarre proprement (reconfig à la volée au prochain boot). |
| Sauvegarde | `:SAVE` | `OK` | Persiste la configuration en flash. |
| Flux événements | `:STREAM ON` / `:STREAM OFF` | `OK` | Active/désactive l’émission des `EVT …`. |

### 4.2 Lecture des reeds (état plateau)

| Commande | Syntaxe | Réponse | Rôle |
|---|---|---|---|
| Snapshot plateau | `:READ ALL` | `OK STATE 0x<64bits>` | Bitboard 64 bits (1=présence). |
| État case | `:READ SQ <SQ>` | `OK SQ <SQ> <0|1>` | 1=pièce présente, 0=vide. |
| Masque ignore | `:READ MASK SET 0x<64bits>` | `OK` | Bits=1 → **ignorer** événements de ces cases. |
| Masque cour.| `:READ MASK?` | `OK MASK 0x<64bits>` | Retourne le masque courant. |

### 4.3 LEDs — contrôle direct

| Commande | Syntaxe | Réponse | Rôle |
|---|---|---|---|
| Allumer case | `:LED SET <SQ> <R> <G> <B>` | `OK` | Valeurs 0..255. |
| Éteindre case | `:LED OFF <SQ>` | `OK` | — |
| Tout éteindre | `:LED OFF ALL` | `OK` | — |
| Remplir | `:LED FILL <R> <G> <B>` | `OK` | Applique aux 64 cases. |
| Rectangle | `:LED RECT <FROM> <TO> <R> <G> <B>` | `OK` | Bloc aligné (ex. `A1 H8`=tout). |
| Bitboard | `:LED BITBOARD 0x<64bits>` | `OK` | Allume bits=1 avec **couleur par défaut** `COLOR.HINT`. |
| Frame 64×RGB | `:LED MAP <hex192>` | `OK` | 64×(R,G,B)=192 octets hex concaténés `A1→H8`. |
| Luminosité | `:LED BRIGHT <0..255>` | `OK` | Dimming global (scale/PWM). |
| Gamma | `:LED GAMMA <float>` | `OK` | Ex. `2.2` (appliqué à la sortie). |

#### Hex de `:LED MAP`
- `hex192` = 192 octets hexadécimaux **sans séparateurs** (optionnellement on peut tolérer des espaces `_` à l’implémentation).  
- Ordre par case : `A1 R,G,B`, `B1 R,G,B`, …, `H1`, puis `A2` … jusqu’à `H8`.

### 4.4 Palette de couleurs nommées

Couleurs logiques pour scénarios de jeu : `COLOR.IDLE`, `COLOR.FROM`, `COLOR.TO`, `COLOR.HINT`, `COLOR.OK`, `COLOR.ERROR`, `COLOR.CHECK`, `COLOR.PREVIEW` (extensible).

| Commande | Syntaxe | Réponse | Rôle |
|---|---|---|---|
| Définir | `:COLOR SET <NAME> <R> <G> <B>` | `OK` | Ex. `:COLOR SET COLOR.HINT 80 160 255`. |
| Lire | `:COLOR GET <NAME>` | `OK COLOR <NAME> <R> <G> <B>` | — |
| Lister | `:COLOR?` | `OK COLORS [NAME=R,G,B ...]` | Liste compacte. |

### 4.5 LED — commandes haut niveau « coups »

| Commande | Syntaxe | Réponse | Rôle |
|---|---|---|---|
| Surligner coups | `:LED MOVES <FROM> <N> <SQ1> … <SQN>` | `OK` | Met `<FROM>` en `COLOR.FROM`, chaque `<SQi>` en `COLOR.HINT`. |
| Valide visuel | `:LED OK <FROM> <TO>` | `OK` | Flash/anim verte (`COLOR.OK`). |
| Erreur visuelle | `:LED FAIL <FROM> <TO>` | `OK` | Flash rouge (`COLOR.ERROR`). |

### 4.6 Cycle de mouvement (avec bouton physique)

| Commande | Syntaxe | Réponse | Rôle |
|---|---|---|---|
| Accepter coup | `:MOVE ACK <FROM> <TO> [PROMO=Q R B N] [CASTLE=K Q] [ENPASSANT=<SQ>]` | `OK` | Confirme FROM→TO ; la carte anime puis revient idle. |
| Refuser coup | `:MOVE NACK <FROM> <TO> [REASON=<txt>]` | `OK` | Flash erreur & retour au bon état. |
| Verrou | `:LOCK` | `OK` | Ignore reeds/bouton (anims). |
| Déverrouille | `:UNLOCK` | `OK` | Reprend le flux normal. |

> **Flux typique** :  
> `EVT LIFT E2` → `:LED MOVES E2 2 E3 E4` → `EVT PLACE E4` → `EVT MOVE E2 E4` → `:MOVE ACK E2 E4` → (anim OK) → `:LED OFF ALL`.

### 4.7 Configuration (incluant couleurs, debounce, orientation, baud)

| Commande | Syntaxe | Réponse | Notes |
|---|---|---|---|
| Lire tout | `:CFG?` | `OK CFG key=val ...` | État courant sérialisé. |
| Lire clé | `:CFG GET <KEY>` | `OK <KEY>=<val>` | — |
| Écrire | `:CFG SET <K1>=<v1> [<K2>=<v2> ...]` | `OK` | Multi-assignation autorisée. |
| Persister | `:SAVE` | `OK` | Écrit en flash. |

**Clés supportées**

- `DEBOUNCE_REED` = `5..200` (ms) — **défaut 30**  
- `DEBOUNCE_BTN` = `10..300` (ms) — **défaut 80**  
- `ORIENT` = `0|90|180|270` — rotation logique (remap cases)  
- `BRIGHT` = `0..255` — luminosité globale (alias `:LED BRIGHT`)  
- `GAMMA` = `0.5..3.0` — exponent gamma (alias `:LED GAMMA`)  
- `STREAM` = `ON|OFF` — émission des événements  
- `BAUD` ∈ `{9600,19200,57600,115200,230400,460800,921600}` — prise d’effet après `:RST`  
- `MASK` = `0x<64bits>` — masque d’événements ignorés (alias `:READ MASK SET`)  
- `COLOR.<NAME>` = `R,G,B` — persistance des couleurs nommées (ex.: `COLOR.HINT=80,160,255`)

**Exemples**
```text
:CFG SET DEBOUNCE_REED=25 DEBOUNCE_BTN=100 ORIENT=180
OK
:CFG SET BRIGHT=180 GAMMA=2.2 STREAM=ON
OK
:CFG SET BAUD=230400
OK
:SAVE
OK
:RST
OK
```

---

## 5) Codes d’erreur standard

- `ERR BAD_CMD` — commande inconnue  
- `ERR BAD_ARG` — argument manquant/mal formé  
- `ERR RANGE` — valeur hors limites  
- `ERR STATE` — commande invalide dans l’état courant  
- `ERR BUSY` — verrouillé/animation en cours  
- `ERR LEN` — longueur/hex incorrects (`LED MAP`)  
- `ERR TIMEOUT` — délai dépassé côté carte

Chaque erreur **peut** inclure un détail : `ERR RANGE DEBOUNCE_REED`.

---

## 6) Exemples de sessions

### 6.1 Coup accepté
```text
PCB→  EVT LIFT E2 t=102345
APP→  :LED MOVES E2 2 E3 E4
PCB→  EVT PLACE E4 t=103210
PCB→  EVT MOVE E2 E4 t=103500
APP→  :MOVE ACK E2 E4
PCB→  OK
APP→  :LED OFF ALL
PCB→  OK
```

### 6.2 Coup refusé
```text
PCB→  EVT LIFT B1 t=20100
APP→  :LED MOVES B1 1 A3
PCB→  EVT PLACE A4 t=20550
PCB→  EVT MOVE B1 A4 t=20600
APP→  :MOVE NACK B1 A4 REASON=ILLEGAL
PCB→  OK
```

### 6.3 Palette & bitboard
```text
APP→  :COLOR SET COLOR.HINT 80 160 255
OK
APP→  :LED BITBOARD 0x000000000000FF00
OK
```

---

## 7) Annexes

### 7.1 Notation des cases & index

Index linéaire `0..63` défini par :  
`index = (rank-1)*8 + (file-1)` avec `file∈{A..H}, rank∈{1..8}`.  
Exemples : `A1=0`, `B1=1`, …, `H1=7`, `A2=8`, …, `H8=63`.

### 7.2 Orientation (`ORIENT`)

La carte remappe les indices **avant** d’émettre/traiter :
- `ORIENT=0`   → identité  
- `ORIENT=90`  → rotation horaire 90°  
- `ORIENT=180` → rotation 180°  
- `ORIENT=270` → rotation horaire 270°  

> Implémentation conseillée : tables de correspondance de 64 entrées pré-calculées.

### 7.3 Timings recommandés

- **Debounce reeds** : 20–30 ms  
- **Debounce bouton** : 50–100 ms  
- **Timeout LIFTED** : 15 s  
- **Timeout PLACED** : 10 s  
- **Latence réponse** : `OK/ERR` sous 50 ms (hors animations longues)

### 7.4 Sécurité & robustesse

- Rejeter `:LED MAP` si `hex192` invalide (`ERR LEN`).
- Ignorer commandes non valides en **WAIT_APP** sauf `:MOVE ACK|NACK`, `:LOCK|:UNLOCK`, `:LED …`.
- Protéger `:CFG SET BAUD=` : appliquer **seulement après** `:SAVE` + `:RST` (ou mentionner clairement le besoin de `:RST`).

---

## 8) Changelog 

- **v1.0.0** : Version initiale ASCII avec couleurs nommées, debounce & config persistance.

---

