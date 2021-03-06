//Board is
// V1 from https://github.com/lewisxhe/TTGO-LoRa-Series (note pinouts and special pin assignments)
//https://github.com/LilyGO/TTGO-LORA32/tree/LilyGO-868-V1.0

// Notes:
// BLE WiFi cannot run concurrently using default partitioning scheme as it is too big
// Requires: ThingPulse SSD1306 library

#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "WiFi.h"
#include "SSD1306.h"

//#include "images.h"

#define SCK     5    // GPIO5  -- SCK
#define MISO    19   // GPIO19 -- MISO
#define MOSI    27   // GPIO27 -- MOSI
#define SS      18   // GPIO18 -- CS
#define RST     23   // GPIO14 -- RESET (If Lora does not work, replace it with GPIO14)
#define DI0     26   // GPIO26 -- IRQ(Interrupt Request)
#define BAND    915E6

#define WAIT_ACK true
bool disp_on = true;
bool transmit_on = true;

RTC_DATA_ATTR int bootCount = 0;

unsigned int counter = 0;

SSD1306 display(0x3c, 4, 15);
String rssi = "RSSI --";
String packSize = "--";
String packet ;

long t0;
long t_loop;
long dropped = 0;

int PERIOD_uS = 20000;
int loopd_us = 20000;

void setup() {
  bootCount++;
  pinMode(16, OUTPUT);
  pinMode(2, OUTPUT);

  digitalWrite(16, LOW);    // set GPIO16 low to reset OLED
  delay(150);
  //digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 in high

  Serial.begin(115200);
  while (!Serial);

  Serial.print("boot # ");
  Serial.println(bootCount);

  Serial.println("sleep for 1 second");
  esp_sleep_enable_timer_wakeup(1 * 1e6);
  delay(1000);
  esp_light_sleep_start();


  Serial.println("Idling for 30 seconds");
  delay(30000);

  //Serial.println("Starting BLE");
  //btStart();
  //BLEDevice::init("ESP32");
  //delay(10000);
  //Serial.println("Stopping BLE");
  //btStop();

  Serial.println("sleep for 1 second");
  esp_sleep_enable_timer_wakeup(1 * 1e6);
  delay(1000);
  esp_light_sleep_start();

  Serial.println("Starting WiFi...");
  WiFi.begin("jsbach", "edap00p00");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  delay(30000);
  Serial.println("WiFi off...");
  WiFi.mode(WIFI_OFF);


  Serial.println("sleep for 1 second");
  esp_sleep_enable_timer_wakeup(1 * 1e6);
  delay(1000);
  esp_light_sleep_start();

  Serial.println("Idling for 30 seconds");
  delay(30000);

  Serial.println("Going to sleep for 5 seconds...");
  esp_sleep_enable_timer_wakeup(5 * 1e6);
  delay(1000);
  esp_deep_sleep_start(); //this will cause rerun of setup after waking
}

void loop() {
  Serial.println(".");
  delay(1000);
}
