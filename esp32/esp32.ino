#include <WiFi.h>
#include <HTTPClient.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <DHT.h>
#include <HardwareSerial.h>

// WiFi
const char* ssid = "Krzysiek";
const char* password = "Bie7216A1.";

// HTTP server na Macu
const char* serverURL = "http://192.168.0.130:8080/dane";

// Czujniki
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature ds18b20(&oneWire);

#define DHTPIN 5
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// UART2 (np. do Arduino)
HardwareSerial SerialPort(2);  // RX=16, TX=17

void setup() {
  Serial.begin(9600);           // debug port
  SerialPort.begin(9600, SERIAL_8N1, 16, 17); // UART2

  WiFi.begin(ssid, password);
  Serial.print("Łączenie z WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nPołączono z WiFi. IP ESP32: " + WiFi.localIP().toString());

  ds18b20.begin();
  dht.begin();
}

void loop() {
  // Pomiar temperatur i wilgotności
  ds18b20.requestTemperatures();
  float tempDS = ds18b20.getTempCByIndex(0);
  float tempDHT = dht.readTemperature();
  float humDHT = dht.readHumidity();

  if (isnan(tempDS) || isnan(tempDHT) || isnan(humDHT)) {
    Serial.println("Blad odczytu z czujników");
  } else {
    // Formatowanie danych
    String data = "Temp1:";
    data += String(tempDS, 1);
    data += " Temp2:";
    data += String(tempDHT, 1);
    data += " Hig:";
    data += String((int)humDHT);
    data += "%";

    // Wysyłka po UART
    SerialPort.println(data);
    Serial.println("Wysłano po UART: " + data);

    // Wysyłka po HTTP GET
    if (WiFi.status() == WL_CONNECTED) {
      HTTPClient http;
      String url = String(serverURL) +
                   "?ds=" + String(tempDS, 1) +
                   "&dht=" + String(tempDHT, 1) +
                   "&hum=" + String(humDHT, 0);

      http.begin(url);
      int httpCode = http.GET();
      if (httpCode > 0) {
        Serial.println("Wysłano dane HTTP: " + url);
        Serial.println("HTTP code: " + String(httpCode));
      } else {
        Serial.println("Błąd HTTP: " + String(httpCode));
      }
      http.end();
    } else {
      Serial.println("Brak połączenia Wi-Fi");
    }
  }

  delay(3000); // co 3 sekundy
}
