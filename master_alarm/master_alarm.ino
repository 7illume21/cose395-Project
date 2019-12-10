#include <SoftwareSerial.h>
#include <core_build_options.h>
#include <swRTC.h>
#include "pitches.h"

SoftwareSerial BTSerial(2, 3);
char buffer[100];
int buzzPin = 4;

unsigned long prevPlayTime = 0;
unsigned long playDuration = 0;
int currentMelody = 0;

swRTC rtc;

int wTimeSet;     // wakeup TIme is SET or NOT
int wTime;        // wakeup Time or NOT
int wakeUP;       // Wakeup or NOT
int timeExpired;   // Time for Wakeup Time Setup is Expired or NOT

int wHour, wMin, wSec;
int wHOffset, wMOffset, wSOffset;

void sing() {
    if(millis() < prevPlayTime + playDuration) {
        // music is playing. do nothing
        return;
    }
    // stop the tone playing:
    noTone(8);
    
    if(currentMelody >= melodySize)
        currentMelody = 0;
    int noteDuration = 1000/tempo[currentMelody];
 
    tone(buzzPin, melody[currentMelody], noteDuration);
    prevPlayTime = millis();
    playDuration = noteDuration * 1.30;
    
    currentMelody++;
}
 
void setup(){
  pinMode(buzzPin, OUTPUT);
  
  Serial.begin(9600);
  BTSerial.begin(9600);

  wakeUP = 0;
  wTimeSet = 0;
  timeExpired = 0;
  
  rtc.stopRTC();
  rtc.setTime(0, 0, 0);
  rtc.setDate(1, 1, 2020);
  rtc.startRTC();
}

void loop(){
  // BLT Input  
  if(BTSerial.available()){
    int bytes = BTSerial.readBytes(buffer, 99);
    buffer[bytes] = 0;
    Serial.println("Slave Said: " + String(buffer));
    wakeUP = 1;
  }

  // Serial Input
  if(Serial.available() && !timeExpired){
    int bytes = Serial.readBytes(buffer, 99);
    if(wTimeSet == 0) {
      buffer[8] = 0;  // exclude newline character
      String input = String(buffer);
      wHour = String(String(input[0]) + String(input[1])).toInt();
      wMin = String(String(input[3]) + String(input[4])).toInt();
      wSec = String(String(input[6]) + String(input[7])).toInt();

      if(wHour <= 23 && wMin <= 59 && wSec <= 59){
        Serial.print(String("(Wake up Time is set) After "));
        Serial.print(String(wHour) + "h " + String(wMin) + "m " + String(wSec) + "s ");
        Serial.println("alarm will be ON.");
        
        wTimeSet = 1;
        wHOffset = rtc.getHours();
        wMOffset = rtc.getMinutes();
        wSOffset = rtc.getSeconds();

        if(wSec + wSOffset > 60){
          wSec = wSec + wSOffset - 60;
          wMin++;
        }else{
          wSec = wSec + wSOffset;
        }
        
        if(wMin + wMOffset > 60){
          wMin = wMin + wMOffset - 60;
          wHour++;
        }else{
          wMin = wMin + wMOffset;
        }

        if(wHour + wHOffset > 24){
          Serial.println("** Wake Up Time set up is NOT POSSIBLE **");
          Serial.println("** PLEASE RELOAD THE PROGRAM **"); 
          timeExpired = 1;
        }
        wHour = wHour + wHOffset;

        // Notify To BLT Slave that wTime is Set
        BTSerial.write("Wake Up Time is SET");
      }else{
        Serial.println("** Invalid Input ** [Format] hh:mm:ss (hh should be less than 24)");
      }
    }else{
      Serial.println("Wake up Time is already set!");
    }
  }

  if(wTimeSet &&
      rtc.getHours() == wHour && 
      rtc.getMinutes() == wMin && 
      rtc.getSeconds() == wSec){
    wTime = 1;
  }
  
  if(wTime){
    if(!wakeUP){
      sing();
    }
  }
}
