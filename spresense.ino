#include "audio_recorder.h"
#include "mfcc.h"

AudioRecorder recorder;
MFCC mfcc(48000, 512, 26, 13); //Sample Rate, Number of ffts, Number of filter banks, Number of ceptrums

const int RECORDING_DURATION_MS = 5000;
const int RECORDINGS_PER_CYCLE = 3; // Number of recordings before waiting
const int WAIT_TIME_MS = 10000;     // Wait time between cycles (10 seconds)

void setup()
{
  Serial.begin(115200);
  
}

void loop()
{
  Serial.println("Audio recording start!");
  // recorder.wav_recorder();

  Serial.println("Waiting before next recording cycle...");
  exit(1);
}

// void extractMFCC()
// {
  

//     std::vector<float> audioData = ;
//     Serial.print("Recorded samples: ");
//     Serial.println(audioData.size());

//     // Extract MFCC features
//     std::vector<std::vector<float>> mfccFeatures = mfcc.mfcc(audioData);

//     // Print MFCC features
//     Serial.println("MFCC Features:");
//     for (size_t i = 0; i < mfccFeatures.size(); ++i)
//     {
//         Serial.print("Frame ");
//         Serial.print(i);
//         Serial.print(": ");
//         for (size_t j = 0; j < mfccFeatures[i].size(); ++j)
//         {
//             Serial.print(mfccFeatures[i][j], 4);
//             Serial.print(" ");
//         }
//         Serial.println();
//     }

//     Serial.println("MFCC extraction complete");
//     Serial.println();
// }