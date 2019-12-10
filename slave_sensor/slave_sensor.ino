#include <Wire.h>
#include <SoftwareSerial.h>
#include "DFRobot_VEML7700.h"
#include <dht11.h>

#define DHT11PIN    2           // Temperature
#define sound_din   3           // Sound
#define sound_ain   A0          // Sound


DFRobot_VEML7700 als;           // Light
dht11 DHT11;                    // Temperature
SoftwareSerial BTSerial(4,5);   // Bluetooth Slave
char buffer[100];               // Bluetooth Slave Buffer

int wTimeSet;

void setup() {
  Serial.begin(9600);
  
  // Light Sensor
  als.begin();
  
  // Sound Sensor
  pinMode(sound_din,INPUT);
  pinMode(sound_ain,INPUT);

  // Bluetooth Slave
  BTSerial.begin(9600);

  wTimeSet = 0;
}

void loop() {
  // Light Sensor
  float lux;
  als.getALSLux(lux);
  Serial.print(String(lux) + " lux / ");

  // Temperature Sensor
  int chk = DHT11.read(DHT11PIN);
  Serial.print(String((float)DHT11.humidity) + " % / ");
  Serial.print(String((float)DHT11.temperature) + " oC / ");

  // Sound Sensor
  int ad_value = analogRead(sound_ain);
  Serial.println(String(ad_value) + " sound /");  

  // From BLT Master
  if(BTSerial.available()){
    int bytes = BTSerial.readBytes(buffer, 99);
    buffer[bytes] = 0;
    Serial.println("Master Said: " + String(buffer));
    wTimeSet = 1;
  }

  // Sensor Condition
  if(wTimeSet && (lux >= 100 || DHT11.humidity >= 90 || ad_value >= 500)){
     // To BLT Master
     BTSerial.write("TURNOFF");
     wTimeSet = 0;
  }

  delay(5000);
}
