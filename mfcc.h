#ifndef MFCC_H
#define MFCC_H

#include <Arduino.h>
#include <vector>
#include <complex>
#include <cmath>

class MFCC
{
public:
    MFCC(int sampleRate = 16000, int nfft = 2048, int nfilt = 26, int numCeps = 13,
         float preemph = 0.97, float lowFreq = 0, float highFreq = NULL,
         bool appendEnergy = true)
        : sampleRate(sampleRate), nfft(nfft), nfilt(nfilt), numCeps(numCeps),
          preemph(preemph), lowFreq(lowFreq), appendEnergy(appendEnergy)
    {
        if (highFreq == NULL)
        {
            highFreq = sampleRate / 2;
        }
        this->highFreq = highFreq;

        // Compute mel filter bank
        melFilterbank = computeMelFilterbank();

        // Compute DCT matrix
        dctMatrix = computeDCTMatrix();
    }

    std::vector<std::vector<float>> mfcc(const std::vector<float> &signal, int winLen = 0, int winStep = 0,
                                         int winfunc = 1)
    {
        if (winLen == 0)
        {
            winLen = 0.025 * sampleRate;
        }
        if (winStep == 0)
        {
            winStep = 0.01 * sampleRate;
        }

        // Pre-emphasis
        std::vector<float> emphasizedSignal = preEmphasis(signal);

        // Framing
        std::vector<std::vector<float>> frames = framesSignal(emphasizedSignal, winLen, winStep);

        // Windowing
        applyWindow(frames, winfunc);

        // Compute power spectrum
        std::vector<std::vector<float>> powSpec = powerSpectrum(frames);

        // Apply mel filterbank
        std::vector<std::vector<float>> filteredSpec = applyMelFilterbank(powSpec);

        // Compute MFCC
        std::vector<std::vector<float>> mfccFeatures = computeMFCC(filteredSpec);

        return mfccFeatures;
    }

private:
    int sampleRate;
    int nfft;
    int nfilt;
    int numCeps;
    float preemph;
    float lowFreq;
    float highFreq;
    bool appendEnergy;
    std::vector<std::vector<float>> melFilterbank;
    std::vector<std::vector<float>> dctMatrix;

    std::vector<float> preEmphasis(const std::vector<float> &signal)
    {
        std::vector<float> emphasizedSignal(signal.size());
        emphasizedSignal[0] = signal[0];
        for (size_t i = 1; i < signal.size(); ++i)
        {
            emphasizedSignal[i] = signal[i] - preemph * signal[i - 1];
        }
        return emphasizedSignal;
    }

    std::vector<std::vector<float>> framesSignal(const std::vector<float> &signal, int frameLen, int frameStep)
    {
        int numFrames = 1 + (signal.size() - frameLen) / frameStep;
        std::vector<std::vector<float>> frames(numFrames, std::vector<float>(frameLen));

        for (int i = 0; i < numFrames; ++i)
        {
            for (int j = 0; j < frameLen; ++j)
            {
                frames[i][j] = signal[i * frameStep + j];
            }
        }

        return frames;
    }

    void applyWindow(std::vector<std::vector<float>> &frames, int winfunc)
    {
        int frameLen = frames[0].size();
        std::vector<float> window;

        if (winfunc == 1)
        { // Hamming window
            window = hammingWindow(frameLen);
        }
        else
        { // Default to rectangular window
            window = std::vector<float>(frameLen, 1.0f);
        }

        for (auto &frame : frames)
        {
            for (size_t i = 0; i < frame.size(); ++i)
            {
                frame[i] *= window[i];
            }
        }
    }

    std::vector<float> hammingWindow(int size)
    {
        std::vector<float> window(size);
        for (int i = 0; i < size; ++i)
        {
            window[i] = 0.54f - 0.46f * cos(2 * M_PI * i / (size - 1));
        }
        return window;
    }

    std::vector<std::vector<float>> powerSpectrum(const std::vector<std::vector<float>> &frames)
    {
        int numFrames = frames.size();
        int frameLen = frames[0].size();
        std::vector<std::vector<float>> powSpec(numFrames, std::vector<float>(nfft / 2 + 1));

        for (int i = 0; i < numFrames; ++i)
        {
            std::vector<std::complex<float>> fftResult = fft(frames[i]);
            for (int j = 0; j <= nfft / 2; ++j)
            {
                float magnitude = std::abs(fftResult[j]);
                powSpec[i][j] = (1.0f / nfft) * (magnitude * magnitude);
            }
        }

        return powSpec;
    }

    std::vector<std::complex<float>> fft(const std::vector<float> &signal)
    {
        int n = signal.size();
        if (n <= 1)
        {
            return std::vector<std::complex<float>>(signal.begin(), signal.end());
        }

        std::vector<std::complex<float>> even = fft(std::vector<float>(signal.begin(), signal.begin() + n / 2));
        std::vector<std::complex<float>> odd = fft(std::vector<float>(signal.begin() + n / 2, signal.end()));

        std::vector<std::complex<float>> result(n);
        for (int k = 0; k < n / 2; ++k)
        {
            float angle = -2 * M_PI * k / n;
            std::complex<float> t = std::polar(1.0f, angle) * odd[k];
            result[k] = even[k] + t;
            result[k + n / 2] = even[k] - t;
        }

        return result;
    }

    std::vector<std::vector<float>> computeMelFilterbank()
    {
        float lowMel = hzToMel(lowFreq);
        float highMel = hzToMel(highFreq);
        std::vector<float> melPoints(nfilt + 2);

        for (int i = 0; i < nfilt + 2; ++i)
        {
            melPoints[i] = lowMel + (highMel - lowMel) * i / (nfilt + 1);
        }

        std::vector<float> fftFreqs(nfft / 2 + 1);
        for (int i = 0; i <= nfft / 2; ++i)
        {
            fftFreqs[i] = i * sampleRate / nfft;
        }

        std::vector<std::vector<float>> filterbank(nfilt, std::vector<float>(nfft / 2 + 1, 0.0f));

        for (int i = 0; i < nfilt; ++i)
        {
            float leftMel = melPoints[i];
            float centerMel = melPoints[i + 1];
            float rightMel = melPoints[i + 2];

            for (int j = 0; j <= nfft / 2; ++j)
            {
                float mel = hzToMel(fftFreqs[j]);
                if (mel > leftMel && mel < rightMel)
                {
                    if (mel <= centerMel)
                    {
                        filterbank[i][j] = (mel - leftMel) / (centerMel - leftMel);
                    }
                    else
                    {
                        filterbank[i][j] = (rightMel - mel) / (rightMel - centerMel);
                    }
                }
            }
        }

        return filterbank;
    }

    float hzToMel(float hz)
    {
        return 2595 * log10(1 + hz / 700);
    }

    float melToHz(float mel)
    {
        return 700 * (pow(10, mel / 2595) - 1);
    }

    std::vector<std::vector<float>> applyMelFilterbank(const std::vector<std::vector<float>> &powSpec)
    {
        int numFrames = powSpec.size();
        std::vector<std::vector<float>> filteredSpec(numFrames, std::vector<float>(nfilt));

        for (int i = 0; i < numFrames; ++i)
        {
            for (int j = 0; j < nfilt; ++j)
            {
                float sum = 0.0f;
                for (int k = 0; k <= nfft / 2; ++k)
                {
                    sum += powSpec[i][k] * melFilterbank[j][k];
                }
                filteredSpec[i][j] = sum;
            }
        }

        return filteredSpec;
    }

    std::vector<std::vector<float>> computeDCTMatrix()
    {
        std::vector<std::vector<float>> dctMatrix(numCeps, std::vector<float>(nfilt));

        for (int i = 0; i < numCeps; ++i)
        {
            for (int j = 0; j < nfilt; ++j)
            {
                dctMatrix[i][j] = sqrt(2.0f / nfilt) * cos(M_PI * i * (2 * j + 1) / (2 * nfilt));
            }
        }

        return dctMatrix;
    }

    std::vector<std::vector<float>> computeMFCC(const std::vector<std::vector<float>> &filteredSpec)
    {
        int numFrames = filteredSpec.size();
        std::vector<std::vector<float>> mfccFeatures(numFrames, std::vector<float>(numCeps));

        for (int i = 0; i < numFrames; ++i)
        {
            std::vector<float> logSpec(nfilt);
            for (int j = 0; j < nfilt; ++j)
            {
                logSpec[j] = log(filteredSpec[i][j] + 1e-8);
            }

            for (int j = 0; j < numCeps; ++j)
            {
                float sum = 0.0f;
                for (int k = 0; k < nfilt; ++k)
                {
                    sum += logSpec[k] * dctMatrix[j][k];
                }
                mfccFeatures[i][j] = sum;
            }
        }

        if (appendEnergy)
        {
            for (int i = 0; i < numFrames; ++i)
            {
                float energy = 0.0f;
                for (int j = 0; j < nfilt; ++j)
                {
                    energy += filteredSpec[i][j];
                }
                mfccFeatures[i][0] = log(energy + 1e-8);
            }
        }

        return mfccFeatures;
    }
};

#endif // MFCC_H