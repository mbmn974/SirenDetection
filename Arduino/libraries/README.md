Libraries :

-"mbmn974-project-1_inferencing.h" : dernier modèle TensorFlow implémenté (analyse d'extraits de 0.25s avec 5 frames de 32 Mel-scale filter bank coefficients) (~1250ms de latence) (métriques : RAW_SAMPLE_COUNT 2048 | SPLIT_AUDIO_LENGTH 1 | windowSize = 512 | hopLength = 384)
		
-"OtherModels" : 

_05s : modèle TensorFlow analysant des extraits de 0.5s (9 frames de 32 coefs) (~2400ms de latence) (métriques : RAW_SAMPLE_COUNT 4096 | SPLIT_AUDIO_LENGTH 1 | windowSize = 512 | hopLength = 448)

_1s  : modèle TensorFlow analysant des extraits d'1s (18 frames de 32 coefs) (~4600ms de latence) (métriques : RAW_SAMPLE_COUNT 8192 | SPLIT_AUDIO_LENGTH 2 | windowSize = 512 | hopLength = 448)

_Possible de changer le modèle testé en adaptant les métriques dans le code "SirenDetection.ino" et en remplaçant le dossier du modèle par celui voulu dans le dossier "librairies" d'Arduino.

-"arduinoFFT.h" : library permettant de calculer les FFT et les magnitudes d'un signal audio (source : https://github.com/kosme/arduinoFFT)
Modifications des méthodes Windowing et ComplexToMagnitude, ajout de la méthode MelCoefficients (méthode se basant sur http://practicalcryptography.com/miscellaneous/machine-learning/guide-mel-frequency-cepstral-coefficients-mfccs/ et https://haythamfayek.com/2016/04/21/speech-processing-for-machine-learning.html).
