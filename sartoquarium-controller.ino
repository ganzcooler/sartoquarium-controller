// OneWire & DallasTemperature libraries needed
#include <OneWire.h>
#include <DallasTemperature.h>

// DS18B20 settings
#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
float tempC;

// variables for delayed readings
uint16_t const INTERVAL = 1000;
uint32_t old_time = 0;

void setup(void) {
  Serial.begin(9600);
  sensors.begin();
}

void loop(void) {
  if (millis() - old_time > INTERVAL) {
    // get sensor readings
    sensors.requestTemperatures();
    tempC = sensors.getTempCByIndex(0);

    // Check if reading was successful
    if (tempC != DEVICE_DISCONNECTED_C) {
      Serial.print(tempC);
    }
    else {
        Serial.print("Error");
    }

    // reset old_time for delayed readings
    old_time = millis();
  }
}
