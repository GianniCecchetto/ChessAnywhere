# Mise en place de l'auto-déploiement et des tests unitaires

## Objectif de la mise en place

Pouvoir compiler et flasher un fichier binaire pour un STM32, ainsi que tester celui-ci à distance.

## Matériels nécessaires

Voici la liste du matériel nécessaire pour pouvoir effectuer ce tutoriel de A à Z.

- 1x Raspberry Pi 3 ou +
- 1x Carte Nucleo ou un STM32
- 1x Programmeur STK-LINK

## Mise en place de l'OS du Raspberry Pi

Cette étape est importante, car il est nécessaire d'utiliser un OS 64 bits pour établir cette mise en place.

img mise en place ... (à faire)

1. Choisir l'OS
   img
3. Sélectionner le board
   img
5. Sélectionner la SD, le stockage
   img
7. Modifier les paramètres pour que le SSH soit actif
img

## Connexion en SSH

Lorsque le Raspberry Pi est setup et alimenté, on peut y accéder en SSH.
Pour se faire, on peut utiliser par exemple PUTTY qui est un outil simple à prendre en main :

![putty](img/ssh_on_putty.png)

Etapes :
1. Spécifiez l'adresse IP du Raspberry Pi. Si vous avez du mal à trouver l'IP de ton Raspberry, vous pouvez utiliser un outil tel que Angry Ip Scanner qui vous permet de scanner tout votre réseau.
2. Sélectionne SSH comme connection type.
3. Appuie sur `Open`.

Une fenêtre similaire doit s'afficher qui vous demande votre nom d'utilisateur ainsi que votre mot de passe que vous avez configuré lors du setup de l'OS :

![connection](img/rasp_connection.png) 

Vous êtes désormais connecté au Raspberry Pi.

## Installation

Il faut désormais télécharger tous les outils nécessaires pour compiler et flasher le microcontrôleur :

```bash
$ sudo apt install -y \
    ca-certificates \
    curl \
    gnupg \
    lsb-release
```

Ensuite on continue avec la toolchain et `openocd` qui va nous permettre de flasher notre micro en ligne de commande :

```bash
$ sudo apt update
$ sudo apt install gcc-arm-none-eabi make cmake -y
$ sudo apt install openocd
```

### Mise à jour de la toolchain

Dans la plupart des cas, votre toolchain ne sera pas à jour. Il est important de mettre à jour celle-ci, sinon vous ne pourrez pas compiler votre projet STM32.

```bash
$ wget https://developer.arm.com/-/media/Files/downloads/gnu/13.3.rel1/binrel/arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi.tar.xz
$ tar -xf arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi.tar.xz
$ sudo mv arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi /opt/
$ export PATH=/opt/arm-gnu-toolchain-13.3.rel1-aarch64-arm-none-eabi/bin:$PATH
```

Vous pouvez vérifier que la procédure a bien fonctionné en faisant la commande suivante :

```bash
 $ arm-none-eabi-gcc --version
```

Le résultat suivant devrait apparaître :

```bash
arm-none-eabi-gcc (15:8-2019-q3-1+b1) 8.3.1 20190703 (release) [gcc-8-branch revision 273027]
Copyright (C) 2018 Free Software Foundation, Inc.
This is free software; see the source for copying conditions.  There is NO
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
```

Avant de continuer, il faut trouver quel est votre fichier config nécessaire pour le STM que vous utilisez, par exemple :

```bash
STM32F1 → stm32f1x.cfg
STM32F0 → stm32f0x.cfg
STM32F3 → stm32f3x.cfg
STM32F4 → stm32f4x.cfg
STM32L4 → stm32l4x.cfg
```

Donc, un autre exemple, si j'ai le `STM32L010RBT6`, alors je sais que je dois prendre le fichier de config `target/stm32l0.cfg`

Maintenant, il vous reste plus qu'à flasher votre STM32 avec la commande suivante (exemple avec le `STM32L010RBT6`) :

```bash
$ sudo openocd -f interface/stlink.cfg -f target/stm32l0.cfg \
  -c "program firmware.elf verify reset exit"
```

firmware.elf correspond au fichier binaire généré lors de la compilation de votre projet.

```bash

```

## Créer un Runner sur GitHub

## Automatisation de la compilation et du flash avec le Runner

## Auto-démarage du runner (Optionnel)

Actuellement, si l'on souhaite que notre Raspberry Pi tourne en tant que Runner, il est nécessaire de s'y connecter en SSH ou en USB et de lancer la commande :

```bash
$ ~/actions-runners $ ./run.sh
```

Cependant, il est possible de programmer cette commande au démrage du Raspberry Pi, afin de ne pas avoir une machine connectée en continu au Raspb. Pour se faire, il suffit :

1. Créez un fichier de service :
```bash
$ sudo nano /etc/systemd/system/runner.service
```
2. Dans ce fichier, on insère (adaptez, si nécessaire `WorkingDirectory`, `ExecStart` et `User`) :
```bash
[Unit]
Description=GitHub Actions Runner
After=network.target

[Service]
Type=simple
User=raspberrypi
WorkingDirectory=/home/raspberrypi/actions-runner
ExecStart=/home/raspberrypi/actions-runner/run.sh
Restart=always

[Install]
WantedBy=multi-user.target
```
3. Rechargez systemd :
```bash
$ sudo systemctl daemon-reload
```
4. Activez votre service au démarrage :
```bash
$ sudo systemctl enable runner.service
```
5. Démarrez-le immédiatement pour tester :
```bash
$ sudo systemctl start runner.service
```
6. Vérifiez qu’il tourne :
```bash
$ systemctl status runner.service
```

Pour vérifier que tout fonctionne correctement, il suffit de reboot notre Raspberry Pi :
```bash
$ sudo reboot
```

Et maintenant lorsque des modifications sont push, le test automatique doit apparaître dans la section `Actions` du répertoire.
