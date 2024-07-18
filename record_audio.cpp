#include "record_audio.h"

AudioRecorder::AudioRecorder() : recording(false) {}

void AudioRecorder::begin() {
    Audio.begin(16000, 1); // 16kHz, mono
}

void AudioRecorder::startRecording(const char* filename) {
    if (recording) return;

    file = SD.open(filename, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file for writing");
        return;
    }
    recording = true;
}

void AudioRecorder::stopRecording() {
    if (!recording) return;

    recording = false;
    file.close();
    Audio.end();
}

bool AudioRecorder::isRecording() {
    return recording;
}

void AudioRecorder::update() {
    if (recording) {
        int16_t sample = Audio.input.read();
        file.write((uint8_t*)&sample, sizeof(sample));
    }
}
