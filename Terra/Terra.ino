#include <SPI.h>
#include "LCD_Driver.h"
#include "GUI_Paint.h"
#include "image.h"
#include <IRremote.hpp>

#define IR_RECEIVE_PIN 2

bool systemOn = false;
String lastMessage = "";
String displayedMessage = "";

// Funkcja do wyświetlania danych temperatur na czarnym tle (zielony tekst)
void displayTemps(const String& data) {
  Paint_Clear(BLACK);

  int t1Index = data.indexOf("Temp1:");
  int t2Index = data.indexOf("Temp2:");
  int higIndex = data.indexOf("Hig:");

  String temp1 = "";
  String temp2 = "";
  String hig = "";

  if (t1Index != -1 && t2Index != -1 && higIndex != -1) {
    temp1 = data.substring(t1Index + 6, t2Index);
    temp1.trim();

    temp2 = data.substring(t2Index + 6, higIndex);
    temp2.trim();

    hig = data.substring(higIndex + 4);
    hig.trim();
  } else {
    Paint_DrawString_EN(10, 10, data.c_str(), &Font16, GREEN, BLACK);
    return;
  }

  Paint_DrawString_EN(60, 90, ("Temp1: " + temp1).c_str(), &Font16, BLACK, GREEN);
  Paint_DrawString_EN(60, 140, ("Temp2: " + temp2).c_str(), &Font16, BLACK, GREEN);
  Paint_DrawString_EN(60, 190, ("Hig: " + hig).c_str(), &Font16, BLACK, GREEN);
}

// Funkcja do wyświetlania menu
void displayMenu() {
  Paint_Clear(WHITE);
  Paint_DrawString_EN(10, 10, "Menu", &Font24, WHITE, BLACK);
  Paint_DrawString_EN(10, 30, "1.Regulacja temp", &Font16, BLACK, WHITE);
  Paint_DrawString_EN(10, 50, "2. Czujnik otwarcia", &Font16, BLACK, WHITE);
  Paint_DrawString_EN(10, 70, "3. Test buzzer", &Font16, BLACK, WHITE);
  Paint_DrawRectangle(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1, BLACK, DOT_PIXEL_1X1, DRAW_FILL_EMPTY);
}

void setup() {
  Config_Init();
  LCD_Init();
  LCD_Clear(0x0000);
  Paint_NewImage(LCD_WIDTH, LCD_HEIGHT, 0, BLACK);
  Paint_Clear(BLACK);

  Serial.begin(9600);
  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  IrReceiver.blink13(true);
}

void loop() {
  // Obsługa pilota IR
  if (IrReceiver.decode()) {
    if (IrReceiver.decodedIRData.protocol == NEC) {
      uint8_t cmd = IrReceiver.decodedIRData.command;

      if (cmd == 0x45) {  // Włącz/wyłącz system
        systemOn = !systemOn;
        if (systemOn) {
          displayMenu();
        } else {
          // Wyłączony - wyświetl temperatury z ostatnich danych
          displayTemps(lastMessage);
        }
      }
    }
    IrReceiver.resume();
  }

  // Odbiór UART od ESP32
  if (Serial.available()) {
    String received = Serial.readStringUntil('\n');
    received.trim();
    if (received.length() > 0) {
      Serial.println("Otrzymano z ESP32: " + received);
      lastMessage = received;
      Serial.println("ACK");

      if (!systemOn) {
        // Aktualizuj wyświetlanie temperatur jeśli system wyłączony
        displayTemps(lastMessage);
      }
    }
  }

  delay(50);
}
