#ifndef READ_WAV_H
#define READ_WAV_H

#include <Arduino.h>
#include <SDHCI.h>

class WavFileReader {
public:
    WavFileReader();
    bool open(const char* filename);
    size_t readSamples(int16_t* buffer, size_t bufferSize);
    void close();

private:
    File file;
    bool parseHeader();
    uint32_t dataSize;
    uint16_t audioFormat;
    uint16_t numChannels;
    uint32_t sampleRate;
    uint16_t bitsPerSample;
};

#endif // READ_WAV_H

