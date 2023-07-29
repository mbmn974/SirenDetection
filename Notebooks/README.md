Jupyter Notebooks :

CNNBinaryClassification : entraînement et enregistrement d'un réseau neuronal convolutif (Convolutional neural network (CNN)) avec les datasets de "sirènes" et de "contre exemples" d'où sont extraits des cofficients de Mel-scale filter bank.

________________________________________________________________________________________________________

ArduinoSoundRecoder : lecture du port série d'une carte Arduino branchée pour enregistrer les valeurs qu'elle envoie. Possibilité de convertir les valeurs PCM reçues en fichier .wav.

________________________________________________________________________________________________________

SoundVisualization : affichage de sons des datasets "sirènes" et 'contre exemples" (affichage de l'amplitude, des coefficients Mel-scale filter bank, des MFCC, du spectrogramme et écoute du son possible).

________________________________________________________________________________________________________

Tools : plusieurs méthodes utilitaires : 	

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-ajout de background noise avec Audiomentation (pour déterminer un SNR),

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-séparation d'un dataset en dataset de train et de test (pour avoir un jeu de test fixe),

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-conversion d'un fichier au format .rw (format d'enregistrement d'un signal PCM Arduino) vers le format .wav (pour utiliser Audiomentation notamment),

&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;-ajout de background noise aux jeux de test fixes pour avoir une augmentation fixe de ces jeux de données.
