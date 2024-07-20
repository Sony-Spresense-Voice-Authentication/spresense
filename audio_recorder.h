#ifndef AUDIO_RECORDER_H
#define AUDIO_RECORDER_H

#include <SDHCI.h>
#include <Audio.h>
#include <vector>

class AudioRecorder
{
public:
    AudioRecorder() : sd(nullptr), audio(nullptr), isRecording(false), errorOccurred(false) {}

    ~AudioRecorder()
    {
        if (audio)
        {
            audio->setReadyMode();
            audio->end();
            // delete audio;
        }
        if (sd)
        {
            delete sd;
        }
    }

    bool begin()
    {
        sd = new SDClass();
        while (!sd->begin())
        {
            Serial.println("Insert SD card.");
            delay(1000);
        }

        audio = AudioClass::getInstance();
        audio->begin(audioAttentionCallback);

        audio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);
        audio->initRecorder(AS_CODECTYPE_WAV,
                            "/mnt/sd0/BIN",
                            SAMPLING_RATE,
                            BIT_DEPTH,
                            CHANNEL_COUNT);

        return true;
    }

    bool startRecording(const char *filename, uint32_t duration_ms)
    {
        if (isRecording)
        {
            return false;
        }

        if (sd->exists(filename))
        {
            sd->remove(filename);
        }

        recordFile = sd->open(filename, FILE_WRITE);
        if (!recordFile)
        {
            Serial.println("File open error");
            return false;
        }

        audio->writeWavHeader(recordFile);
        audio->startRecorder();
        isRecording = true;

        uint32_t startTime = millis();
        uint32_t recordingSize = (SAMPLING_RATE * CHANNEL_COUNT * BIT_DEPTH / 8) * (duration_ms / 1000);

        while (audio->getRecordingSize() < recordingSize && !errorOccurred)
        {
            err_t err = audio->readFrames(recordFile);
            if (err != AUDIOLIB_ECODE_OK)
            {
                break;
            }
        }

        stopRecording();
        return true;
    }

    bool stopRecording()
    {
        if (!isRecording)
        {
            return false;
        }

        audio->stopRecorder();
        audio->readFrames(recordFile);
        audio->closeOutputFile(recordFile);
        recordFile.close();
        isRecording = false;

        // Read recorded audio data
        File readFile = sd->open(recordFile.name(), FILE_READ);
        if (!readFile)
        {
            Serial.println("Failed to open file for reading");
            return false;
        }

        // Skip WAV header
        readFile.seek(44);

        // Read audio data
        int16_t sample;
        recordedAudio.clear();
        while (readFile.available() >= 2)
        {
            readFile.read(reinterpret_cast<uint8_t *>(&sample), 2);
            recordedAudio.push_back(static_cast<float>(sample) / 32768.0f);
        }

        readFile.close();
        return true;
    }

    std::vector<float> getRecordedAudio()
    {
        return recordedAudio;
    }

private:
    static void audioAttentionCallback(const ErrorAttentionParam *atprm)
    {
        Serial.println("Attention!");
        if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
        {
            Serial.println("Error occurred");
            // Set a flag to indicate an error occurred
            // Note: This is a static method, so we can't access instance variables directly
            // You may need to implement a different error handling mechanism
        }
    }

    SDClass *sd;
    AudioClass *audio;
    File recordFile;

    static const uint32_t SAMPLING_RATE = 16000;
    static const uint8_t CHANNEL_COUNT = 1;
    static const uint8_t BIT_DEPTH = 16;

    std::vector<float> recordedAudio;
    bool isRecording;
    bool errorOccurred;
};

#endif // AUDIO_RECORDER_H