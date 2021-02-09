/*
   ESP32 BLE MIDI Trigger Test

   Generates a BLE MIDI note event when a digital pin is toggled.

   Board: V1 from https://github.com/lewisxhe/TTGO-LoRa-Series (note pinouts and special pin assignments)
   https://github.com/LilyGO/TTGO-LORA32/tree/LilyGO-868-V1.0

   Arduino BLE MIDI version

   Jan 2021
   johnty.wang@mail.mcgill.ca

*/

#include <BLEMIDI_Transport.h>

#include <hardware/BLEMIDI_ESP32.h>

BLEMIDI_CREATE_DEFAULT_INSTANCE()

#define D0 34 //TTGO

bool armed = true;
bool isConnected = false;

static const uint8_t led = 5;
//unsigned long int time_0;


void OnConnected() {
  isConnected = true;
  digitalWrite(led, HIGH);
}

void OnDisconnected() {
  isConnected = false;
  digitalWrite(led, LOW);
}

void setup() {
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  delay(500);
  digitalWrite(led, LOW);
  Serial.begin(115200);

  pinMode(D0, INPUT);

  MIDI.begin();

  BLEMIDI.setHandleConnected(OnConnected);
  BLEMIDI.setHandleDisconnected(OnDisconnected);

  for (int i = 0; i < 5; i++) {
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(100);
  }
}

void loop() {

  if (isConnected) {

    //time_elapsed = millis() - time_0;
    
    if (digitalRead(D0) == LOW && !armed) {
      armed = true;
      //AppleMIDI.noteOff(84, 0, 1);
    }

    if (digitalRead(D0) == HIGH && armed) {
      //AppleMIDI.noteOn(84, 127, 1);
      MIDI.sendNoteOn (60, 100, 1);
      armed = false;
      Serial.println(".");
    }
    //delay(delay_ms);

    // note up
    /*
        midiPacket[2] = 0x80; // note up, channel 0
        midiPacket[4] = 0;    // velocity
        pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes)
        pCharacteristic->notify();


        delay(delay_ms);
    */

  }
}
