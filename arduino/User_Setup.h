// ##################################################################################
//  User_Setup.h — CYD ESP32-2432S028R (JCZN, 2-USB-Variante, ILI9341-Panel)
//  Abgeleitet aus der funktionierenden Hersteller-IDF-Referenz (lcd.c/lcd.h, Demo 5_1_LCD_Test):
//  ILI9341-Treiber, BGR-Farbreihenfolge, KEINE Inversion, Reset unbenutzt (-1),
//  30 MHz SPI-Takt, Pins MISO 12 / MOSI 13 / SCLK 14 / CS 15 / DC 2 / BL 21.
//  WICHTIG: TFT_RST bleibt -1 — GPIO 12 ist die Panel-Datenleitung (SDO),
//  ein Reset-Impuls darauf blockiert das Display (erprobt am 05.11.2026).
//  Autor: Paul Simmen mit Vibe (GLM-5-latest-short), 05.11.2026
//  Bei der 1-USB-Originalversion gilt diese Konfiguration sinngemäss ebenso (ILI9341).
// ##################################################################################

// ###### Treiber ######
#define ILI9341_DRIVER        // klassischer ILI9341-Treiber, wie in der Hersteller-Referenz
#define TFT_WIDTH  240
#define TFT_HEIGHT 320

// ###### Farbreihenfolge (MADCTL 0x08 im Hersteller-Code = BGR) ######
#define TFT_RGB_ORDER TFT_BGR
// KEINE TFT_INVERSION — das Panel läuft im Normalmodus (0x20)

// ###### Hintergrundbeleuchtung ######
#define TFT_BL   21
#define TFT_BACKLIGHT_ON HIGH

// ###### Pins (schaltplanbestätigt: P2-Stecker, 05.11.2026) ######
#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1   // Reset-Leitung existiert auf dieser Revision nicht am ESP32 (EN-Reset beim Einschalten genügt)
#define TOUCH_CS 33   // Touch (XPT2046) hängt auf eigenen SPI-Pins (SCK 25, MOSI 32, MISO 39, IRQ 36);
                      // für Touch-Sketches die XPT2046-Bibliothek mit eigener SPI-Instanz verwenden.

// ###### Fonts ######
#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF
#define SMOOTH_FONT

// ###### SPI-Takt: 30 MHz wie in der Hersteller-Referenz ######
#define SPI_FREQUENCY       30000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY  2500000
