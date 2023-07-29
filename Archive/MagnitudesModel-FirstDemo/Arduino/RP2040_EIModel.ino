/* Edge Impulse ingestion SDK
 * Copyright (c) 2022 EdgeImpulse Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

/* Includes ---------------------------------------------------------------- */
#include <mbmn974-project-1_inferencing.h>

#include "arduinoFFT.h"

#include <PDM.h>

//#include <WiFiNINA.h>//pour utiliser la led RGB de la carte; à enlever quand fin de démo

#define SCL_INDEX 0x00
#define SCL_TIME 0x01
#define SCL_FREQUENCY 0x02
#define SCL_PLOT 0x03

//nb of output channels
static const char channels = 1;

//pcm frequency
static const int frequency = 16000;

// Number of audio samples read
volatile int samplesRead;

int storeSize;

arduinoFFT FFT = arduinoFFT(); /* Create FFT object */
/*
These values can be changed in order to evaluate the functions
*/
const uint16_t samples = 2048;//64; //This value MUST ALWAYS be a power of 2

const double samplingFrequency = 16000;//5000;
//const uint8_t amplitude = 100;

//buffer to read samples into
short sampleBuffer[samples*2];
//buffer to store samples  // can store up to 256 values ? why ?
double storeBuffer[samples]; //pas besoin du *2 car double ? (plus de valeurs possible)

float magnitudes[samples];

/*
These are the input and output vectors
Input vectors receive computed results from FFT
*/
//double vReal[samples];
double vImag[samples];

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
    memcpy(out_ptr, magnitudes + offset, length * sizeof(float));
    return 0;
}

/**
 * @brief      Arduino setup function
 */
void setup()
{
  // put your setup code here, to run once:
  Serial.begin(115200);
  // comment out the below line to cancel the wait for USB connection (needed for native USB)
  while (!Serial);
  //Serial.println("Edge Impulse Inferencing Demo");

  //pinMode(LEDB, OUTPUT);

  storeSize = 0;

    // Configure the data receive callback
  PDM.onReceive(onPDMdata);

  // Optionally set the gain
  // Defaults to 20 on the BLE Sense and 24 on the Portenta Vision Shield
  PDM.setGain(40);

  // Initialize PDM with:
  // - one channel (mono mode)
  // - a 16 kHz sample rate for the Arduino Nano 33 BLE Sense
  // - a 32 kHz or 64 kHz sample rate for the Arduino Portenta Vision Shield
  if (!PDM.begin(channels, frequency)) {
    Serial.println("Failed to start PDM!");
    while (1);
  }
}

/**
 * @brief      Arduino main function
 */
void loop()
{
  if(storeSize == samples){
    //Calcul des 128 magnitudes
    FFT.Windowing(storeBuffer, samples, FFT_WIN_TYP_HAMMING, FFT_FORWARD);	/* Weigh data */
    FFT.Compute(storeBuffer, vImag, samples, FFT_FORWARD); /* Compute FFT */
    FFT.ComplexToMagnitude(storeBuffer, vImag, samples); /* Compute magnitudes */
    //

    //convertion double vers float
    for (int i = 0 ; i < samples; i++)
    {
        magnitudes[i] = (float) storeBuffer[i];
    }

    //Prédiction de la classe de l'extrait audio en utilisant le modèle MLP avec les magnitudes
    ei_impulse_result_t result = { 0 };

    // the features are stored into flash, and we don't want to load everything into RAM
    signal_t features_signal;
    features_signal.total_length = sizeof(magnitudes) / sizeof(magnitudes[0]);
    features_signal.get_data = &raw_feature_get_data;

    // invoke the impulse
    EI_IMPULSE_ERROR res = run_classifier(&features_signal, &result, false /* debug */);
    if (res != EI_IMPULSE_OK) {
        ei_printf("ERR: Failed to run classifier (%d)\n", res);
        return;
    }

    // print inference return code
    //ei_printf("run_classifier returned: %d\r\n", res);

    float value = result.classification[0].value;

    //for (uint16_t i = 0; i < EI_CLASSIFIER_LABEL_COUNT; i++) {
    //ei_printf("%.5f\r\n", result.classification[0].value);
    Serial.println(value); //indice 0 car 1 seule prediction
    //}

    /*if(value >= 0.5){
      digitalWrite(LEDB, HIGH);
    }
    else{
      digitalWrite(LEDB, LOW);
    }*/

    //while(1); /* Run Once */
    //delay(2000); /* Repeat after delay */

    storeSize=0;
  }
  if(samplesRead){
    /*for (int i = 0; i < samplesRead; i++) {
      Serial.println(sampleBuffer[i]);
    }*/
    //Serial.println(storeSize+1);
    samplesRead = 0;
  }

    
}

void onPDMdata() {
  // Query the number of available bytes
  int bytesAvailable = PDM.available();

  // Read into the sample buffer
  PDM.read(sampleBuffer, bytesAvailable);

  // 16-bit, 2 bytes per sample
  samplesRead = bytesAvailable / 2;

  int reached = 0;
  for(int i = 0; i<samplesRead; i++){
    if(storeSize + i== samples){
      break;
    }
    storeBuffer[storeSize + i] = (double)sampleBuffer[i];
    vImag[storeSize + i] = 0.0;
    reached = i+1;
  }
  storeSize = storeSize + reached; //prochain index
}