/*
  CYD_TouchTest.ino - Touch-Test und -Kalibrierung
  fuer das CYD (Cheap Yellow Display) ESP32-2432S028R,
  2-USB-Variante von JCZN, ILI9341-Display, resistiver Touch XPT2046.

  Voraussetzung: CYD_DisplayTest laeuft
  (User_Setup.h aus dem Ordner arduino/ dieses Repos in TFT_eSPI uebernehmen).

  Bibliotheken (Arduino-Bibliotheksverwalter):
   - TFT_eSPI (Bodmer)
   - XPT2046_Touchscreen (Paul Stoffregen) - keine Konfiguration noetig,
     alle Pins werden in diesem Sketch gesetzt.

  Hardware-Hintergrund:
   Der Touch-Controller XPT2046 haengt am ZWEITEN SPI-Bus mit eigenen Pins
   (T_CLK 25, T_DIN/MOSI 32, T_OUT/MISO 39, T_CS 33, T_IRQ 36) -
   unabhaengig vom Display-SPI (MISO 12, MOSI 13, SCLK 14, CS 15).
   Deshalb funktionieren die TFT_eSPI-eigenen Touchfunktionen nicht.

  Bedienung:
   - Finger/Pen auf die schwarze Flaeche: malen (Test auf Genauigkeit)
   - Farbfelder unten: Stiftfarbe wechseln, CLR = Flaeche leeren
   - Serial Monitor (115200 Baud):
       c  Kalibrierung starten (4 Kreuze nacheinander anfassen)
       x  Zeichenflaeche leeren

  Autor: Paul Simmen mit Vibe (GLM-5-latest-short), 05.11.2026
*/

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// ---------- Touch-Pins (XPT2046, separater SPI-Bus) ----------
#define XPT2046_IRQ  36  // T_IRQ
#define XPT2046_MOSI 32  // T_DIN
#define XPT2046_MISO 39  // T_OUT
#define XPT2046_CLK  25  // T_CLK
#define XPT2046_CS   33  // T_CS

// ---------- Kalibrierwerte: RAW-Wert am jeweiligen Bildschirmrand ----------
// Startwerte nach Random Nerd Tutorials. Nach der Kalibrierung ('c') im
// Serial Monitor/Display die vorgeschlagenen Werte hier eintragen.
#define TS_X_AT_0    200
#define TS_X_AT_319  3700
#define TS_Y_AT_0    240
#define TS_Y_AT_239  3800

// ---------- Layout (Landscape, Rotation 1: 320 x 240) ----------
#define W 320
#define H 240
#define BAR_H     20   // Statusleiste oben
#define PAL_H     24   // Farbleiste unten
#define PAINT_TOP BAR_H
#define PAINT_BOT (H - PAL_H)
#define FIELD_W   53   // Breite eines Farbfelds (320/6)

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(HSPI);  // Touch am 2. SPI-Baustein.
// Sollte der Touch nicht reagieren: SPIClass(VSPI) probieren und
// unten touch.begin(touchSPI) unverändert lassen.

XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// Aktuelle Kalibrierwerte (float, damit auch gespiegelte Achsen funktionieren)
float tsX0 = TS_X_AT_0, tsX1 = TS_X_AT_319;
float tsY0 = TS_Y_AT_0, tsY1 = TS_Y_AT_239;

const int NUM_COLORS = 5;
const int colors[NUM_COLORS] = { TFT_WHITE, TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW };
int penColor = TFT_WHITE;
int selectedField = 0;

int lastX = -1, lastY = -1;  // fuer Linien zwischen Messpunkten

// ---------------------------------------------------------------
// Hilfsfunktionen
// ---------------------------------------------------------------

// RAW-Wert linear auf Bildschirmkoordinate mappen.
// ts_at_min/ts_at_max = RAW-Wert an Bildschirmposition 0 bzw. scrMax.
// Funktioniert auch bei gespiegelten Achsen (ts_at_min > ts_at_max).
int mapRaw(long raw, float atMin, float atMax, int scrMax) {
  float v = (raw - atMin) * (float)scrMax / (atMax - atMin);
  int r = (int)v;
  if (r < 0) r = 0;
  if (r > scrMax) r = scrMax;
  return r;
}

long median9(long *a) {
  // Insertion Sort, dann mittlerer Wert
  for (int i = 1; i < 9; i++) {
    long key = a[i];
    int j = i - 1;
    while (j >= 0 && a[j] > key) { a[j + 1] = a[j]; j--; }
    a[j + 1] = key;
  }
  return a[4];
}

void drawCross(int x, int y, int c) {
  tft.drawCircle(x, y, 8, c);
  tft.drawFastHLine(x - 14, y, 28, c);
  tft.drawFastVLine(x, y - 14, 28, c);
}

void clearPaint() {
  tft.fillRect(0, PAINT_TOP, W, PAINT_BOT - PAINT_TOP, TFT_BLACK);
}

void drawPalette() {
  for (int i = 0; i < 6; i++) {
    int x = i * FIELD_W;
    if (i < NUM_COLORS) {
      tft.fillRect(x, PAINT_BOT, FIELD_W - 2, PAL_H, colors[i]);
    } else {
      tft.fillRect(x, PAINT_BOT, W - x, PAL_H, TFT_DARKGREY);
      tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
      tft.drawCentreString("CLR", x + (W - x) / 2, PAINT_BOT + 5, 2);
    }
  }
  // Rahmen um gewaehltes Farbfeld
  tft.drawRect(selectedField * FIELD_W, PAINT_BOT, FIELD_W, PAL_H, TFT_ORANGE);
}

void drawUI() {
  tft.fillRect(0, 0, W, BAR_H, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.drawString("Touch-Test: malen!   c = Kalibrierung", 2, 1, 2);
  clearPaint();
  drawPalette();
}

void showStatus(TS_Point &p, int x, int y) {
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  String s = "x=" + String(x) + " y=" + String(y) + "  RAW " + String(p.x) + "/" + String(p.y) + "   ";
  tft.drawString(s, 2, 1, 2);
}

// Auf Touch am aktuellen Ziel warten, 9 Messwerte nehmen, Median liefern.
// Rueckgabe false bei Timeout (15 s).
bool sampleTarget(float &outX, float &outY) {
  long sx[9], sy[9];
  long t0 = millis();
  while (!(touch.tirqTouched() && touch.touched())) {
    if (millis() - t0 > 15000) return false;
    delay(10);
  }
  for (int i = 0; i < 9; i++) {
    TS_Point p = touch.getPoint();
    sx[i] = p.x;
    sy[i] = p.y;
    delay(30);
  }
  outX = median9(sx);
  outY = median9(sy);
  return true;
}

// ---------------------------------------------------------------
// Kalibrierung: 4 Kreuze anfassen, dann Vorschlaege ausgeben
// ---------------------------------------------------------------
void calibrate() {
  // Zielpunkte: oben links, oben rechts, unten links, unten rechts
  const int tg[4][2] = { {40, 60}, {280, 60}, {40, 200}, {280, 200} };
  const char *names[4] = { "oben links", "oben rechts", "unten links", "unten rechts" };
  float rx[4], ry[4];

  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.drawCentreString("Kalibrierung", W / 2, 4, 2);
  tft.drawCentreString("4 Kreuze nacheinander anfassen", W / 2, 24, 2);

  for (int i = 0; i < 4; i++) {
    tft.fillRect(0, 100, W, 20, TFT_BLACK);
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawCentreString(String(i + 1) + "/4: " + names[i], W / 2, 100, 2);
    drawCross(tg[i][0], tg[i][1], TFT_CYAN);

    if (!sampleTarget(rx[i], ry[i])) {
      tft.fillRect(0, 100, W, 20, TFT_BLACK);
      tft.setTextColor(TFT_RED, TFT_BLACK);
      tft.drawCentreString("Timeout - Abbruch", W / 2, 100, 2);
      delay(2000);
      drawUI();
      return;
    }
    drawCross(tg[i][0], tg[i][1], TFT_GREEN);
    Serial.println("Ziel " + String(i + 1) + " (" + names[i] + "): RAW x=" +
                   String((long)rx[i]) + " y=" + String((long)ry[i]));
    while (touch.touched()) delay(20);  // auf Loslassen warten
    delay(300);
  }

  // Lineare Inter-/Extrapolation auf die Bildschirmraender
  tsX0 = rx[0] + (rx[1] - rx[0]) * (float)(0   - tg[0][0]) / (tg[1][0] - tg[0][0]);
  tsX1 = rx[0] + (rx[1] - rx[0]) * (float)(319 - tg[0][0]) / (tg[1][0] - tg[0][0]);
  tsY0 = ry[0] + (ry[2] - ry[0]) * (float)(0   - tg[0][1]) / (tg[2][1] - tg[0][1]);
  tsY1 = ry[0] + (ry[2] - ry[0]) * (float)(239 - tg[0][1]) / (tg[2][1] - tg[0][1]);

  // Qualitaetspruefung mit dem 4. Punkt (unten rechts, Soll 280/200)
  int px = mapRaw((long)rx[3], tsX0, tsX1, W - 1);
  int py = mapRaw((long)ry[3], tsY0, tsY1, H - 1);
  float err = sqrtf((px - 280) * (px - 280) + (py - 200) * (py - 200));

  Serial.println();
  Serial.println("---- Kalibrierwerte: in CYD_TouchTest.ino eintragen ----");
  Serial.println("#define TS_X_AT_0    " + String((long)tsX0));
  Serial.println("#define TS_X_AT_319  " + String((long)tsX1));
  Serial.println("#define TS_Y_AT_0    " + String((long)tsY0));
  Serial.println("#define TS_Y_AT_239  " + String((long)tsY1));
  Serial.println("-------------------------------------------------------");
  Serial.println("Pruefpunkt unten rechts: Soll 280/200, gemessen " +
                 String(px) + "/" + String(py) + ", Abweichung " +
                 String((int)(err + 0.5)) + " px");

  tft.fillRect(0, 100, W, 80, TFT_BLACK);
  tft.setTextColor(TFT_GREEN, TFT_BLACK);
  tft.drawCentreString("Fertig! Werte im Serial Monitor.", W / 2, 104, 2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawCentreString("TS_X " + String((long)tsX0) + ".." + String((long)tsX1), W / 2, 126, 2);
  tft.drawCentreString("TS_Y " + String((long)tsY0) + ".." + String((long)tsY1), W / 2, 146, 2);
  tft.drawCentreString("Pruef-Abweichung: " + String((int)(err + 0.5)) + " px", W / 2, 166, 2);
  delay(4000);
  drawUI();
}

// ---------------------------------------------------------------
// Setup und Hauptschleife
// ---------------------------------------------------------------

void setup() {
  Serial.begin(115200);

  // Touch-SPI zuerst starten (eigener Bus, stoert das Display nicht)
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touch.begin(touchSPI);
  touch.setRotation(1);

  tft.init();
  tft.setRotation(1);

  drawUI();
  Serial.println("CYD Touch-Test bereit. c = Kalibrierung, x = loeschen");
}

void loop() {
  // Serielle Befehle
  while (Serial.available()) {
    char c = Serial.read();
    if (c == 'c') calibrate();
    else if (c == 'x') { clearPaint(); Serial.println("Zeichenflaeche geleert"); }
  }

  if (touch.tirqTouched() && touch.touched()) {
    TS_Point p = touch.getPoint();
    int x = mapRaw(p.x, tsX0, tsX1, W - 1);
    int y = mapRaw(p.y, tsY0, tsY1, H - 1);
    showStatus(p, x, y);

    if (y >= PAINT_TOP && y < PAINT_BOT) {
      // Malen (Linie zum letzten Punkt, wenn plausibel nahe)
      if (lastX >= 0 && (abs(x - lastX) + abs(y - lastY)) < 80) {
        tft.drawLine(lastX, lastY, x, y, penColor);
      }
      tft.fillCircle(x, y, 2, penColor);
      lastX = x;
      lastY = y;
    } else if (y >= PAINT_BOT) {
      // Farbleiste: 5 Farben + CLR
      int idx = x / FIELD_W;
      if (idx < NUM_COLORS) {
        penColor = colors[idx];
        selectedField = idx;
      } else {
        clearPaint();
      }
      drawPalette();
      lastX = -1;
      lastY = -1;
    } else {
      lastX = -1;
      lastY = -1;
    }
    delay(10);
  } else {
    lastX = -1;
    lastY = -1;
  }
}
