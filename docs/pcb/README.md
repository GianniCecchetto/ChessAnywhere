## Documentation disponible

Le répertoire contient l’ensemble des fichiers nécessaires pour comprendre, assembler et utiliser la carte.  

- **README.md** : fichier principal de documentation (présent document).  
- **BOM.PDF** : *Bill of Materials* — liste complète des composants nécessaires au montage du PCB.  
- **Assembly Drawings.PDF** : vues d’assemblage pour faciliter le placement des composants sur la carte.  
- **Dimensional Drawings.PDF** : dessins cotés indiquant les dimensions mécaniques et contraintes d’intégration.  
- **PCB Top-Bottom Assy Dwgs.PDF** : représentation du PCB avec placement des composants côté top/bottom.  
- **Schematic_Prints.PDF** : schémas électroniques complets de la carte (alimentation, microcontrôleur, connecteurs, LEDs, capteurs).  
- **pinning_programmer.jpg** : schéma de correspondance des broches (pinout) pour la programmation via SWD.  

Ces documents doivent être consultés conjointement avec ce README afin d’assurer une compréhension complète du fonctionnement et du montage de la carte.


### 1.0 - Schéma

Le schéma électronique de la carte est organisé en plusieurs sous-blocs fonctionnels.  
Chaque bloc correspond à une partie précise de l’architecture matérielle : alimentation, microcontrôleur, interface de communication, matrice de capteurs et LEDs.

---

#### 1. Bloc alimentation (*supply.SchDoc*)
- **Entrées** : bornier à vis et jack 2,1 mm (5 V).  
- **Régulateur** : *LD1117S33CTR* produisant du **3,3 V** pour le microcontrôleur et la logique.  
- **Jumpers W1/W2** : permettent de couper les pistes avant/après le régulateur pour mesurer la consommation ou injecter une tension externe.  
- **Condensateurs** : découplage (100 nF) et filtrage (10 µF) pour stabiliser la tension.

---

#### 2. Microcontrôleur STM32 (*uc.SchDoc*)
- **MCU** : STM32G030F6P6 (ARM Cortex-M0+).  
- **Interfaces exposées** :
  - **USART2** (TX, RX, RTS, CTS) pour la communication série.  
  - **I²C1** (SCL, SDA) pour périphériques externes.  
  - **SWD** (SWCLK, SWIO, NRST) pour le flashage et le débogage.  
- **Connecteur P8** : pinning SWD standard pour ST-LINK.  
- **Jumpers W5/W8** : permettent de couper VCC/GND du MCU afin d’utiliser un contrôleur externe si nécessaire.

---

#### 3. Connecteurs externes (*connector.SchDoc*)
- **USB-C (USB4105-GF-A)** avec convertisseur USB-série (*FT231XS*), exposant les signaux USART2.  
- **Headers P2–P7** : accès aux alimentations (3,3 V / 5 V / GND), aux bus de communication (USART, I²C), et à la matrice ROW/COL.  
- **P9** : petit connecteur annexe pour signaux additionnels (USART/I²C).

---

#### 4. Matrice de capteurs (*matrix.SchDoc*)
- **Capteurs** : 64 interrupteurs REED disposés en matrice (ROW × COL).  
- **Décodage** :
  - *CD74HC238* (démultiplexeurs) pour activer les lignes.  
  - *SN74HC251* (multiplexeur) pour lire les colonnes.  
- **Transistors MOSFET (BSS138)** : commutation et adaptation de niveaux.  
- **Signal READ** : contrôle la phase de lecture de la matrice.

---

#### 5. LEDs WS2812B (*leds.SchDoc*)
- **Chaîne de 64 LEDs adressables WS2812B** (une par case de l’échiquier).  
- Chaque LED dispose de son condensateur de découplage (100 nF).  
- **Signal DIN/DOUT** : chaînage série des LEDs, le microcontrôleur envoie les données à la première LED qui relaie vers la suivante.  
- **Alimentation** : 5 V direct (haute consommation, prévoir une alimentation stable).

---

#### 6. Interconnexions globales
- **Bus ROW/COL** : relie la matrice des REEDs au microcontrôleur.  
- **Bus I²C et UART** : disponibles à la fois pour le MCU interne et un contrôleur externe via headers.  
- **LEDs et capteurs** : alimentés séparément mais synchronisés par le microcontrôleur.

---

### Résumé
Le schéma propose une architecture modulaire :  
- **Alimentation** (5 V → 3,3 V régulé).  
- **Microcontrôleur STM32** avec possibilité de contournement par contrôleur externe.  
- **Matrice REEDs** pour détecter les positions des pièces.  
- **LEDs WS2812B** pour rétroéclairage et signalisation.  
- **Connectivité complète** via USB, UART, I²C et SWD.

