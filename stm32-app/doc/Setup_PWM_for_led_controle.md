# Configuration PWM + DMA pour led addressable NEOpixel WS2812b
- environnement: STM32(CubeMX ou cubeIDE)
- board de dévloppement: STM32 L432KC (Nucleo)
## 1. Sélection du Timer et du Canal

* Ouvrir **Pinout & Configuration**
* Activer un **Timer avancé** (ex : `TIM1`)
* Sélectionner un canal en mode **PWM Generation** (ex : `TIM1_CH1`)
* CubeMX affecte automatiquement une GPIO (ex : `PA8`) en `Alternate Function (AF)`.


---

## 2. Calcul de la fréquence PWM

La fréquence PWM est donnée par :

$$
f_{PWM} = \frac{f_{TIM}}{(PSC + 1) \times (ARR + 1)}
$$

* $f_{TIM}$ = fréquence d’horloge du timer (dans notre cas = APB1, Attention un prescaler peux lui être appliqué).
* $PSC$ = Prescaler
* $ARR$ = Auto-Reload Register (Counter Period)

La période d’un slot WS2812 doit être environ **1,25 µs** (soit **800 kHz**).

Exemple avec $f_{TIM} = 32\,MHz$ :

$$
ARR = \frac{f_{TIM}}{f_{PWM}} - 1
$$

$$
ARR = \frac{32\,000\,000}{800\,000} - 1 = 39
$$

Donc :

* **PSC = 0**
* **ARR = 39**
  → Période = 1,25 µs

---

## 3. Paramètres du Timer

Dans **Configuration → TIM1 → Parameter Settings → Counter Settings** :

* **Prescaler (PSC)** : `0`
* **Counter Period (ARR)** : `39` (pour 32 MHz → 800 kHz)
* **Counter Mode** : `Up`
* **Auto-Reload Preload (ARPE)** : `Enable`
* **Repetition Counter** : `0`

---

## 4. Paramètres PWM (Channel 1)

Toujours dans l’onglet TIM1 :

* **Mode** : `PWM Generation CH1`
* **Pulse** : valeur mise à jour par le **DMA** (duty cycle)
* **Polarity** : `High`

---

## 5. Configuration du DMA

Dans **DMA Settings** → Add :

* **Request** : `TIM1_CH1`
* **Direction** : `Memory to Peripheral`
* **Mode** : `Normal`
* **Priority** : `Medium`
* **Memory increment** : `Enabled`
* **Peripheral increment** : `Disabled`
* **Data Width** :

  * Memory : `Half Word (16 bits)`
  * Peripheral : `Half Word (16 bits)`


## 6. NVIC (Interrupts)

Dans **NVIC Settings** :

* Activer l’interruption du canal DMA (ex : `DMA1 Channel2 Global Interrupt`).
* Priorité par défaut = `0`.


## 7. GPIO Settings

Vérifier la sortie PWM (ex : `PA8`) :

* **Mode** : `Alternate Function`
* **Output Type** : `Push-Pull`
* **Pull** : `No Pull`
* **Speed** : `n/a`



## Tableau des valeurs de ARR en fonction de la féquence pour obtenir 800Khz

| Fréquence système (fCLK) | PSC | ARR |
| ------------------------ | --- | --- |
| 8 MHz                    | 0   | 9   |
| 16 MHz                   | 0   | 19  |
| 32 MHz                   | 0   | 39  |
| 48 MHz                   | 0   | 59  |
| 72 MHz                   | 0   | 89  |


