#include "audio_recorder.h"
#include <SDHCI.h>
#include <Audio.h>
#include <MP.h>


#define BUTTON_PIN 23  // Define the pin D3 to recive the button output

#define LED_TRUE_PIN 25   // Define the pin D25 to indicate valid state
#define LED_FALSE_PIN 26  //Define the pin D26 to indicate invalid state

#define RPI_START_PIN 2  // Define the pin D02 to send start signal to Raspberry Pi
#define RPI_TRUE_PIN 3   // Define the pin D03 to receive true signal from Raspberry Pi
#define RPI_FALSE_PIN 4  // Define the pin D04 to receive false signal from Raspberry Pi

#define TIMEOUT 30000  // 30 seconds timeout

AudioRecorder recorder;
SDClass theSD;

void setup() {
  Serial.begin(115200);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  pinMode(LED_TRUE_PIN, OUTPUT);
  digitalWrite(LED_TRUE_PIN, LOW);
  pinMode(LED_FALSE_PIN, OUTPUT);
  digitalWrite(LED_FALSE_PIN, LOW);


  pinMode(RPI_TRUE_PIN, INPUT_PULLUP);
  pinMode(RPI_FALSE_PIN, INPUT_PULLUP);

  while (!theSD.begin()) {
    Serial.println("SD card initialization failed. Retrying...");
    delay(1000);  // wait until SD card is mounted.
  }
  Serial.println("SD card initialized successfully.");

  Serial.println("Setup complete.");
}

void loop() {

  Serial.println("Waiting for button press.");
  while (digitalRead(BUTTON_PIN) != HIGH) {
    delay(100);  // Debouncing delay
  }

  recorder.wav_recorder();

  // Start USB Mass Storage
  theSD.beginUsbMsc();
  Serial.println("UsbMsc started successfully.");
  delay(3000);  //Waiting for execution

  Serial.println("Waiting for response from Raspberry Pi...");
  unsigned long startTime = millis();
  while (true) {
    int validState = digitalRead(RPI_TRUE_PIN);
    int invalidState = digitalRead(RPI_FALSE_PIN);

    if (validState == HIGH) {
      Serial.println("Valid user.");
      digitalWrite(LED_TRUE_PIN, HIGH);
      delay(5000);
      digitalWrite(LED_TRUE_PIN, LOW);
      break;
    } else if (invalidState == HIGH) {
      Serial.println("Invalid user.");
      for (int i = 0; i < 6; i++) {
        digitalWrite(LED_FALSE_PIN, HIGH);
        delay(400);
        digitalWrite(LED_FALSE_PIN, LOW);
        delay(400);
      }
      break;
    }

    if (millis() - startTime >= TIMEOUT) {
      Serial.println("Timeout: No response from Raspberry Pi.");
      break;
    }

    delay(100);
  }

  theSD.endUsbMsc();
  Serial.println("UsbMsc stopped successfully.");
}
