#include "extract_pcm.h"

PCMExtractor::PCMExtractor() : dataSize(0), audioFormat(0), numChannels(0), sampleRate(0), bitsPerSample(0) {}

bool PCMExtractor::openWavFile(const char* filename) {
    file = SD.open(filename);
    if (!file) {
        Serial.println("Failed to open WAV file");
        return false;
    }
    return parseWavHeader();
}

size_t PCMExtractor::extractPCMData(int16_t* buffer, size_t bufferSize) {
    if (!file) {
        Serial.println("File not open");
        return 0;
    }
    return file.read((uint8_t*)buffer, bufferSize * sizeof(int16_t)) / sizeof(int16_t);
}

void PCMExtractor::closeWavFile() {
    if (file) {
        file.close();
    }
}

bool PCMExtractor::parseWavHeader() {
    if (!file) {
        return false;
    }

    // WAV header consists of 44 bytes
    uint8_t header[44];
    if (file.read(header, 44) != 44) {
        Serial.println("Failed to read WAV header");
        return false;
    }

    if (header[0] != 'R' || header[1] != 'I' || header[2] != 'F' || header[3] != 'F') {
        Serial.println("Invalid WAV file");
        return false;
    }

    dataSize = header[40] | (header[41] << 8) | (header[42] << 16) | (header[43] << 24);
    audioFormat = header[20] | (header[21] << 8);
    numChannels = header[22] | (header[23] << 8);
    sampleRate = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
    bitsPerSample = header[34] | (header[35] << 8);

    if (audioFormat != 1) {
        Serial.println("Unsupported WAV format");
        return false;
    }

    if (bitsPerSample != 16) {
        Serial.println("Unsupported bits per sample");
        return false;
    }

    return true;
}

