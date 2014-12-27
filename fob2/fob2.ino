#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Time.h>

#include "sha1.h"
#include "TOTP.h"
#include "key.h"

#include "secret_keys.h"

#if (SSD1306_LCDHEIGHT != 32)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

#define OLED_RESET_PIN 4
#define PUSH_BUTTON_PIN 3
#define PUSH_BUTTON_INTERRUPT 1

// define MY_SECRET_KEYS in secret_keys.h
Key keys[] = MY_SECRET_KEYS;
int numberOfKeys = (sizeof(keys)/sizeof(*keys));
int numberOfScreens = numberOfKeys + 1; // for clock display

volatile int buttonPressCounter = 0;

Adafruit_SSD1306 display(OLED_RESET_PIN);

void setup() {                
  Serial.begin(9600);
  
  pinMode(PUSH_BUTTON_PIN, INPUT);
  
  attachInterrupt(PUSH_BUTTON_INTERRUPT, onButtonPress, RISING);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 128x32)
  display.setTextColor(WHITE);
  display.setTextWrap(false);
  display.clearDisplay();
  
  display.setTextSize(3);
  display.setCursor(0,0);
  display.println("OpenFob");
  display.display();
  delay(700);
}

void loop() {
  loopWithoutDelay(); 
  delay(100);
}

void onButtonPress(){
  static unsigned long last_interrupt_time = 0;
  const unsigned long interrupt_time = millis();
  
  if( interrupt_time - last_interrupt_time > 50 ){ // debouncing
    buttonPressCounter += 1;
  }
  last_interrupt_time = interrupt_time;
}

void loopWithoutDelay(){
  checkForTimeSync();
  
  if( timeStatus() == timeNotSet ){
    displayTimeSyncNeeded();
    return;
  }
  
  //if( digitalRead(PUSH_BUTTON_PIN) ){
      displayCurrentScreen();
//  }else{
//    blankDisplay();
//  }
}

void blankDisplay(){
  display.clearDisplay();
  display.display();
}

void displayTimeSyncNeeded(){
  display.setTextSize(2);
  
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
    
  display.setTextSize(3);
  display.clearDisplay();
  display.setCursor(0,0);
  display.println("SYNCED!");
  display.display();
  delay(500);
}

void displayCurrentScreen(){
  int screenIx = buttonPressCounter % numberOfScreens;
  if( screenIx == 0 ){
    displayClockScreen();
  }else{
    Key &keyToDisplay = keys[screenIx-1];
    displayKeyScreen(keyToDisplay);
  }
}

void displayClockScreen(){
  display.setTextSize(2);
  display.invertDisplay(true);

  display.clearDisplay();
  
  display.setCursor(0,0);
  display.println(currentTimeString());
  display.println(currentDateString());
  display.display();
}

void displayKeyScreen(Key &key){
  display.setTextSize(2);
  display.invertDisplay(false);

  display.clearDisplay();
  
  display.setCursor(0,0);
  display.print(key.getName());
  display.setCursor(0,16);
  display.print("   " + key.getCurrentCode());
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

