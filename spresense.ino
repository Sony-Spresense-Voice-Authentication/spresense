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
    while (!Serial)
    {
        ; // wait for serial port to connect
    }

    if (!recorder.begin())
    {
        Serial.println("Failed to initialize AudioRecorder");
        while (1)
            ;
    }

    Serial.println("Audio recorder initialized. Starting continuous recording and MFCC extraction.");
}

void loop()
{
    for (int i = 0; i < RECORDINGS_PER_CYCLE; ++i)
    {
        recordAndExtractMFCC();
    }

    Serial.println("Waiting before next recording cycle...");
    delay(WAIT_TIME_MS);
}

void recordAndExtractMFCC()
{
    char filename[20];
    snprintf(filename, sizeof(filename), "rec_%lu.wav", millis());

    Serial.print("Recording 5 seconds of audio to ");
    Serial.println(filename);

    if (recorder.startRecording(filename, RECORDING_DURATION_MS))
    {
        Serial.println("Recording complete");
    }
    else
    {
        Serial.println("Recording failed");
        return;
    }

    std::vector<float> audioData = recorder.getRecordedAudio();
    Serial.print("Recorded samples: ");
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
}