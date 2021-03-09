// V1 from https://github.com/lewisxhe/TTGO-LoRa-Series (note pinouts and special pin assignments)
//https://github.com/LilyGO/TTGO-LORA32/tree/LilyGO-868-V1.0

#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <OSCMessage.h>
#include "esp_wifi.h"

//#include "AppleMidi.h"


#define SOFTAP 0

//#define D0 21 //adafruit
#define D0 34 //TTGO

WiFiUDP udp;

char ssid[] = "LEDE"; //  your network SSID (name)
char pass[] = "";    // your network password (use for WPA, or use as key for WEP)

//IPAddress local_IP(192,168,88,88);
//IPAddress gateway(192,168,88,81);
//IPAddress subnet(255,255,0,0);

IPAddress destIP(192, 168, 2, 100); //
const unsigned int destPort = 7000;

long looptimer_0;
long t0 = millis();
bool isConnected = false;


// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  esp_wifi_set_ps(WIFI_PS_NONE);
  // Serial communications and wait for port to open:
  //oscMsg.add((int32_t)1);
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  pinMode(D0, INPUT);

  //static:
  //  if (!WiFi.config(local_IP, gateway, subnet)) {
  //    Serial.println("STA failed to configure!");
  //  }

  Serial.print(F("Connecting to WiFi..."));
  if (!SOFTAP) {
    WiFi.begin(ssid, pass);
  }
  else {
    WiFi.softAP(ssid, pass);
    IPAddress IP = WiFi.softAPIP();
    Serial.print("AP IP is: ");
    Serial.println(IP);
  }

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F(""));
  Serial.println(F("WiFi connected"));
  isConnected = true;

  Serial.println();
  Serial.print(F("IP address is "));
  if (SOFTAP) Serial.println(WiFi.softAPIP());
  else Serial.println(WiFi.localIP());

  //Serial.println("doing nothing for 30 seconds....");
  //delay(30*1000);


  t0 = micros();
  looptimer_0 = millis();

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool armed = false;
bool valChanged;

int send_interval = 100000;
//in Hz: 0       10     20     25     50     75     80    100,  150   200
int send_intervals[10] = {999999, 100000, 50000, 40000, 20000, 13333, 12500, 10000, 6667, 5000};
const int numIntervals = 10;
const int test_interval_ms = 10000; //duration to run each send rate for, in ms
int currMode = 0;


//~2300:      0
//1000Hz:   960
//1500Hz:   570
//500Hz:   2000
//200Hz:   5000
//100Hz:  10000
//50Hz:   20000;
//10Hz:  100000;
int preVal = 0;
int val;

void loop()
{
  //do each mode for 10 seconds
  if (millis() - looptimer_0 > test_interval_ms)
  {
    Serial.println(".");
    if (currMode == 0) { //time sleep for 5 seconds to sync power data
      esp_sleep_enable_timer_wakeup(5 * 1e6);
      delay(100);
      esp_light_sleep_start();
    }
    else
    {
      send_interval = send_intervals[currMode];
      Serial.print("SI = ");
      Serial.println(send_interval);
    }
    currMode++;
    if (currMode >= numIntervals)
    {
      currMode = 0;
    }
    //reset loop timer for next round
    looptimer_0 = millis();
  }

  armed = false;

  //  if (val != preVal)
  //    valChanged = true;
  //  else
  //    valChanged = false;

  long t1 = micros();
  int diff = t1 - t0;
  if ( (diff >= send_interval) ) //|| (valChanged && (val == 1)) )
    armed = true;

  if (isConnected && armed) {
    OSCMessage oscMsg("/a");
    udp.beginPacket(destIP, destPort);
    val = digitalRead(D0);
    oscMsg.add(val);
    oscMsg.send(udp);

    udp.endPacket();
    //Serial.println(".");
    t0 = micros();
  }
  preVal = val;
}
