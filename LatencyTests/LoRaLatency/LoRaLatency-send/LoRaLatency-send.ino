/*
 Board is
 V1 from https://github.com/lewisxhe/TTGO-LoRa-Series (note pinouts and special pin assignments)
         https://github.com/LilyGO/TTGO-LORA32/tree/LilyGO-868-V1.0

   receives D0 input trigger and transits via LoRa

   Jan 2021
   johnty.wang@mail.mcgill.ca

   */

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>

#define SCK     5    // GPIO5  -- SCK
#define MISO    19   // GPIO19 -- MISO
#define MOSI    27   // GPIO27 -- MOSI
#define SS      18   // GPIO18 -- CS
#define RST     23   // GPIO14 -- RESET (If Lora does not work, replace it with GPIO14)
#define DI0     26   // GPIO26 -- IRQ(Interrupt Request)
#define BAND    915E6

#define D0 34 //TTGO

#define WAIT_ACK true
bool transmit_on = true;

RTC_DATA_ATTR int bootCount = 0;

unsigned int counter = 0;

long t0;
long t_loop;
long dropped = 0;

void setup() {
  bootCount++;
  pinMode(2, OUTPUT);
  pinMode(D0, INPUT);

  Serial.begin(115200);
  while (!Serial);

  Serial.print("boot # ");
  Serial.println(bootCount);


  Serial.println("Starting LoRa..."); 
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI0);
  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  LoRa.setSignalBandwidth(500E3);
  LoRa.setSpreadingFactor(7);
  Serial.println("...ok");
}

bool valChanged = false;
int preVal = -1;
bool armed = false;

void loop() {
  armed = false;
  int val = digitalRead(D0);
  if (val != preVal)
    valChanged = true;
  else
    valChanged = false;

  if (valChanged && (val == 1))
  {
    armed = true;
    LoRa.beginPacket();
    LoRa.print("1");
    LoRa.endPacket();
    //Serial.println('.');
  }

  preVal = val;

}
