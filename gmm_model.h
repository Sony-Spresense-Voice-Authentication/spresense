#ifndef GMM_MODEL_H
#define GMM_MODEL_H

#include <Arduino.h>
#include <vector>

class GMM {
public:
    GMM();
    bool loadModel(const char* modelPath);
    int predict(const float* mfccData, size_t mfccSize);

private:
    struct Gaussian {
        std::vector<float> mean;
        std::vector<float> covariance;
        float weight;
    };

    std::vector<Gaussian> gaussians;
    size_t numGaussians;
    size_t numDimensions;

    float calculateProbability(const Gaussian& gaussian, const float* data, size_t dataSize);
};

#endif // GMM_MODEL_H

