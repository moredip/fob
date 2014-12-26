#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Time.h>

#include "sha1.h"
#include "TOTP.h"

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define OLED_RESET_PIN 4
#define PUSH_BUTTON_PIN 3

// The shared secret is MyLegoDoor
uint8_t hmacKey[] = {0x4d, 0x79, 0x4c, 0x65, 0x67, 0x6f, 0x44, 0x6f, 0x6f, 0x72};
TOTP totp = TOTP(hmacKey, 10);


Adafruit_SSD1306 display(OLED_RESET_PIN);

void setup() {                
  Serial.begin(9600);
  pinMode(PUSH_BUTTON_PIN, INPUT);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)

  display.clearDisplay();
  
  display.setTextSize(3);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println("OpenFob");
  display.display();
  delay(700);
}

void loop() {
  loopWithoutDelay(); 
  delay(100);
}

void loopWithoutDelay(){
  checkForTimeSync();
  
  if( timeStatus() == timeNotSet ){
    displayTimeSyncNeeded();
    return;
  }
  
  if( digitalRead(PUSH_BUTTON_PIN) ){
    writeTimeToDisplay();
  }else{
    blankDisplay();
  }
  
  String totpCode = totp.getCode(now());
  Serial.println( "TOTP: "+totpCode )
}
  
void blankDisplay(){
  display.clearDisplay();
  display.display();
}

void displayTimeSyncNeeded(){
  display.setTextSize(2);
  display.setTextColor(WHITE);
  
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("need\ntime sync");
  display.display();
}

// based on Time library's sample code
#define TIME_SYNC_HEADER "T"
#define SOME_TIME_IN_THE_PAST 1419630586
void checkForTimeSync(){  
  if (!Serial.available()) {
    return;
  }
  
  if (!Serial.find(TIME_SYNC_HEADER)) {
    return;
  }
  
  unsigned long pctime = Serial.parseInt();
  if( pctime < SOME_TIME_IN_THE_PAST) {
    return;
  }
  setTime(pctime);
  
  Serial.println("time synced:");
  writeTimeToSerial();
  
    display.setTextSize(2);
  display.setTextColor(WHITE);
  
  display.setTextSize(3);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("SYNCED!");
  display.display();
  delay(500);
}

void writeTimeToDisplay(){
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.clearDisplay();
  
  display.setCursor(0,0);
  display.println(currentTimeString());
  display.println(currentDateString());
  display.display();
}

String currentTimeString(){
  static char buffer[12];
  sprintf(buffer, "%02d:%02d:%02d", hour(), minute(), second());
  return String(buffer);
}

String currentDateString(){
  static char buffer[12];
  sprintf(buffer, "%d/%d/%d", day(), month(), year());
  return String(buffer);
}
  
void writeTimeToSerial(){
  Serial.println(currentTimeString());
  Serial.println(currentDateString());
}

