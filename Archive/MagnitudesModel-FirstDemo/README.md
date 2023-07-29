# SirenDetection
Internship's siren detection arduino implementation and python deep learning model

Arduino's folder :
  - arduinoFFT : adaptation of the FFT calculous to match the DFT python method results (original library : https://github.com/kosme/arduinoFFT)
  - mbmn974-projet-1_inferencing : trained Tensorflow model converted to an Arduino library by using EdgeImpulse
  - RP2040_EIModel.ino : main Arduino Code that implements the model library, the fft library (and the PDM library) and make prediction using the model and processed data (1024 magnitudes) as inputs.
