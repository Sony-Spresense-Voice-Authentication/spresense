#ifndef RECORD_AUDIO_H
#define RECORD_AUDIO_H

#include <Arduino.h>
#include <SD.h>
#include <Audio.h>

class AudioRecorder {
public:
    AudioRecorder();
    void begin();
    void startRecording(const char* filename);
    void stopRecording();
    bool isRecording();
    void update();

private:
    SDLib::File file;  // 使用SD库的File类
    bool recording;
};

#endif // RECORD_AUDIO_H

