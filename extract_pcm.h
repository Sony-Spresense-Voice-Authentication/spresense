#ifndef EXTRACT_PCM_H
#define EXTRACT_PCM_H

#include <Arduino.h>
#include <SD.h>

class PCMExtractor {
public:
    PCMExtractor();
    bool openWavFile(const char* filename);
    size_t extractPCMData(int16_t* buffer, size_t bufferSize);
    void closeWavFile();

private:
    SDLib::File file;  // 使用SD库的File类
    bool parseWavHeader();
    uint32_t dataSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
};

#endif // EXTRACT_PCM_H



