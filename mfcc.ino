#include <Arduino.h>
#include <FFT.h> // 使用Arduino的FFT库
#include <Math.h>

// 定义参数
const int SAMPLE_RATE = 16000; // 采样率
const int FRAME_SIZE = 256; // 帧大小
const int FRAME_SHIFT = 128; // 帧移位
const int MEL_FILTERS = 26; // Mel滤波器数量
const int MFCC_SIZE = 13; // MFCC系数数量

float preEmphasis = 0.97; // 预加重系数
float window[FRAME_SIZE];
float frame[FRAME_SIZE];
float fftBuffer[FRAME_SIZE];
float melFilterBank[MEL_FILTERS][FRAME_SIZE / 2 + 1];
float mfcc[MFCC_SIZE];
float previousMfcc[MFCC_SIZE];

// 初始化
void setup() {
    Serial.begin(9600);
    // 初始化Hamming窗
    for (int i = 0; i < FRAME_SIZE; i++) {
        window[i] = 0.54 - 0.46 * cos(2 * PI * i / (FRAME_SIZE - 1));
    }
    // 初始化Mel滤波器组
    initMelFilterBank();
}

void loop() {
    // 获取音频数据

    
    // (从麦克风或其他输入获取数据)
    
    // 预加重
    preEmphasisFilter();
    
    // 分帧处理
    for (int i = 0; i < FRAME_SIZE; i++) {
        frame[i] = frame[i] * window[i]; // 加窗
    }
    
    // FFT变换
    for (int i = 0; i < FRAME_SIZE; i++) {
        fftBuffer[i] = frame[i];
    }
    fft(fftBuffer, FRAME_SIZE);

    // 计算能量
    float energy[FRAME_SIZE / 2 + 1];
    for (int i = 0; i < FRAME_SIZE / 2 + 1; i++) {
        energy[i] = sqrt(fftBuffer[i] * fftBuffer[i] + fftBuffer[FRAME_SIZE - i - 1] * fftBuffer[FRAME_SIZE - i - 1]);
    }

    // Mel滤波器组滤波和对数变换
    float melEnergies[MEL_FILTERS];
    for (int i = 0; i < MEL_FILTERS; i++) {
        melEnergies[i] = 0;
        for (int j = 0; j < FRAME_SIZE / 2 + 1; j++) {
            melEnergies[i] += energy[j] * melFilterBank[i][j];
        }
        melEnergies[i] = log(melEnergies[i]);
    }

    // DCT变换
    for (int i = 0; i < MFCC_SIZE; i++) {
        mfcc[i] = 0;
        for (int j = 0; j < MEL_FILTERS; j++) {
            mfcc[i] += melEnergies[j] * cos(PI * i / MEL_FILTERS * (j + 0.5));
        }
    }

    // 差分计算
    float delta[MFCC_SIZE];
    for (int i = 0; i < MFCC_SIZE; i++) {
        delta[i] = mfcc[i] - previousMfcc[i];
        previousMfcc[i] = mfcc[i];
    }

    // 输出MFCC系数和差分系数
    for (int i = 0; i < MFCC_SIZE; i++) {
        Serial.print("MFCC[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(mfcc[i]);
        Serial.print("Delta[");
        Serial.print(i);
        Serial.print("]: ");
        Serial.println(delta[i]);
    }

    // 等待一段时间再进行下一个帧的处理
    delay(100);
}

void preEmphasisFilter() {
    for (int i = FRAME_SIZE - 1; i > 0; i--) {
        frame[i] = frame[i] - preEmphasis * frame[i - 1];
    }
    frame[0] = frame[0] - preEmphasis * frame[0];
}

void initMelFilterBank() {
    // 初始化Mel滤波器组系数
    float melMin = 0;
    float melMax = 2595 * log10(1 + (SAMPLE_RATE / 2) / 700.0);
    float melPoints[MEL_FILTERS + 2];
    for (int i = 0; i < MEL_FILTERS + 2; i++) {
        melPoints[i] = melMin + i * (melMax - melMin) / (MEL_FILTERS + 1);
    }
    float freqPoints[MEL_FILTERS + 2];
    for (int i = 0; i < MEL_FILTERS + 2; i++) {
        freqPoints[i] = 700 * (pow(10, melPoints[i] / 2595) - 1);
    }
    int binPoints[MEL_FILTERS + 2];
    for (int i = 0; i < MEL_FILTERS + 2; i++) {
        binPoints[i] = floor((FRAME_SIZE + 1) * freqPoints[i] / SAMPLE_RATE);
    }
    for (int i = 0; i < MEL_FILTERS; i++) {
        for (int j = 0; j < FRAME_SIZE / 2 + 1; j++) {
            if (j < binPoints[i]) {
                melFilterBank[i][j] = 0;
            } else if (j >= binPoints[i] && j <= binPoints[i + 1]) {
                melFilterBank[i][j] = (j - binPoints[i]) / (binPoints[i + 1] - binPoints[i]);
            } else if (j > binPoints[i + 1] && j <= binPoints[i + 2]) {
                melFilterBank[i][j] = (binPoints[i + 2] - j) / (binPoints[i + 2] - binPoints[i + 1]);
            } else {
                melFilterBank[i][j] = 0;
            }
        }
    }
}
