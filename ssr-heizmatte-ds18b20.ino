#include <OneWire.h>
#include <DallasTemperature.h>

// --- Pin Definitionen ---
const int oneWirePin = 7;
const int ssrPin = 5;

// --- OneWire & DallasTemperature Objekte ---
OneWire oneWire(oneWirePin);
DallasTemperature sensors(&oneWire);
const int EXPECTED_SENSORS = 2; // Erwartete Anzahl Sensoren

// --- SSR Variablen ---
const unsigned long zyklusDauer = 2000; // ms
unsigned long letzterWechselZeitpunktSSR = 0;
int leistung = 0; // 0-100 %
unsigned long einschaltZeitSSR = 0; // ms

// --- DS18B20 Variablen ---
unsigned long letzteAnforderungZeitDS18B20 = 0; // Zeit der letzten Anforderung
const unsigned long MIN_READ_INTERVAL_DS18B20 = 1000; // Mindestzeit zwischen Lesebeginn (Anforderung)
const int CONVERSION_TIME_DS18B20 = 750; // ms (für 12-bit)
float temperature1 = DEVICE_DISCONNECTED_C; // Initialisieren mit Fehlerwert
float temperature2 = DEVICE_DISCONNECTED_C; // Initialisieren mit Fehlerwert
bool conversionStartedDS18B20 = false;
int deviceCount = 0; // Anzahl gefundener Sensoren

void setup() {
  pinMode(ssrPin, OUTPUT);
  digitalWrite(ssrPin, LOW); // Sicherstellen, dass SSR zu Beginn aus ist

  Serial.begin(9600);
  while (!Serial) { ; } // Warten auf Serielle Verbindung (für bestimmte Arduinos nötig)

  sensors.begin();
  deviceCount = sensors.getDeviceCount(); // Einmalige Prüfung der Sensoranzahl
  Serial.print("Anzahl DS18B20 Sensoren gefunden: ");
  Serial.println(deviceCount);

  if (deviceCount < EXPECTED_SENSORS) {
    Serial.println("FEHLER: Nicht alle erwarteten Sensoren gefunden! Verkabelung prüfen!");
    // Optional: Programm anhalten oder in einen sicheren Zustand gehen
    // while(true);
  } else {
     Serial.println("Sensoren initialisiert.");
  }

  sensors.setWaitForConversion(false); // Nicht-blockierende Abfragen aktivieren

  Serial.println("Bereit für die Eingabe von Leistungswerten (0-100):");
}

void loop() {
  unsigned long currentTime = millis();

  // --- Serielle Eingabe für SSR Leistung ---
  if (Serial.available() > 0) {
    String eingabeString = Serial.readStringUntil('\n');
    int eingabeWert = eingabeString.toInt();

    if (eingabeWert >= 0 && eingabeWert <= 100) {
      leistung = eingabeWert;
      einschaltZeitSSR = (leistung * zyklusDauer) / 100; // Berechnung mit unsigned long für Genauigkeit
      Serial.print("Eingestellte Leistung: ");
      Serial.print(leistung);
      Serial.println("%");
    } else {
      Serial.println("Ungültiger Leistungswert (0-100).");
    }
  }

  // --- Temperaturmessung DS18B20 (Nicht-blockierend) ---
  // 1. Prüfen, ob neue Messung angefordert werden soll
  if (!conversionStartedDS18B20 && (currentTime - letzteAnforderungZeitDS18B20 >= MIN_READ_INTERVAL_DS18B20)) {
    if (deviceCount > 0) { // Nur anfordern, wenn Sensoren beim Start gefunden wurden
        if (sensors.requestTemperatures()) { // Befehl an *alle* Sensoren
            letzteAnforderungZeitDS18B20 = currentTime; // Zeitpunkt der Anforderung merken
            conversionStartedDS18B20 = true;
        } else {
            Serial.println("FEHLER beim Anfordern der Temperaturen!");
        }
    } else {
         // Keine Sensoren vorhanden, nichts anfordern
         // Serial.println("Keine Sensoren zum Anfordern vorhanden."); // Debug-Ausgabe ggf.
         letzteAnforderungZeitDS18B20 = currentTime; // Verhindert ständiges Prüfen, wenn keine Sensoren da sind
    }
  }

  // 2. Prüfen, ob Konvertierung abgeschlossen ist und gelesen werden kann
  if (conversionStartedDS18B20 && (currentTime - letzteAnforderungZeitDS18B20 >= CONVERSION_TIME_DS18B20)) {
    Serial.print(currentTime);
    Serial.print(" ms; Lese Temperaturen: ");

    if (deviceCount > 0) { // Lese nur, wenn Sensoren erwartet werden
        temperature1 = sensors.getTempCByIndex(0);
        Serial.print(" T1=");
        if (temperature1 == DEVICE_DISCONNECTED_C) {
            Serial.print("Fehler!");
        } else {
            Serial.print(temperature1);
            Serial.print("C");
        }
    }

    if (deviceCount > 1) { // Lese zweiten Sensor nur, wenn erwartet
        temperature2 = sensors.getTempCByIndex(1);
         Serial.print(" T2=");
        if (temperature2 == DEVICE_DISCONNECTED_C) {
            Serial.print("Fehler!");
        } else {
            Serial.print(temperature2);
            Serial.print("C");
        }
    }
    Serial.println();
    conversionStartedDS18B20 = false; // Flag zurücksetzen, Konvertierung abgeschlossen
  }


  // --- SSR Steuerung (Zeitproportional) ---
  if (leistung <= 0) {
    digitalWrite(ssrPin, LOW);
  } else if (leistung >= 100) {
    digitalWrite(ssrPin, HIGH);
  } else {
    // Zeitproportionale Steuerung für Werte zwischen 0 und 100
    if (currentTime - letzterWechselZeitpunktSSR >= zyklusDauer) {
      // Starte einen neuen Zyklus
      letzterWechselZeitpunktSSR += zyklusDauer; // Merke den Startzeitpunkt des Zyklus
      if (einschaltZeitSSR > 0) {
        digitalWrite(ssrPin, HIGH);             // Schalte zu Beginn des Zyklus ein
      }
      // Serial.print(currentTime); Serial.println(" ms; SSR Zyklus Start: EIN"); // Debug
    } else if (currentTime - letzterWechselZeitpunktSSR >= einschaltZeitSSR) {
      // Wenn die Einschaltdauer abgelaufen ist, ausschalten
      // Prüfe zusätzlich, ob es überhaupt an war, um unnötiges Schreiben zu vermeiden (optional aber gut)
      if (digitalRead(ssrPin) == HIGH) {
          digitalWrite(ssrPin, LOW);
          // Serial.print(currentTime); Serial.println(" ms; SSR Zyklus: AUS"); // Debug
      }
    }
    // Ansonsten (zwischen HIGH und Ablauf einschaltZeitSSR) bleibt der Zustand HIGH
  }
}