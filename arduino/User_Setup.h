// ##################################################################################
//  User_Setup.h — CYD ESP32-2432S028R, 2-USB-Variante (CYD2USB, ST7789-Panel)
//  Basis: Hersteller-User_Setup.h (JCZN) + Community-Pinout (witnessmenow),
//  angepasst für die 2-USB-Variante: ST7789-Treiber, BGR-Farbreihenfolge, Inversion an.
//  Autor: Paul Simmen mit Vibe (GLM-5-latest-short), 05.11.2026
//  Hinweis: Diese Datei ersetzt die User_Setup.h im installierten
//  TFT_eSPI-Bibliotheksordner (macOS: ~/Dokumente/Arduino/libraries/TFT_eSPI/).
//  Bei der 1-USB-Originalversion stattdessen: ILI9341_2_DRIVER, keine Inversion, TFT_RGB.
// ##################################################################################

// ###### Treiber ######
#define ST7789_DRIVER        // 2-USB-Variante (statt ILI9341_2_DRIVER beim Original)
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// ###### Farbreihenfolge und Inversion (2-USB-Variante) ######
#define TFT_RGB_ORDER TFT_BGR
#define TFT_INVERSION_ON
// Falls Farben trotzdem vertauscht wirken: TFT_RGB_ORDER TFT_RGB probieren
// Falls Weiss/Schwarz invertiert: TFT_INVERSION_OFF probieren

// ###### Hintergrundbeleuchtung ######
#define TFT_BL   21
#define TFT_BACKLIGHT_ON HIGH

// ###### Pins (bei 1-USB- und 2-USB-Variante identisch) ######
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1   // Panel-Reset hängt am Board-EN; Hersteller-File nutzt 12 — falls das Bild schwarz bleibt, 12 testen
#define TOUCH_CS 33   // Achtung: Touch (XPT2046) hängt auf EIGENEN SPI-Pins (SCK 25, MOSI 32, MISO 39, IRQ 36).
                      // TFT_eSPI-interne Touch-Funktionen (tft.getTouch) funktionieren deshalb NICHT.
                      // Für Touch: XPT2046_Touchscreen-Bibliothek oder Hersteller-Library TFT_Touch verwenden.

// ###### Fonts ######
#define LOAD_GLCD     // Font 1
#define LOAD_FONT2    // Font 2
#define LOAD_FONT4    // Font 4
#define LOAD_FONT6    // Font 6
#define LOAD_FONT7    // Font 7 (7-Segment)
#define LOAD_FONT8    // Font 8
#define LOAD_GFXFF    // FreeFonts
#define SMOOTH_FONT

// ###### SPI-Takt ######
// 40 MHz ist stabil; das Hersteller-File nutzt 55 MHz — nach erfolgreichem Test hochschrauben
#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
