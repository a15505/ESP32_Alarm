# ESP32_Alarm
Le système d'alarme possède les composants suivants:

1 Module ESP-32 vroom
1 Afficheur ACL Nokia 5110
1 Blindage pour ESP32
1 Module RFID RC522
1 Buzzer
1 Sirène (alarme)
3 zones (Détecteurs porte d'entrée, porte patio, détecteurs de mouvement)

Fonctionnement

Le module ESP32 détecte le code valide d'une carte magnétique pour s'armer ou se désarmer. Une fois armé, un délai s'applique jusqu'à l'activation du système. L'affficheur ACL affiche l'état du système. Le code est basé sur une « state machine ». En cas d'intrusion, la sirène est activée par le « cas » d'intrusion.


Prototype fonctionnel https://www.youtube.com/watch?v=QXhJJbaL5g0

Créé par Yvon Quémener 6 janvier 2024
