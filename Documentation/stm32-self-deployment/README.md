# Mise en place de l'auto-déploiement et des tests unitaires

## Objectif de la mise en place

Pouvoir compiler et flasher un fichier binaire pour un STM32, ainsi que tester celui-ci à distance.

## Matériels nécessaires

Voici la liste du matériel nécessaire pour pouvoir effectuer ce tutoriel de A à Z.

- 1x Raspberry Pi 3 ou +
- 1x Carte Nucleo ou un STM32
- 1x Programmeur STK-LINK

## Auto-démarage du runner (Optionnel)

Actuellement, si l'on souhaite que notre Raspberry Pi tourne en tant que Runner, il est nécessaire de s'y connecter en SSH ou en USB et de lancer la commande :

```bash
~/actions-runners $ ./run.sh
```

Cependant, il est possible de programmer cette commande au démrage du Raspberry Pi, afin de ne pas avoir une machine connectée en continu au Raspb. Pour se faire, il suffit :

1. Créez un fichier de service :
```bash
sudo nano /etc/systemd/system/runner.service
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
sudo systemctl daemon-reload
```
4. Activez votre service au démarrage :
```bash
sudo systemctl enable runner.service
```
5. Démarrez-le immédiatement pour tester :
```bash
sudo systemctl start runner.service
```
6. Vérifiez qu’il tourne :
```bash
systemctl status runner.service
```

Pour vérifier que tout fonctionne correctement, il suffit de reboot notre Raspberry Pi :
```bash
sudo reboot
```

Et maintenant lorsque des modifications sont push, le test automatique doit apparaître dans la section `Actions` du répertoire.
