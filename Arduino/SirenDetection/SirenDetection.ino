#include <mbmn974-project-1_inferencing.h>

#include "arduinoFFT.h"

#include <PDM.h>

#include <WiFiNINA.h>//pour utiliser la led RGB de la carte; à enlever si plus besoin de la led

#define RAW_SAMPLE_COUNT 2048 //8192          //longueur du signal à analyser
#define FREQUENCY 8000                        //fréquence d'enregistrement du signal
#define CHANNELS 1                            //nombre de canaux de l'enregistrement
#define SPLIT_AUDIO_LENGTH 1 //2              //permet de diviser le traitement du signal en plusieurs traitements indépendants (pour économiser de la mémoire)
                                              //et de concaténer ces différents traitements à la fin

static const uint16_t windowSize = 512;//512; //taille de la fenêtre glissante
static const uint16_t hopLength = 384;//448;  //recouvrement de la fenêtre
static const uint8_t nbFilters = 32;          //nombre de filtre du Mel-scale filterbank
static const uint8_t nbFrames = int((RAW_SAMPLE_COUNT/SPLIT_AUDIO_LENGTH-windowSize)/hopLength)+1; //nombre de frames de Mel-scale filterbanks par traitement du signal indépendant

/** Audio buffers, pointers and selectors */
//structure permettant de stocker le signal audio acquis par [le microphone PDM et le buffer PCM], et permettant de réaliser le traitement audio
typedef struct {
    int16_t *buffer;
    double min, max;
    uint8_t buf_ready;
    uint32_t buf_count;
    uint32_t n_samples;
    bool analyze;
} inference_t;

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */

static inference_t inference;               //stockage du signal audio
static signed short sampleBuffer[2048];     //buffer du micro PDM, stocké en PCM
static volatile bool record_ready = false;

float coefficients[nbFrames*SPLIT_AUDIO_LENGTH][nbFilters]; //stockage des Mel-scale filterbanks

/**
 * @brief      Copy raw feature data in out_ptr
 *             Function called by inference library
 *
 * @param[in]  offset   The offset
 * @param[in]  length   The length
 * @param      out_ptr  The out pointer
 *
 * @return     0
 */
int raw_feature_get_data(size_t offset, size_t length, float *out_ptr) {
    memcpy(out_ptr, coefficients + offset, length * sizeof(float));
    return 0;
}

/**
 * @brief      Arduino setup function
 */
void setup()
{
  // put your setup code here, to run once:
  //Serial.begin(115200); //à enlever si carte utilisée sans une connexion à un port série (sur batterie externe par exemple)

  // comment out the below line to cancel the wait for USB connection (needed for native USB)
  //while (!Serial);

  pinMode(LEDB, OUTPUT);

  if (microphone_inference_start(RAW_SAMPLE_COUNT) == false) {
        //Serial.println("ERR: Could not allocate audio buffer");
        return;
  }
}

/**
 * @brief      Arduino main function
 */
void loop()
{
  if(inference.analyze){
    //Traitement du signal audio enregistré
    uint8_t framesCounter = 0;
    for(uint8_t s=0; s<SPLIT_AUDIO_LENGTH;s++){ //si le traitement du signal se fait sur des parties indépendantes

      double* framesBuffer = new double[nbFrames * (windowSize >> 1)];

      for(uint8_t n = 0; n<nbFrames; n++){
        double* bufferData = new double[windowSize];
        double* bufferImag = new double[windowSize];
        for(uint16_t i = 0; i<windowSize; i++){
          //Normalisation
          bufferData[i] = normalization(double(inference.buffer[i+(n*hopLength)+(s*(RAW_SAMPLE_COUNT/SPLIT_AUDIO_LENGTH))]), inference.min, inference.max);
          bufferImag[i] = 0;
        }

        //FFT et calcul des magnitudes
        FFT.Windowing(bufferData, windowSize, FFT_WIN_TYP_HAMMING, FFT_FORWARD); // Weigh data
        FFT.Compute(bufferData, bufferImag, windowSize, FFT_FORWARD); // Compute FFT
        FFT.ComplexToMagnitude(bufferData, bufferImag, windowSize); // Compute magnitudes
        
        delete[] bufferImag;
        
        for(uint16_t i = 0; i<windowSize>>1; i++){
          framesBuffer[n * (windowSize >> 1) + i] = bufferData[i];
        }

        delete[] bufferData;
      }

      //Calcul des Mel-scale filterbanks
      FFT.MelCoefficients(framesBuffer, nbFrames, windowSize, nbFilters, FREQUENCY);

      //conversion double vers float et stockage des Mel-scale filterbanks dans le bon format pour les méthodes EdgeImpulse
      for (int i = 0 ; i < nbFrames; i++)
      {
        for (int j = 0; j < nbFilters; j++){
          coefficients[i+framesCounter][j] = (float) framesBuffer[i * nbFilters + j];
        }
      }

      delete[] framesBuffer;
      
      framesCounter += nbFrames;
    }

    //Prédiction à partir des frames de Mel-scale filterbanks

    ei_impulse_result_t result = { 0 };

    // the features are stored into flash, and we don't want to load everything into RAM
    signal_t features_signal;
    features_signal.total_length = sizeof(coefficients) / sizeof(coefficients[0][0]);
    features_signal.get_data = &raw_feature_get_data;

      //Prédiction à partir des features traitées en mémoire flash et résultat stocké dans result
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false);//  debug);
    if (res != EI_IMPULSE_OK) {
        //ei_printf("ERR: Failed to run classifier (%d)\n", res);
        return;
    }

      //Accès à la valeur prédite
    float value = result.classification[0].value;

    //Traitement de la valeur prédite

    //Serial.println(value);

    if(value >= 0.5){
      digitalWrite(LEDB, HIGH);
    }
    else{
      digitalWrite(LEDB, LOW);
    }

    inference.analyze = false;
  }

  bool m = microphone_inference_record();
  if (!m) {
      //Serial.println("ERR: Failed to record audio...\n");
      return;
  }
    
}

/**
 * @brief      PDM buffer full callback
 *             Copy audio data to app buffers
 */
void pdm_data_ready_inference_callback(void)
{

  int bytesAvailable = PDM.available();

  // read into the sample buffer
  int bytesRead = PDM.read((char *)&sampleBuffer[0], bytesAvailable);

  if ((inference.buf_ready == 0) && (record_ready == true)) {

      for(int i = 0; i < bytesRead>>1; i++) {
          inference.buffer[inference.buf_count++] = sampleBuffer[i];
          if(sampleBuffer[i]<inference.min){
            inference.min = sampleBuffer[i];
          }
          else if(sampleBuffer[i]>inference.max){
            inference.max = sampleBuffer[i];
          }

          if(inference.buf_count >= inference.n_samples) {
              inference.buf_count = 0;
              inference.analyze = true;
              inference.buf_ready = 1;
              break;
          }
      }
  }
}

//Initialisation de la structure de stockage audio 'inference' et configuration du microphone PDM
static bool microphone_inference_start(uint32_t n_samples)
{
    inference.buffer = (int16_t *)malloc(n_samples * sizeof(int16_t));

    if(inference.buffer == NULL) {
        //Serial.println("Could not allocate");
        return false;
    }

    inference.buf_count  = 0;
    inference.n_samples  = n_samples;
    inference.buf_ready  = 0;
    inference.analyze = false;

    // configure the data receive callback
    PDM.onReceive(pdm_data_ready_inference_callback);

    PDM.setGain(40);

    PDM.setBufferSize(2048);
    delay(250);

    // initialize PDM with:
    // - one channel (mono mode)
    if (!PDM.begin(CHANNELS, FREQUENCY)) {
        //Serial.println("ERR: Failed to start PDM!");
        microphone_inference_end();
        return false;
    }

    return true;
}

/**
 * @brief      Stop PDM and release buffers
 */
static void microphone_inference_end(void)
{
    PDM.end();
    free(inference.buffer);
}

/**
 * @brief      Wait on new data
 *
 * @return     True when finished
 */
static bool microphone_inference_record(void)
{
    bool ret = true;

    inference.min = 32767;
    inference.max = -32768;

    record_ready = true;
    while (inference.buf_ready == 0) {
        delay(10);
    }

    inference.buf_ready = 0;
    record_ready = false;

    return ret;
}

//min-max normalization of an audio sample
double normalization(double value, double min, double max){
  return (((value - min) / (max - min))*2)-1;
}

