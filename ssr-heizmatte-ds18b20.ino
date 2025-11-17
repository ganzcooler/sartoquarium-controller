#include <OneWire.h>
#include <DallasTemperature.h>
#include "PID_v1.h"

// --- Pin Definitionen ---
const int oneWirePin = 7;
const int ssrPin = 5;

// --- OneWire & DallasTemperature Objekte ---
OneWire oneWire(oneWirePin);
DallasTemperature sensors(&oneWire);
const int EXPECTED_SENSORS = 2; // Erwartete Anzahl Sensoren

// --- Regelungsvariablen ---
double sollTemperatur = 20.0; // Soll-Wert für die Regelung
double istTemperatur = 0.0;   // Ist-Wert von Sensor 0
double leistung = 0.0;        // Ausgangsleistung für SSR (0-100)
const float SICHERHEITS_TEMP_MAX = 60.0; // Maximale Temperatur für Sensor 1

// --- PID Regelung ---
double Kp=10, Ki=0.1, Kd=25;
PID pid(&istTemperatur, &leistung, &sollTemperatur, Kp, Ki, Kd, DIRECT);

// --- SSR Variablen ---
const unsigned long zyklusDauer = 2000; // ms
unsigned long letzterWechselZeitpunktSSR = 0;
unsigned long einschaltZeitSSR = 0; // ms

// --- DS18B20 Variablen ---
unsigned long letzteAnforderungZeitDS18B20 = 0; // Zeit der letzten Anforderung
const unsigned long MIN_READ_INTERVAL_DS18B20 = 1000; // Mindestzeit zwischen Lesebeginn (Anforderung)
const int CONVERSION_TIME_DS18B20 = 750; // ms (für 12-bit)
float temperature1 = DEVICE_DISCONNECTED_C; // Regelungssensor
float temperature2 = DEVICE_DISCONNECTED_C; // Sicherheitssensor
bool conversionStartedDS18B20 = false;
int deviceCount = 0; // Anzahl gefundener Sensoren

void setup() {
  pinMode(ssrPin, OUTPUT);
  digitalWrite(ssrPin, LOW); // Sicherstellen, dass SSR zu Beginn aus ist

  Serial.begin(9600);
  while (!Serial) { ; } // Warten auf Serielle Verbindung

  sensors.begin();
  deviceCount = sensors.getDeviceCount();
  Serial.print("Anzahl DS18B20 Sensoren gefunden: ");
  Serial.println(deviceCount);

  if (deviceCount < EXPECTED_SENSORS) {
    Serial.println("FEHLER: Nicht alle erwarteten Sensoren gefunden! Verkabelung prüfen!");
  } else {
     Serial.println("Sensoren initialisiert.");
  }

  sensors.setWaitForConversion(false);

  // PID Regler initialisieren
  pid.SetMode(AUTOMATIC);
  pid.SetOutputLimits(0, 100); // Leistung in %
  pid.SetSampleTime(1000);     // PID alle 1000ms berechnen

  Serial.println("Temperaturregelung gestartet.");
  Serial.println("Senden Sie 'S<temperatur>' um den Sollwert zu ändern (z.B. S25.5).");
}

void loop() {
  unsigned long currentTime = millis();

  // --- Serielle Eingabe für Soll-Temperatur ---
  if (Serial.available() > 0) {
    String eingabeString = Serial.readStringUntil('\n');
    if (eingabeString.startsWith("S")) {
      float neuerSollwert = eingabeString.substring(1).toFloat();
      if (neuerSollwert > 0 && neuerSollwert < 100) { // Plausibilitätscheck
        sollTemperatur = neuerSollwert;
        Serial.print("Neuer Sollwert: ");
        Serial.println(sollTemperatur);
      } else {
        Serial.println("Ungültiger Sollwert.");
      }
    }
  }

  // --- Temperaturmessung DS18B20 (Nicht-blockierend) ---
  if (!conversionStartedDS18B20 && (currentTime - letzteAnforderungZeitDS18B20 >= MIN_READ_INTERVAL_DS18B20)) {
    if (deviceCount > 0) {
        if (sensors.requestTemperatures()) {
            letzteAnforderungZeitDS18B20 = currentTime;
            conversionStartedDS18B20 = true;
        } else {
            Serial.println("FEHLER beim Anfordern der Temperaturen!");
        }
    } else {
         letzteAnforderungZeitDS18B20 = currentTime;
    }
  }

  if (conversionStartedDS18B20 && (currentTime - letzteAnforderungZeitDS18B20 >= CONVERSION_TIME_DS18B20)) {
    if (deviceCount > 0) {
        temperature1 = sensors.getTempCByIndex(0);
        if (temperature1 != DEVICE_DISCONNECTED_C) {
            istTemperatur = temperature1; // Update für PID-Regler
        }
    }
    if (deviceCount > 1) {
        temperature2 = sensors.getTempCByIndex(1);
    }
    conversionStartedDS18B20 = false;
  }

  // --- Sicherheitsabschaltung ---
  if (temperature2 > SICHERHEITS_TEMP_MAX) {
    leistung = 0; // Heizung sofort ausschalten
    Serial.println("!!! SICHERHEITSABSCHALTUNG: Temperatur an Heizmatte zu hoch !!!");
  } else {
    // --- PID Regelung ---
    // Nur regeln, wenn der Sensor gültige Werte liefert
    if (temperature1 != DEVICE_DISCONNECTED_C) {
      pid.Compute();
    } else {
      leistung = 0; // Bei Sensorfehler sicherheitshalber ausschalten
    }
  }

  // Berechne die Einschaltdauer basierend auf der PID-Ausgabe
  einschaltZeitSSR = (leistung * zyklusDauer) / 100.0;

  // --- SSR Steuerung (Zeitproportional) ---
  if (leistung <= 0) {
    digitalWrite(ssrPin, LOW);
  } else if (leistung >= 100) {
    digitalWrite(ssrPin, HIGH);
  } else {
    if (currentTime - letzterWechselZeitpunktSSR >= zyklusDauer) {
      letzterWechselZeitpunktSSR += zyklusDauer;
      if (einschaltZeitSSR > 0) {
        digitalWrite(ssrPin, HIGH);
      }
    } else if (currentTime - letzterWechselZeitpunktSSR >= einschaltZeitSSR) {
      if (digitalRead(ssrPin) == HIGH) {
          digitalWrite(ssrPin, LOW);
      }
    }
  }

  // --- Serielle Ausgabe für Monitoring ---
  // (Wird nur alle 2 Sekunden ausgegeben, um die Lesbarkeit zu verbessern)
  static unsigned long letzteAusgabeZeit = 0;
  if (currentTime - letzteAusgabeZeit >= 2000) {
    letzteAusgabeZeit = currentTime;
    Serial.print("Soll: "); Serial.print(sollTemperatur);
    Serial.print(" C, Ist: ");
    if (temperature1 == DEVICE_DISCONNECTED_C) {
      Serial.print("Fehler!");
    } else {
      Serial.print(istTemperatur); Serial.print(" C");
    }
    Serial.print(", T_sicher: ");
    if (temperature2 == DEVICE_DISCONNECTED_C) {
      Serial.print("Fehler!");
    } else {
      Serial.print(temperature2); Serial.print(" C");
    }
    Serial.print(", Leistung: "); Serial.print(leistung); Serial.println(" %");
  }
}