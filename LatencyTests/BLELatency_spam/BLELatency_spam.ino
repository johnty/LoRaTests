/*
   ESP32 BLE MIDI Trigger Test

   Constantly send note at fixed intervals
   v = 127 if high, 0 if low

   AUTO_CHANGE will cause the send rate
   to cycle through list defined in send_intervals[]
   for each test_interval_ms, otherwise it will be whatever
   send_interval is set to initially upon start

   Board: V1 from https://github.com/lewisxhe/TTGO-LoRa-Series (note pinouts and special pin assignments)
   https://github.com/LilyGO/TTGO-LORA32/tree/LilyGO-868-V1.0

   Jan 2021
   johnty.wang@mail.mcgill.ca

*/

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define D0 34 //TTGO
//#define D0 4 //D32 PRO
bool armed = true;

static const uint8_t led = 5;

BLECharacteristic *pCharacteristic;
bool deviceConnected = false;

uint16_t time_ble_ts_0; //for BLE timestamping
long time_1; //for send rate control
long looptimer_0;

#define AUTO_CHANGE false


//                 in Hz:     10     20     25     50     75     80    100,  200   400  inf
int send_intervals[10] = {100000, 50000, 40000, 20000, 13333, 12500, 10000, 5000, 2500, 0};
int send_interval = 10000; //default 10hz
int numIntervals = 10;

int test_interval_ms = 30000;
int currMode = -1;

#define MIDI_SERVICE_UUID        "03b80e5a-ede8-4b33-a751-6ce34ec4c700"
#define MIDI_CHARACTERISTIC_UUID "7772e5db-3868-4112-a1a9-f2669d106bf3"

uint8_t midiPacket[] = {
  0x80,  // header
  0x80,  // timestamp, to be filled
  0x00,  // status
  0x3c,  // 0x3c == 60 == middle c
  0x00   // velocity
};

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      digitalWrite(led, HIGH);
      send_interval = 10000;
      Serial.print("SI = ");
      Serial.println(send_interval);
      currMode = 0;
      looptimer_0 = millis();
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      digitalWrite(led, LOW);
    };

    void onCongestion(BLEServer* pServer) {
      Serial.println("CONGESTION!!!");
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        Serial.println("*********");
        Serial.print("Received Value: ");
        for (int i = 0; i < rxValue.length(); i++)
        { Serial.print(rxValue[i], HEX);
          Serial.print(" ");
        }
        Serial.println();

        Serial.print("len = ");
        Serial.println(rxValue.length());
        Serial.println("*********");
      }
    }
};

void setup() {
  pinMode(led, OUTPUT);
  digitalWrite(led, HIGH);
  delay(500);
  digitalWrite(led, LOW);
  Serial.begin(115200);

  pinMode(D0, INPUT);

  midiPacket[3] = 0x3c; // middle C
  midiPacket[2] = 0x90; // note down, channel 0

  BLEDevice::init("esp32");

  //BLEDevice::setMTU(25);
  // Create the BLE Server
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  //BLEDevice::setEncryptionLevel((esp_ble_sec_act_t)ESP_LE_AUTH_REQ_SC_BOND);

  // Create the BLE Service
  BLEService *pService = pServer->createService(MIDI_SERVICE_UUID);

  // Create a BLE Characteristic
  pCharacteristic = pService->createCharacteristic(
                      BLEUUID(MIDI_CHARACTERISTIC_UUID),
                      BLECharacteristic::PROPERTY_READ   |
                      BLECharacteristic::PROPERTY_WRITE  |
                      BLECharacteristic::PROPERTY_NOTIFY |
                      BLECharacteristic::PROPERTY_WRITE_NR
                    );
  pCharacteristic->setAccessPermissions(ESP_GATT_PERM_READ_ENCRYPTED | ESP_GATT_PERM_WRITE_ENCRYPTED);

  // https://www.bluetooth.com/specifications/gatt/viewer?attributeXmlFile=org.bluetooth.descriptor.gatt.client_characteristic_configuration.xml
  // Create a BLE Descriptor
  pCharacteristic->addDescriptor(new BLE2902());

  //connect rx callback
  pCharacteristic->setCallbacks(new MyCallbacks());

  // Start the service
  pService->start();

  //set conn intervals
  pServer->getAdvertising()->setMinPreferred(0x06); //6x1.25 = 7.5 ms
  pServer->getAdvertising()->setMaxPreferred(0x0C); //12x1.25 = 15 ms (Apple BLE MIDI spec)


  //  BLESecurity *pSecurity = new BLESecurity();
  //  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_BOND);
  //  pSecurity->setCapability(ESP_IO_CAP_NONE);
  //  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  pServer->getAdvertising()->addServiceUUID(MIDI_SERVICE_UUID);

  // Start advertising
  pServer->getAdvertising()->start();

  for (int i = 0; i < 5; i++) {
    digitalWrite(led, HIGH);
    delay(100);
    digitalWrite(led, LOW);
    delay(100);
  }

  time_ble_ts_0 = millis();
  looptimer_0 = time_1 = time_ble_ts_0;
}

void loop() {
  if (Serial.available())
  {
    char ch = Serial.read();
    int idx = '9' - ch; //'0'-'9' is ASCII 48-57
    if (idx < numIntervals && idx >= 0) { //ignore carriage return
      send_interval = send_intervals[idx];
    }
    //Serial.print("SI = ");
    //Serial.println(send_interval);
  }

  if (deviceConnected)
  {
    if (AUTO_CHANGE)
    {
      updateInterval();
    }
    int time_elapsed = millis() - time_ble_ts_0;
    if (time_elapsed > 8191)
      time_ble_ts_0 = millis();
    //timetamp high = 10HH HHHH
    byte th = B10000000;
    th = time_elapsed >> 7;
    midiPacket[0] = B10000000 | th;
    //timestamp low = 1LLL LLLL
    byte tl = B01111111 & time_elapsed;
    midiPacket[1] = B10000000 | tl;

    long tn = micros();
    int diff = tn - time_1;
    if (diff >= send_interval)
    {
      armed = true;
    }
    if (armed)
    {
      if (digitalRead(D0) == LOW)
      {
        midiPacket[4] = 0;  // velocity 0        
      }

      if (digitalRead(D0) == HIGH)
      {
        midiPacket[4] = 127;  // velocity
      }
      pCharacteristic->setValue(midiPacket, 5); // packet, length in bytes
      pCharacteristic->notify();
      armed = false;
      time_1 = micros();
    }
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
