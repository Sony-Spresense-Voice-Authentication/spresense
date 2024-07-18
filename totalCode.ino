#include <SDHCI.h>
#include <Audio.h>
#include <arch/board/board.h>
#include <arduinoFFT.h>

#define RECORD_FILE_NAME "/mnt/sd0/sound.wav"
#define MODEL_PARAMS_PATH "/mnt/sd0/models_parameters/"

SDClass theSD;
AudioClass *theAudio;

File myFile;
bool ErrEnd = false;

/**
 * @brief Audio attention callback
 *
 * When an audio internal error occurs, this function will be called back.
 */
static void audio_attention_cb(const ErrorAttentionParam *atprm)
{
  puts("Attention!");

  if (atprm->error_code >= AS_ATTENTION_CODE_WARNING)
  {
    ErrEnd = true;
  }
}

/* Sampling rate: Set 16000 or 48000 */
static const uint32_t recoding_sampling_rate = 48000;
/* Number of input channels: Set either 1, 2, or 4 */
static const uint8_t recoding_cannel_number = 2;
/* Audio bit depth: Set 16 or 24 */
static const uint8_t recoding_bit_length = 16;
/* Recording time [seconds] */
static const uint32_t recoding_time = 10;
/* Bytes per second */
static const int32_t recoding_byte_per_second = recoding_sampling_rate * recoding_cannel_number * recoding_bit_length / 8;
/* Total recording size */
static const int32_t recoding_size = recoding_byte_per_second * recoding_time;

/* MFCC calculation parameters */
const uint16_t SAMPLE_WINDOW = 64;
const double samplingFrequency = 48000;
ArduinoFFT<double> FFT = ArduinoFFT<double>();
double vReal[SAMPLE_WINDOW];
double vImag[SAMPLE_WINDOW];

void setup()
{
  Serial.begin(9600);
  Serial.println("Starting setup...");

  /* Initialize SD */
  while (!theSD.begin())
  {
    /* wait until SD card is mounted. */
    Serial.println("Insert SD card.");
    delay(1000);
  }
  Serial.println("SD card initialized.");

  theAudio = AudioClass::getInstance();

  theAudio->begin(audio_attention_cb);

  Serial.println("Initialization of Audio Library");

  /* Select input device as microphone */
  theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC);

  /* Search for WAVDEC codec in "/mnt/sd0/BIN" directory */
  theAudio->initRecorder(AS_CODECTYPE_WAV, "/mnt/sd0/BIN", recoding_sampling_rate, recoding_bit_length, recoding_cannel_number);
  Serial.println("Recorder Initialized!");

  /* Open file for data write on SD card */
  if (theSD.exists(RECORD_FILE_NAME))
  {
    Serial.printf("Removing existing file [%s].\n", RECORD_FILE_NAME);
    theSD.remove(RECORD_FILE_NAME);
  }

  Serial.printf("Attempting to open file: %s\n", RECORD_FILE_NAME);
  myFile = theSD.open(RECORD_FILE_NAME, FILE_WRITE);
  /* Verify file open */
  if (!myFile)
  {
    Serial.println("File open error. Possible reasons:");
    Serial.println("1. SD card not properly mounted.");
    Serial.println("2. Incorrect file path.");
    Serial.println("3. File system error.");
    return;
  }

  Serial.printf("File opened! [%s]\n", RECORD_FILE_NAME);

  theAudio->writeWavHeader(myFile);
  Serial.println("WAV Header written!");

  theAudio->startRecorder();
  Serial.println("Recording started!");
}

void loop()
{
  err_t err;

  /* Recording end condition */
  if (theAudio->getRecordingSize() > recoding_size)
  {
    Serial.println("Recording size limit reached.");
    theAudio->stopRecorder();
    delay(1000);  // Let the recorder settle
    err = theAudio->readFrames(myFile);
    goto exitRecording;
  }

  /* Read frames to record in file */
  err = theAudio->readFrames(myFile);

  if (err != AUDIOLIB_ECODE_OK)
  {
    Serial.printf("Recording Error! Code = %d\n", err);
    theAudio->stopRecorder();
    goto exitRecording;
  }

  if (ErrEnd)
  {
    Serial.println("Error detected, stopping recording.");
    theAudio->stopRecorder();
    goto exitRecording;
  }

  delay(100);  // Adjust this delay to match your system's performance

  return;

exitRecording:
  theAudio->closeOutputFile(myFile);
  myFile.close();

  theAudio->setReadyMode();
  theAudio->end();

  Serial.println("Recording ended.");

  // Read and process the WAV file
  readWavFile(RECORD_FILE_NAME);

  exit(1);
}

class Wav {
public:
  void parseHeader(File& file) {
    Serial.println("Parsing WAV header...");
    file.seek(22);
    numChannels = read16(file);
    Serial.printf("Number of channels: %d\n", numChannels);
    file.seek(24);
    sampleRate = read32(file);
    Serial.printf("Sample rate: %d\n", sampleRate);
    file.seek(34);
    bitsPerSample = read16(file);
    Serial.printf("Bits per sample: %d\n", bitsPerSample);
    file.seek(40);
    dataSize = read32(file);
    Serial.printf("Data size: %d\n", dataSize);
  }

  uint16_t numChannels;
  uint32_t sampleRate;
  uint16_t bitsPerSample;
  uint32_t dataSize;

private:
  uint16_t read16(File& file) {
    uint16_t value;
    file.read((uint8_t*)&value, 2);
    return value;
  }

  uint32_t read32(File& file) {
    uint32_t value;
    file.read((uint8_t*)&value, 4);
    return value;
  }
};

void readWavFile(const char* filename) {
  Serial.printf("Reading WAV file: %s\n", filename);
  File file = theSD.open(filename);
  if (!file) {
    Serial.println("Failed to open WAV file.");
    return;
  }

  Wav wav;
  wav.parseHeader(file);

  uint16_t buffer[SAMPLE_WINDOW];
  while (file.available()) {
    int bytesRead = file.read(buffer, sizeof(buffer));
    processPcmData(buffer, bytesRead / 2);
  }

  file.close();
}

void processPcmData(uint16_t* buffer, int length) {
  Serial.printf("Processing PCM data, length: %d\n", length);
  for (int i = 0; i < length; i++) {
    vReal[i] = buffer[i];
    vImag[i] = 0;
  }

  FFT.windowing(vReal, length, FFT_WIN_TYP_HAMMING, FFT_FORWARD);
  FFT.compute(vReal, vImag, length, FFT_FORWARD);
  FFT.complexToMagnitude(vReal, vImag, length);

  float mfcc[13];
  calculateMFCC(vReal, length, mfcc);

  Serial.println("MFCC coefficients:");
  for (int i = 0; i < 13; i++) {
    Serial.printf("%f ", mfcc[i]);
  }
  Serial.println();

  compareWithGmm(mfcc);
}

void calculateMFCC(double* vReal, int length, float* mfcc) {
  // 简单实现，实际MFCC计算会复杂得多
  for (int i = 0; i < 13; i++) {
    mfcc[i] = vReal[i];
  }
}

void compareWithGmm(float* mfcc) {
  Serial.println("Comparing MFCC with GMM...");
  float covariances[13];
  float means[13];
  float weights[13];

  loadGmmParameters(MODEL_PARAMS_PATH "pzka_covariances.txt", covariances);
  loadGmmParameters(MODEL_PARAMS_PATH "pzka_means.txt", means);
  loadGmmParameters(MODEL_PARAMS_PATH "pzka_weights.txt", weights);

  float probability = calculateGmmProbability(mfcc, covariances, means, weights);

  Serial.print("GMM Probability: ");
  Serial.println(probability);
}

void loadGmmParameters(const char* path, float* array) {
  Serial.printf("Loading GMM parameters from: %s\n", path);
  File file = theSD.open(path);
  if (!file) {
    Serial.println("Failed to open GMM parameter file.");
    return;
  }

  int index = 0;
  while (file.available()) {
    array[index++] = file.parseFloat();
  }
  file.close();
}

float calculateGmmProbability(float* mfcc, float* covariances, float* means, float* weights) {
  float probability = 0.0;
  for (int i = 0; i < 13; i++) {
    probability += weights[i] * exp(-0.5 * ((mfcc[i] - means[i]) * (mfcc[i] - means[i]) / covariances[i]));
  }
  return probability;
}


