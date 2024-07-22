#include "audio_recorder.h"
#include <SDHCI.h>
#include <Audio.h>
#include <MP.h>


#define BUTTON_PIN 23    // Define the pin D3 to recive the button output
#define RELAY_PIN 27     // Define the pin D27 to connect the relay
#define DELAY_TIME 3000  // Defines the duration of the unlock in milliseconds

AudioRecorder recorder;
SDClass theSD;

void unlock() {
  digitalWrite(RELAY_PIN, HIGH);  // Output high level unlock
  Serial.println("Unlocked");
}

void relock() {
  digitalWrite(RELAY_PIN, LOW);  // Output low level lock-up
  Serial.println("Relocked");
}


void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
}

void loop() {
  while(buttonState == LOW){
    Serial.println("Press the button to open the program");
    delay(1000);
  }

  int buttonState = digitalRead(BUTTON_PIN);
  if (buttonState == HIGH) {
    Serial.println("Audio recording start!");
    recorder.wav_recorder();
    

  }
  
}
