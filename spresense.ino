#include "audio_recorder.h"
#include "mfcc.h"
#include <SDHCI.h>
#include <Audio.h>

AudioRecorder recorder;
SDClass theSD;
MFCC mfcc(48000, 2048, 26, 13); //Sample Rate, Number of ffts, Number of filter banks, Number of ceptrums

const int RECORDING_DURATION_MS = 5000;
const int RECORDINGS_PER_CYCLE = 3; // Number of recordings before waiting
const int WAIT_TIME_MS = 10000;     // Wait time between cycles (10 seconds)

std::vector<std::vector<float>> extractMFCC()
{
    // Initialize SD card
    while (!theSD.begin()) {
      Serial.println("Insert SD card.");
      delay(1000);
    }

    // Open the WAV file
    File myFile = theSD.open("compare.wav");
    if (!myFile) {
        Serial.println("Failed to open file");
        return std::vector<std::vector<float>>();  // Return an empty vector
    }

    // Create an AudioClass instance
    AudioClass *audio = AudioClass::getInstance();

    // Initialize audioData vector
    std::vector<float> audioData;

    // Read the WAV file and store the data in the vector
    while (myFile.available()) {
        int16_t buffer[1024];
        size_t bytesRead = myFile.read(buffer, sizeof(buffer));
        for (size_t i = 0; i < bytesRead / sizeof(int16_t); ++i) {
            audioData.push_back((float)buffer[i] / 32767.0f);  // Normalize to [-1.0, 1.0]
        }
    }

    // Close the file
    myFile.close();

    Serial.print("Recorded samples: ");
    if audioData.size() == 0:
    
    Serial.println(audioData.size());

    // Extract MFCC features
    std::vector<std::vector<float>> mfccFeatures = mfcc.mfcc(audioData);

    // Print MFCC features
    Serial.println("MFCC Features:");
    for (size_t i = 0; i < mfccFeatures.size(); ++i)
    {
        Serial.print("Frame ");
        Serial.print(i);
        Serial.print(": ");
        for (size_t j = 0; j < mfccFeatures[i].size(); ++j)
        {
            Serial.print(mfccFeatures[i][j], 4);
            Serial.print(" ");
        }
        Serial.println();
    }

    Serial.println("MFCC extraction complete");
    Serial.println();

    return mfccFeatures;
}

void setup()
{
  Serial.begin(115200);
  
}

void loop()
{
  Serial.println("Audio recording start!");
  // recorder.wav_recorder();
  std::vector<std::vector<float>> mfccFeatures = extractMFCC();


  Serial.println("Waiting before next recording cycle...");
  exit(1);
}

