Partie Arduino :
-"SirenDetection.ino" : code Arduino implémentant : 	- l'acquisition d'un signal sous format PCM,
							- le traitement de ce signal pour en extraire des Mel-scale filterbanks,
							- l'utilisation d'un modèle TensorFlow pour prédire une classe en fonction des données extraites.
       
-"ArduinoRecording.ino" : code Arduino permettant d'envoyer sur le port série un signal audio sous format PCM d'1 seconde échantillonné à 8kHz (1s d'enregistrement et ~1s d'envoi).
 
-"libraries" : dossier contenant les libraries nécessaires au fonctionnement du code Arduino "SirenDetection.ino".
	
Fonctionnement et utilisation de la partie Arduino : pdf "Fonctionnement et utilisation du système de détection automatique de sirène de l'Arduino RP2040"
