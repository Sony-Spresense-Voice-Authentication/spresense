#define SS 4
#define MOSI 11
#define MISO 12
#define SCK 13

#include <Arduino.h>
#include <SD.h>
#include "record_audio.h"
#include "extract_pcm.h"
#include "calculate_mfcc.h"
#include "gmm_model.h"

// 实例化录音、PCM提取、MFCC计算和GMM模型类
AudioRecorder recorder;
PCMExtractor pcmExtractor;
MFCC mfcc;
GMM gmm;

void setup() {
    Serial.begin(115200);
    while (!Serial);

    if (!SD.begin(SS)) {
        Serial.println("SD card initialization failed!");
        return;
    }

    recorder.begin();
    mfcc.begin(16000, 13);

    // 加载GMM模型
    if (!gmm.loadModel("/gmm_model.bin")) {
        Serial.println("Failed to load GMM model");
        return;
    }

    // 录音10秒
    Serial.println("Starting recording...");
    recorder.startRecording("/sound.wav");
    delay(10000); // 录音10秒
    recorder.stopRecording();
    Serial.println("Recording stopped.");

    // 打开WAV文件并提取PCM数据
    if (!pcmExtractor.openWavFile("/sound.wav")) {
        Serial.println("Failed to open WAV file");
        return;
    }

    const size_t bufferSize = 16000 * 10; // 10 seconds buffer
    int16_t pcmData[bufferSize];
    size_t samplesRead = pcmExtractor.extractPCMData(pcmData, bufferSize);
    Serial.print("Read samples: ");
    Serial.println(samplesRead);

    pcmExtractor.closeWavFile();

    // 计算MFCC特征
    const size_t mfccSize = 13;
    float mfccData[mfccSize];
    mfcc.compute(pcmData, samplesRead, mfccData, mfccSize);

    // 使用GMM模型进行预测
    int prediction = gmm.predict(mfccData, mfccSize);
    Serial.print("Predicted Gaussian: ");
    Serial.println(prediction);

    mfcc.end();
}

void loop() {
    // Main loop does nothing in this example
}
