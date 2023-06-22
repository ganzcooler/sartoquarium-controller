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
bool dht_success;

// DS18B20 settings
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature DS18B20(&oneWire);
float ds18b20_temp;
bool ds18b20_success;

// variables for delayed readings
uint16_t const INTERVAL = 1000;
uint32_t last_time = 0;

void setup(void)
{
  Serial.begin(9600);
  DS18B20.begin();
  dht.begin();
}

// main loop
void loop(void)
{
  // Delay
  if (millis() - last_time < INTERVAL)
  {
    return;
  }
  last_time = millis();

  // read sensors
  readDS18B20();
  readDHT22();

  // print to serial port
  sendToSerial();
}

void sendToSerial()
{
  /*
  Serial string syntax:
  <DS18B20 Temp>;<DHT Temp>;<DHT Hum>;<CR>
  */

  // DS18B20
  if (ds18b20_success)
  {
    Serial.print(ds18b20_temp);
    Serial.print(";");
  }
  else
  {
    Serial.print("ErrDS18");
    Serial.print(";");
  }

  // DHT22
  if (dht_success)
  {
    Serial.print(dht_temp);
    Serial.print(";");
    Serial.print(dht_hum);
    Serial.print(";\n");
  }
  else
  {
    Serial.print("ErrDHT");
    Serial.print(";");
    Serial.print("ErrDHT");
    Serial.print(";\n");
  }
}

void readDHT22()
{
  dht_success = true;
  dht_hum = dht.readHumidity();
  dht_temp = dht.readTemperature();

  // Check if reading was successful
  if (isnan(dht_hum) || isnan(dht_temp))
  {
    dht_success = false;
  }
}

void readDS18B20()
{
  ds18b20_success = true;
  DS18B20.requestTemperatures();
  ds18b20_temp = DS18B20.getTempCByIndex(0);

  // Check if reading was successful
  if (ds18b20_temp == DEVICE_DISCONNECTED_C)
  {
    ds18b20_success = false;
  }
}