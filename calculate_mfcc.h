#ifndef CALCULATE_MFCC_H
#define CALCULATE_MFCC_H

#include <Arduino.h>

class MFCC {
public:
    MFCC();
    void begin(uint32_t sampleRate, uint8_t numCoefficients);
    void compute(const int16_t* pcmData, size_t pcmSize, float* mfccOut, size_t mfccSize);
    void end();

private:
    uint32_t sampleRate;
    uint8_t numCoefficients;
    // Additional private members and helper functions
};

#endif // CALCULATE_MFCC_H
#pragma once
