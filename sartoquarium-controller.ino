// OneWire & DallasTemperature libraries needed
#include <OneWire.h>
#include <DallasTemperature.h>
#include "DHT.h"

#define DHTPIN 2
#define DHTTYPE DHT22
#define ONE_WIRE_BUS 3

// DHT22 (AM2302) settings
DHT dht(DHTPIN, DHTTYPE);
float dht_temp;
float dht_hum;
float dht_pres;

// DS18B20 settings
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
float temp_ds18b20;

// variables for delayed readings
uint16_t const INTERVAL = 1000;
uint32_t old_time = 0;

void setup(void) {
  Serial.begin(9600);
  DS18B20.begin();
  dht.begin();
}

void loop(void) {
  // Delay without delay function
  if (millis() - old_time < INTERVAL) {
    return;
  }
  // reset old_time for delayed readings
  old_time = millis();

  // get sensor readings
  DS18B20.requestTemperatures();
  temp_ds18b20 = DS18B20.getTempCByIndex(0);

  // Check if reading was successful
  if (temp_ds18b20 != DEVICE_DISCONNECTED_C) {
    Serial.print(temp_ds18b20);
  } else {
    Serial.print("Error reading DS18B20");
  }

  dht_hum = dht.readHumidity();
  dht_temp = dht.readTemperature();
  // Check if any reads from DHT failed and exit early (to try again).
  if (isnan(dht_hum) || isnan(dht_temp)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Serial.print("Humidity: ");
  Serial.print(dht_hum);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(dht_temp);
  Serial.println(" *C ");
}
