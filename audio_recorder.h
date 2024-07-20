#ifndef AUDIO_RECORDER_H
#define AUDIO_RECORDER_H

#include <SDHCI.h>
#include <Audio.h>
#include <arch/board/board.h>

#define RECORD_FILE_NAME "compare.wav"

class AudioRecorder {
private:
  SDClass theSD;
  AudioClass *theAudio;

  File myFile;

  static bool ErrEnd;

  static void audio_attention_cb(const ErrorAttentionParam *atprm) {
    puts("Attention!");

    if (atprm->error_code >= AS_ATTENTION_CODE_WARNING) {
      ErrEnd = true;
    }
  }

  const uint32_t recoding_sampling_rate = 48000;
  const uint8_t recoding_cannel_number = 1;
  const uint8_t recoding_bit_length = 16;
  const uint32_t recoding_time = 5;
  const int32_t recoding_byte_per_second = recoding_sampling_rate * recoding_cannel_number * recoding_bit_length / 8;
  const int32_t recoding_size = recoding_byte_per_second * recoding_time;
  /* Volume gain in decibels */
  const int32_t volume_gain_db = 210; // Adjust gain as needed

public:
  void wav_recorder() {
    Serial.begin(115200);

    while (!theSD.begin()) {
      Serial.println("Insert SD card.");
      delay(1000);
    }

    theAudio = AudioClass::getInstance();
    theAudio->begin(audio_attention_cb);
    puts("Initialization of Audio Library");
    theAudio->setRecorderMode(AS_SETRECDR_STS_INPUTDEVICE_MIC, volume_gain_db);
  

    theAudio->initRecorder(AS_CODECTYPE_WAV, "/mnt/sd0/BIN", recoding_sampling_rate, recoding_bit_length, recoding_cannel_number);
    puts("Recorder Initialized!");

    if (theSD.exists(RECORD_FILE_NAME)) {
      printf("Removing existing file [%s].\n", RECORD_FILE_NAME);
      theSD.remove(RECORD_FILE_NAME);
    }

    myFile = theSD.open(RECORD_FILE_NAME, FILE_WRITE);
    if (!myFile) {
      printf("File open error\n");
      exit(1);
    }

    printf("File opened! [%s]\n", RECORD_FILE_NAME);
    theAudio->writeWavHeader(myFile);
    puts("WAV Header written!");
    theAudio->startRecorder();
    puts("Recording started!");

    while (true) {
      err_t err;

      if (theAudio->getRecordingSize() > recoding_size) {
        theAudio->stopRecorder();
        delay(1000);
        err = theAudio->readFrames(myFile);
        break;
      }

      err = theAudio->readFrames(myFile);

      if (err != AUDIOLIB_ECODE_OK) {
        printf("Recording Error! Code = %d\n", err);
        theAudio->stopRecorder();
        break;
      }

      if (ErrEnd) {
        printf("Error detected, stopping recording.\n");
        theAudio->stopRecorder();
        break;
      }

      delay(100);
    }

    theAudio->closeOutputFile(myFile);
    myFile.close();
    theAudio->setReadyMode();
    theAudio->end();
    puts("Recording ended.");
  }
};

bool AudioRecorder::ErrEnd = false;

#endif // AUDIO_RECORDER_H
