/*
 *  CYD Display-Test — ESP32-2432S028R, 2-USB-Variante (ST7789)
 *  Prüft die Farbtreue nach der Umstellung der User_Setup.h:
 *  sechs Farbstreifen, Text — und unten blinkt die rote Board-LED (GPIO 4, aktiv LOW).
 *  Autor: Paul Simmen mit Vibe (GLM-5-latest-short), 05.11.2026
 */

#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

const int BAR_H = 40;  // 6 Streifen à 40 px = 240 px (Portrait)

void setup() {
  Serial.begin(115200);

  tft.init();
  tft.setRotation(0);   // 0 = Portrait 240x320

  uint16_t colors[] = { TFT_RED, TFT_GREEN, TFT_BLUE,
                        TFT_YELLOW, TFT_MAGENTA, TFT_CYAN };
  for (int i = 0; i < 6; i++) {
    tft.fillRect(0, i * BAR_H, 240, BAR_H, colors[i]);
  }

  // Untere Zone schwarz + Text
  tft.fillRect(0, 6 * BAR_H, 240, 320 - 6 * BAR_H, TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 250);
  tft.println("CYD Display-Test");
  tft.setCursor(10, 275);
  tft.println("ST7789 2USB OK");

  Serial.println("Display-Test gezeichnet.");
}

void loop() {
  // Rote Kanal der RGB-LED blinken lassen (LOW = an)
  pinMode(4, OUTPUT);
  digitalWrite(4, LOW);
  delay(500);
  digitalWrite(4, HIGH);
  delay(500);
}
