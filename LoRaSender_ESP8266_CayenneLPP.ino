//Board: WeMos D1 Mini ESP8266

#include "Arduino.h"

#include <PCF8574.h>  // https://github.com/xreef/PCF8574_library

#include <SPI.h>
#include <LoRa.h> // https://github.com/sandeepmistry/arduino-LoRa
#include <Wire.h>
#include <CayenneLPP.h> //https://github.com/ElectronicCats/CayenneLPP

// SDA/SCL pins for I2C
#define CUSTOM_SDA D4
#define CUSTOM_SCL D3

//433E6 for Asia
//866E6 for Europe
//915E6 for North America
#define BAND 866E6

//define the pins used by the LoRa transceiver module
#define SS D8

#define DIO0 D1
#define DIO1 D2

//RST pins is only in use for lora p2p library.
//replacing it with RX to keep deep sleep function
//#define RST D0 // D0 works perfectly
#define RST RX

//sleep time in microseconds
#define SLEEP_TIME 300e6

// Set i2c address
PCF8574 pcf8574(0x20, CUSTOM_SDA, CUSTOM_SCL);
CayenneLPP lpp(100);

void setup() {

  //configure RX pine to output pin
  pinMode(RX, FUNCTION_3);
  pinMode(RX, OUTPUT);

  Serial.begin(115200);
  while (!Serial);

  Serial.println("LoRa Sender CayenneLPP");

  initLoRa();

  initPcf8574();

  readLevelsAndSend();
  
  deepSleep();
}

void initPcf8574() {
  //Set 4 pins for input to measeure water level
  for (int i = 0; i <= 3; ++i) {
    pcf8574.pinMode(i, INPUT);
  }

  //Set 4 pins for output to control relays (To be implemented in next version)
  for (int i = 4; i <= 7; ++i) {
    pcf8574.pinMode(i, OUTPUT);
  }
  pcf8574.begin();
}

void initLoRa() {
  SPI.begin();
  //setup LoRa transceiver module
  LoRa.setPins(SS, RST, DIO0);

  if (!LoRa.begin(BAND)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }

  Serial.println("LoRa Initializing OK!");
}

void readLevelsAndSend() {
  PCF8574::DigitalInput di = pcf8574.digitalReadAll();
  int lowLevelValue = di.p2;
  int midLevelValue = di.p1;
  int highLevelValue = di.p0;
  
  Serial.print("Sending packet: ");
  Serial.print("low:"); Serial.print(lowLevelValue);
  Serial.print(", mid:"); Serial.print(midLevelValue);
  Serial.print(", high:"); Serial.println(highLevelValue);

  lpp.reset();
  lpp.addDigitalInput(1, lowLevelValue);
  lpp.addDigitalInput(2, midLevelValue);
  lpp.addDigitalInput(3, highLevelValue);
  
  // send packet
  LoRa.beginPacket();
  LoRa.write(lpp.getBuffer(), lpp.getSize());

  LoRa.endPacket();
}

void deepSleep() {
  // https://randomnerdtutorials.com/esp8266-deep-sleep-with-arduino-ide/
  Serial.println("Sleeping...");
  ESP.deepSleep(SLEEP_TIME);
}

void loop() {
  //NOOP
}
