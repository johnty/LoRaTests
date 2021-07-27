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

#define WAIT_ACK false
bool transmit_on = true;

RTC_DATA_ATTR int bootCount = 0;
//                 in Hz:     10     20     25     50     75     80    100,  200   400  inf
int send_intervals[10] = {100000, 50000, 40000, 20000, 13333, 12500, 10000, 5000, 2500, 0};
int send_interval = 100000; //default 10hz
int numIntervals = 10;

int test_interval_ms = 30000;
int currMode = -1;

long looptimer_0;

#define AUTO_CHANGE true

unsigned int counter = 0;
long sendrate_t0;

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
  
  sendrate_t0 = micros();
  looptimer_0 = millis();
}

bool valChanged = false;
int preVal = -1;
bool armed = false;



void loop() {

   if (AUTO_CHANGE)
    {
      updateInterval();
    }

  if ( (micros() - sendrate_t0) > send_interval ) {
    armed = true;
    sendrate_t0 = micros();
  }

  if (armed) {
    LoRa.beginPacket();
    if (digitalRead(D0) == LOW )
      LoRa.print("0");
    else
      LoRa.print("1");
    LoRa.endPacket();
    armed = false;
  }
}

void updateInterval()
{
  if (millis() - looptimer_0 > test_interval_ms)
  {
    send_interval = send_intervals[currMode];
    Serial.print("SI = ");
    Serial.println(send_interval);
    currMode++;
    if (currMode >= numIntervals)
    {
      currMode = 0;
    }
    //reset loop timer for next round
    looptimer_0 = millis();
  }
}
