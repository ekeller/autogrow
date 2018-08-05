/* RTC - DS1307 */
#include <Wire.h>
#include "RTClib.h"
/* Thermometer - DS18B20 */
#include <OneWire.h>
#include <DallasTemperature.h>
/* LCD */
#include <LiquidCrystal.h>

#define TEMPERATURE 10
#define R1 24
#define R2 25
#define R3 26
#define R4 27
#define R5 28
#define R6 29
#define R7 30
#define R8 31

#define DEBUG false

const int relay_array[] = {R1, R2, R3, R4, R5, R6, R7, R8};

const int rs = 2, en = 3, d4 = 4, d5 = 5, d6 = 6, d7 = 7;

char daysOfTheWeek[7][12] = {"Dom", "Seg", "Ter", "Qua", "Qui", "Sex", "Sab"};

LiquidCrystal LCD(rs, en, d4, d5, d6, d7);

RTC_DS1307 rtc;

// Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
OneWire oneWire(TEMPERATURE);

// Pass our oneWire reference to Dallas Temperature.
DallasTemperature sensors(&oneWire);

DeviceAddress insideThermometer;

void discoverOneWireDevices(void) {
  byte i;
  byte data[12];
  byte addr[8];
  String debugger;
  
  while(oneWire.search(addr)) {
    debugger.concat("Found \'1-Wire\' device with address:\n\r");
    for(i = 0; i < 8; i++) {
      debugger.concat("0x");
      if (addr[i] < 16) {
        debugger.concat('0');
      }
      debugger.concat(String(addr[i], HEX));
      if (i < 7) {
        debugger.concat(", ");
      }
      insideThermometer[i] = addr[i];
    }
    if (OneWire::crc8(addr, 7) != addr[7]) {
        debugger.concat("CRC is not valid!\n");
        return;
    }
    if (DEBUG) Serial.println(debugger);
  }
  oneWire.reset_search();
  return;
}

/*
 * Relay setup
 */
void relays_setup() {
  int arraySize = sizeof(relay_array) / sizeof(int);
  
  for (int i = 0; i < arraySize; i++) {
    pinMode(relay_array[i], OUTPUT);
    
    if (DEBUG) {
      Serial.println("SETUP relay on port " + String(relay_array[i-1], DEC));
    }
  }
}

/*
 * Turns relay on
 */
void relays_on(int i) {
  digitalWrite(relay_array[i-1], HIGH);

  if (DEBUG) {
    Serial.println("ACTIVATE relay on port " + String(relay_array[i-1], DEC));
  }
}

/*
 * Turns relay off
 */
void relays_off(int i) {
  digitalWrite(relay_array[i-1], LOW);

  if (DEBUG) {
    Serial.println("DEACTIVATE relay on port " + String(relay_array[i-1], DEC));
  }
}

void setup(void) {
  Serial.begin(9600);

  if (!rtc.begin() && DEBUG) {
    Serial.println("Couldn't find RTC");
    while (1);
  }

  if (!rtc.isrunning() && DEBUG) {
    Serial.println("RTC is NOT running!");
    // This line sets the RTC with an explicit date & time, for example to set
    // January 21, 2014 at 3am you would call:
    // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
  }

  //auto update from computer time when uploading
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // check for oneWire sensors
  discoverOneWireDevices();
  // temperature sensor
  sensors.begin();
  // setup pinMode for relays
  relays_setup();
  // set up the LCD's number of columns and rows
  LCD.begin(16, 2);
}

void printTemperature(DeviceAddress deviceAddress){
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00) {
    Serial.println("Error getting temperature");
  } else {
    Serial.println("Current temperature");
    Serial.print("C: ");
    Serial.print(tempC);
    Serial.print(" F: ");
    Serial.print(DallasTemperature::toFahrenheit(tempC));
    Serial.println();
  }
}

void printTemperatureForLCD(DeviceAddress deviceAddress){
  float tempC = sensors.getTempC(deviceAddress);
  if (tempC == -127.00) {
    LCD.clear();
    LCD.print("Error getting temperature");
  } else {
    LCD.print(String("T C:" + String(tempC, 1) + " F:" + String(DallasTemperature::toFahrenheit(tempC), 1)));
  }
}

void printDateForLCD(const DateTime now){
  String dateSeparator = "/";
  String timeSeparator = ":";
  String day = now.day() < 9 ? "0" + String(now.day(), DEC) : String(now.day(), DEC);
  String month = now.month() < 9 ? "0" + String(now.month(), DEC) : String(now.month(), DEC);
  String year = String(now.year(), DEC).substring(2);
  
  String dateTime = day + dateSeparator;
  dateTime.concat(month + dateSeparator);
  dateTime.concat(year);
  dateTime.concat(" ");
  dateTime.concat(String(now.hour(), DEC) + timeSeparator);
  dateTime.concat(String(now.minute(), DEC) + timeSeparator);
  dateTime.concat(String(now.second(), DEC));

  LCD.print(dateTime);
  
//  if (dateTime.length() > 16) {
//    for (int i = 16; i < dateTime.length(); i++) {
//      LCD.scrollDisplayLeft();
//    }
//    delay(1000);
//    for (int j = 16; j < dateTime.length(); j++) {
//      LCD.scrollDisplayRight();
//    }
//  }
}

void loop(void) {
  DateTime now = rtc.now();
  
  if (DEBUG) {
    Serial.println(String(now.year(), DEC) + "/" +
      String(now.month(), DEC) + "/" +
      String(now.day(), DEC) + " " +
      String(now.hour(), DEC) + ":" +
      String(now.minute(), DEC) + ":" +
      String(now.second(), DEC)
    );
  }

  sensors.requestTemperatures();
  Serial.print("TEMP: ");
  printTemperature(insideThermometer);

  LCD.setCursor(0, 0);
  printDateForLCD(now);
  LCD.setCursor(0, 1);
  printTemperatureForLCD(insideThermometer);

  relays_on(1);
  relays_on(2);
  relays_on(3);
  relays_on(4);
  relays_on(5);
  relays_on(6);
  relays_on(7);
  relays_on(8);
  delay(1000);
}
