# Das CYD (ESP32-2432S028R) erkunden

Dokumentation und Lernprojekt rund um den **Cheap Yellow Display (CYD)** – das ESP32-Entwicklungsboard mit eingebautem Touchscreen, das in der Maker-Community enorm populär ist.

> **Autor:** Paul Simmen mit Vibe (GLM-5-latest-short), 26.10.2026

---

## Was ist der CYD?

Der **Cheap Yellow Display** ist der Community-Spitzname für ESP32-Boards mit aufgelötetem Farb-Touchdisplay. Der Klassiker und Bezugspunkt fast aller Tutorials ist das **ESP32-2432S028R**: ein ESP32-WROOM-32 mit 2,8-Zoll-TFT auf einer Platine – fertig verdrahtet, ohne Lötarbeit. Der Spitzname stammt vom gelben PCB; der Hersteller selbst verwendet ihn nicht.

Zentrale Anlaufstelle der Community ist das GitHub-Repo [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) mit Pinouts, Beispiel-Sketchen und Dutzenden fertigen Projekten.

---

## Live-Beispiel: Varianten-Check unseres Boards (05.11.2026)

Genau die oben beschriebene Falle ist beim eigenen Kauf passiert – ein reales Beispiel, warum der Check wichtig ist:

**Bestellt** (AliExpress, Listing = Originalversion): ESP32-2432S028R mit ILI9341, 1× Micro-USB – so beschrieben es Listing und Datenblatt.

**Geliefert:** die **2-USB-Variante (CYD2USB)** mit **ST7789-Controller**. Der Varianten-Check an der Hardware:

| Prüfpunkt | Ergebnis |
|---|---|
| USB-Buchsen | **2** → CYD2USB-Variante, nicht die 1-USB-Originalversion |
| Werks-Demo | läuft (WLAN-Abfrage, Hinweis auf fehlende SD-Card, Menüs Controls/Visuals/Selectors) → Board funktionsfähig |
| Beilagen | USB-Kabel, Touch-Pen, 4-Pin-Anschlusskabel, Acrylgehäuse – vollständig |

**Konsequenz:** Kein Grund zur Reklamation – die 2-USB-Variante ist genauso brauchbar. Pinout und Touch (XPT2046, resistiv) sind identisch; nur die Display-Konfiguration weicht ab (siehe unten). Beim Flashen: Wenn der Computer das Board nicht erkennt, den jeweils anderen USB-Port probieren.

---

## Fallstricke beim «Kauf ins Blaue»

Wer bei AliExpress, Amazon & Co. einfach das erstbeste «ESP32 Display 2.8 inch» bestellt, kann Pech haben. Die grössten Stolpersteine:

### 1. Gleicher Look, andere Hardware – die Varianten-Falle

Unter der Bezeichnung «CYD» werden mehrere, **nicht kompatible** Boards verkauft:

| Variante | Erkennungsmerkmal | Unterschied |
|---|---|---|
| **ESP32-2432S028R (1× USB)** | Ein Micro-USB-Anschluss | Originalversion: ILI9341, Standard-Pinout – die meisten Tutorials meinen dieses Board |
| **CYD2USB (2× USB)** | Zwei USB-Buchsen | Anderer Display-Controller (ST7789), **Farben vertauscht/invertiert**, andere Konfiguration nötig |
| Kapazitiver Touch | Angaben wie «CST816» im Listing | Anderer Touch-Chip (I2C statt XPT2046/SPI) – andere Bibliothek nötig |
| Andere Grössen/Chips | 2,4″, 3,5″, ESP32-S3 … | Abweichende Pinouts und Treiber |

**Wichtig:** Unter derselben Modellnummer werden teils **zwei Revisionen** ausgeliefert – wie unser Live-Beispiel oben zeigt. Das Listing-Foto genau ansehen – die USB-Anzahl ist das schnellste Unterscheidungsmerkmal.

### 2. Irreführende Listings

- Modellnummer wird oft nur unvollständig angegeben («ESP32-2432S028» statt «…S028**R**»)
- «Touch» wird versprochen, ohne zu sagen ob **resistiv** (Druck, Stift) oder **kapazitiv** (Finger) – die Originalversion ist resistiv
- Bezeichnungen wie «ESP32 Display 2.8 inch» oder «ESP32 Touchscreen LCD» verbergen die tatsächliche Variante
- Der Hersteller liefert kaum Dokumentation mit – die Doku kommt aus der Community, nicht vom Verkäufer

### 3. Was man beim Kauf prüfen sollte (Checkliste)

- [ ] Modellnummer **ESP32-2432S028R** explizit im Listing
- [ ] Treiberchip **ILI9341** angegeben
- [ ] **Ein** USB-Anschluss auf den Fotos (nicht zwei) – sonst 2-USB-Variante
- [ ] Touch-Art: resistiv (Standard) oder kapazitiv (andere Software!)
- [ ] Zubehör im Auge behalten: USB-Kabel, Touch-Pen, Gehäuse – oft separat oder gar nicht im Bundle
- [ ] USB-Seriell-Wandler (meist CH340C) → unter Umständen ist ein Treiber nötig
- [ ] Bewertungen des Verkäufers lesen, insbesondere Fotos in Kundenrezensionen

### 4. Erwartungs-Management

- Der Bildschirm belegt rund **20 GPIOs** des ESP32. Als universelles Dev-Board mit vielen Sensoren taugt das Board kaum. Faustregel: **Kaufen, wenn der Bildschirm das Projekt ist – nicht, wenn er ein Teil davon ist.**
- Resistiver Touch reagiert auf Druck, nicht auf Berührung wie ein Smartphone; die 240×320-Auflösung ist bescheiden.
- 5-V-Betrieb über USB; für Batteriebetrieb braucht es zusätzliche Überlegungen (z. B. Laderegler).

---

## Technische Details der Originalversion (ESP32-2432S028R, 1× USB)

### Übersicht

| Merkmal | Wert |
|---|---|
| Controller | ESP32-WROOM-32, Dual-Core, bis 240 MHz |
| Speicher | 520 KB SRAM, 448 KB ROM, 4 MB Flash |
| Konnektivität | Wi-Fi + Bluetooth |
| Display | 2,8″ TFT, 240×320 Pixel, 16-Bit RGB65K |
| Display-Controller | **ILI9341** (SPI, HSPI-Bus) |
| Touchscreen | resistiv, **XPT2046** (SPI) |
| Backlight | gesteuerter Steuerkreis, GPIO 21 |
| Speicher-Erweiterung | microSD/TF-Kartenslot (VSPI-Bus) |
| Weitere On-Board-Schaltkreise | RGB-LED, fotoempfindlicher Sensor (LDR), Lautsprecher-Treiber, DHT11-Schnittstelle |
| Betriebsspannung | 5 V (USB), ca. 90 mA Stromverbrauch |
| Betriebstemperatur | −20 °C bis +70 °C |
| Modulgrösse | ca. 50 × 86 mm |
| Gewicht | ca. 50 g |

*Quelle: Hersteller-Datenblatt (AliExpress-Listing), ergänzt um Community-Dokumentation.*

### GPIO-Belegung (Originalversion, 1× USB)

Das Board belegt die ESP32-GPIOs **vor** dem ersten eigenen Programmcode fest. Die 2-USB-Variante nutzt dieselbe GPIO-Zuordnung (Display, Touch, SD, RGB-LED, LDR, Lautsprecher):

| Funktion | Pins |
|---|---|
| Display (HSPI) | CS: GPIO 15, DC: GPIO 2, MOSI: GPIO 13, MISO: GPIO 12, SCLK: GPIO 14, Backlight: GPIO 21 |
| Touch (XPT2046, SPI) | CS: GPIO 33, MOSI: GPIO 32, MISO: GPIO 39, CLK: GPIO 25, IRQ: GPIO 36 |
| microSD (VSPI) | CS: GPIO 5, MOSI: GPIO 23, MISO: GPIO 19, SCK: GPIO 18 |
| RGB-LED | Rot: GPIO 4, Grün: GPIO 16, Blau: GPIO 17 |
| Lichtsensor (LDR) | GPIO 34 |
| Lautsprecher | GPIO 26 |
| Freie/herausgeführte Pins | GPIO 35 (nur Input), GPIO 22, GPIO 27, GPIO 1/3 (auf Stiftleiste P1) |

*Pinout gemäss Community-Repo [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display).*

### Abweichungen der 2-USB-Variante (CYD2USB, wie unserem Board)

| Merkmal | 1× USB (Original) | 2× USB (CYD2USB) |
|---|---|---|
| Display-Controller | ILI9341 | **ST7789** |
| TFT_eSPI-Treiber | ILI9341_2_DRIVER | **ST7789_DRIVER** |
| Farb-/Inversions-Settings | keine Inversion | **TFT_RGB_ORDER = BGR**, **TFT_INVERSION_ON** |
| GPIO-Pinout | identisch | identisch |
| Touch | XPT2046, resistiv | XPT2046, resistiv (identische Pins) |

Wer die ILI9341-Konfiguration auf die 2-USB-Variante flasht, erhält ein Weissbild oder invertierte/verwaschene Farben – umgekehrt genauso. Erst den Treiber anpassen, dann weiter debuggen ([displayengineer.com](https://www.displayengineer.com/compatible/esp32-2432s028r-cyd)).

### Software-Stack

| Ebene | Werkzeug |
|---|---|
| Arduino IDE | **TFT_eSPI** (Bodmer) für Display, **XPT2046_Touchscreen** (Paul Stoffregen) für Touch |
| MicroPython | läuft auf dem Board; Einrichtung in unter 30 Minuten machbar |
| GUI-Frameworks | LVGL (mit TFT_eSPI-Konfiguration) |
| Smart Home | ESPHome / Home-Assistant-Dashboards (fertige Community-Projekte) |

**Konfigurations-Hinweis:** TFT_eSPI erfordert eine "User_Setup.h" mit den obigen Pins und – bei der 2-USB-Variante – dem ST7789-Treiber samt Inversion/BGR (siehe Tabelle oben).

### Zum 4-Pin-Anschlusskabel

Das mitgelieferte 4-farbige Kabel passt in den kleinen 4-Pin-Stecker (1,25 mm Raster) am Platinenrand. Der Hersteller nennt die Schnittstelle «Temperatur-/Feuchtigkeitssensor-Schnittstelle» (gedacht für einen DHT11); die Belegung laut Schaltplan (P1): Pin 1 = 5 V (rot), Pin 2 = TX (weiss), Pin 3 = RX (grün), Pin 4 = GND (schwarz). Damit lässt sich das Board auch ohne USB speisen, extern flashen oder ein Sensor anschliessen ([element14-Guide](https://community.element14.com/technologies/embedded/b/blog/posts/esp32-cheap-yellow-display-cyd-guide-with-a-jellyfish-example), [esp32-chess-Doku](https://github.com/valleyco/esp32-chess/blob/master/docs/boards/esp32-2432s028r/connectors.md)).

---

## Hersteller-Demo-Paket (Google Drive «CYD_Handbuch»)

Die komplette Hersteller-Ablage (JCZN/Shenzhen Jingcai Intelligent) liegt im Google Drive im Ordner **CYD_Handbuch → 2.8inch_ESP32-2432S028R**:

| Ordner | Inhalt | Nutzen für uns |
|---|---|---|
| 1-Demo | Demo_Arduino (30+ Beispiele: TFT_Rainbow, LVGL, WLAN-Server, DHT11, MP3, Buzzer …), Demo_LVGL, libraries (TFT_eSPI, LVGL, LovyanGFX, DHT u. a.) | Beispiel-Code inkl. «TFT_eSPI bottom layer replacement file» mit fertiger User_Setup.h |
| 2-Specification | Specifications-EN | Datenblatt (beschreibt die ILI9341-Revision) |
| 3-Structure_Diagram | Dimensions.png | Mechanische Masse |
| 4-Driver_IC_Data_Sheet | JC2432A028N (LCM, ILI9341V), ESP32-WROOM-32, FM8002A (Audio), 25Q32 (Flash), DHT11 | Chip-Datenblätter |
| 5-Schematic | MCU-/LCM-Schaltplan, WROOM-Pindefinition | Schaltpläne |
| 6-User_Manual | Getting started 2.8 Inch.pdf | Arduino-Einstieg (Tutorial zur User_Setup.h) |
| 7-Character&Picture_Molding_Tool | CH340-Treiber, JPGCompact, WinHex, Serial-Tools | PC-Werkzeuge |
| 8-Burn operation | LVGL-DEMO2.8R.bin (Werks-Demo), flash_download_tool 3.9.3, Anleitung | Wiederherstellung der Werksfirmware |

### Zentraler Befund: Hersteller-User_Setup.h muss angepasst werden

Die mitgelieferte **User_Setup.h** (TFT_eSPI bottom layer replacement file) konfiguriert **ILI9341_2_DRIVER ohne Inversion** – der Hersteller liefert sie also für die 1-USB-Originalversion aus (konsistent mit Datenblatt und Panel-Spec JC2432A028N von 2022, das ILI9341V nennt). Unsere 2-USB-Variante mit ST7789 braucht folgende Anpassungen in User_Setup.h:

1. "#define ILI9341_2_DRIVER" auskommentieren, stattdessen "**#define ST7789_DRIVER**" aktivieren
2. "**#define TFT_RGB_ORDER TFT_BGR**" einkommentieren
3. "**#define TFT_INVERSION_ON**" einkommentieren
4. Pins unverändert lassen: MISO 12, MOSI 13, SCLK 14, CS 15, DC 2, RST −1, BL 21, TOUCH_CS 33
5. TFT_WIDTH 240 / TFT_HEIGHT 320 bleiben

Zeigt der Rainbow-Test danach weiterhin falsche Farben, einmal TFT_RGB statt TFT_BGR probieren (bei einzelnen ST7789-Panels abweichend). Danach ist die ganze Demo-Sammlung des Herstellers nutzbar.

---

## Quellen und weiterführende Links

- [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) – Community-Hub mit Pinouts und Beispielen
- [esp32.diy – CYD Complete Build Guide](https://esp32.diy/esp32-cheap-yellow-display/)
- [cheapyellowdisplay.co.uk – About the CYD](https://cheapyellowdisplay.co.uk/about-cyd)
- [Proje Defteri – CYD Pinout and Setup Guide](https://projedefteri.com/en/blog/esp32-cheap-yellow-display-cyd-pinout-guide/)
- [displayengineer.com – ILI9341 vs. ST7789 Revisionen](https://www.displayengineer.com/compatible/esp32-2432s028r-cyd)
- [ralsina/esposito – CYD2USB Hardware-Guide](https://github.com/ralsina/esposito/blob/main/CYD2USB.md)
- Hersteller-Netdisk (Beipackzettel): [pan.jczn1688.com/1](https://pan.jczn1688.com/1#/) – gespiegelt im eigenen Google Drive «CYD_Handbuch»

---

## Projektziele / nächste Schritte

- [x] Board erhalten, Varianten-Check anhand der eigenen Hardware → **2-USB-Variante (CYD2USB, ST7789)** festgestellt (05.11.2026)
- [x] Hersteller-Demo-Paket in Google Drive «CYD_Handbuch» gesichtet; User_Setup.h-Anpassung für ST7789 dokumentiert (05.11.2026)
- [ ] Entwicklungsumgebung einrichten (Arduino IDE, TFT_eSPI mit ST7789-Settings)
- [ ] Erste Sketche: Display-Test (Farbtreue mit Inversion/BGR prüfen), Touch-Kalibrierung
- [ ] Konkrete Projekte: Statusdisplay, Sensor-Dashboard
