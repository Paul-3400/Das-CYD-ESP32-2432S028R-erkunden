# Das CYD (ESP32-2432S028R) erkunden

Dokumentation und Lernprojekt rund um den **Cheap Yellow Display (CYD)** – das ESP32-Entwicklungsboard mit eingebautem Touchscreen, das in der Maker-Community enorm populär ist.

> **Autor:** Paul Simmen mit Vibe (GLM-5-latest-short), 26.10.2026

---

## Was ist der CYD?

Der **Cheap Yellow Display** ist der Community-Spitzname für ESP32-Boards mit aufgelötetem Farb-Touchdisplay. Der Klassiker und Bezugspunkt fast aller Tutorials ist das **ESP32-2432S028R**: ein ESP32-WROOM-32 mit 2,8-Zoll-TFT auf einer Platine – fertig verdrahtet, ohne Lötarbeit. Der Spitzname stammt vom gelben PCB; der Hersteller selbst verwendet ihn nicht.

Zentrale Anlaufstelle der Community ist das GitHub-Repo [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) mit Pinouts, Beispiel-Sketchen und Dutzenden fertigen Projekten.

---

## Unser Board (geliefert 05.11.2026) – das Wichtigste zuerst

Bestellt als ESP32-2432S028R (Listing/Datenblatt beschrieben die 1-USB-Originalversion), geliefert wurde die **2-USB-Variante von JCZN** (Shenzhen Jingcai Intelligent). Nach erfolgreichem Debugging steht fest:

| Merkmal | Wert |
|---|---|
| USB-Buchsen | 2 |
| **Display-Controller** | **ILI9341** – NICHT ST7789! (belegt durch Hersteller-IDF-Quellcode + erfolgreichen Test) |
| Touch | XPT2046, resistiv, eigene SPI-Pins (SCK 25, MOSI 32, MISO 39, CS 33, IRQ 36) |
| Reset des Panels | **nicht an einen GPIO gebunden** – TFT_RST = −1 |
| Lauffähige TFT_eSPI-Konfiguration | siehe [arduino/User_Setup.h](arduino/User_Setup.h) (ILI9341_DRIVER, BGR, keine Inversion, RST −1, 30 MHz) |
| Beilagen | USB-Kabel, Touch-Pen, 4-Pin-Kabel (5 V rot / TX weiss / RX grün / GND schwarz), Acrylgehäuse |

**⚠️ Zentrale Lehre:** Die Faustregel «2× USB = ST7789» gilt für dieses JCZN-Board **nicht**. Die USB-Anzahl ist nur ein Hinweis, kein Beweis. Die Hersteller-Unterlagen (Datenblatt, Guide, Arduino-User_Setup.h) sind teils **widersprüchlich und veraltet**; verlässliche Quelle war der **IDF-Quellcode** der Werksfirmware (lcd.c).

---

## Fallstricke beim «Kauf ins Blaue»

Wer bei AliExpress, Amazon & Co. einfach das erstbeste «ESP32 Display 2.8 inch» bestellt, kann Pech haben. Die grössten Stolpersteine:

### 1. Gleicher Look, andere Hardware – die Varianten-Falle

| Variante | Erkennungsmerkmal | Unterschied |
|---|---|---|
| **ESP32-2432S028R (1× USB)** | Ein Micro-USB-Anschluss | Originalversion: ILI9341, Standard-Pinout |
| **2×-USB-Varianten** | Zwei USB-Buchsen | **Nicht einheitlich!** Manche mit ST7789 (Farben invertiert/vertauscht) – unser JCZN-Board jedoch mit ILI9341 |
| Kapazitiver Touch | Angaben wie «CST816» im Listing | Anderer Touch-Chip (I2C statt XPT2046/SPI) – andere Bibliothek nötig |
| Andere Grössen/Chips | 2,4″, 3,5″, ESP32-S3 … | Abweichende Pinouts und Treiber |

**Wichtig:** Unter derselben Modellnummer werden teils **mehrere Revisionen** ausgeliefert – wie unser Fall zeigt. USB-Anzahl und Listing-Fotos sind nur Indizien; den Chip-Treiber verifiziert man per Testbild.

### 2. Irreführende Listings und unzuverlässige Hersteller-Doku

- Modellnummer wird oft nur unvollständig angegeben («ESP32-2432S028» statt «…S028**R**»)
- «Touch» ohne Angabe ob **resistiv** oder **kapazitiv**
- Der Hersteller liefert Dokumentation mit, die sich **selbst widerspricht** (unser Fall: Datenblatt ILI9341 ✓, Arduino-User_Setup.h veraltet/fehlerhaft mit TFT_RST 12 auf der SDO-Leitung, IDF-Quellcode korrekt)
- Faustregel: **Der Quellcode der Werksfirmware ist die vertrauenswürdigste Quelle**, nicht die PDFs

### 3. Was man beim Kauf prüfen sollte (Checkliste)

- [ ] Modellnummer **ESP32-2432S028R** explizit im Listing
- [ ] Treiberchip angegeben (ILI9341/ST7789) – und nach Lieferung per Testbild verifizieren
- [ ] USB-Anzahl beachten (Hinweis auf Revision)
- [ ] Touch-Art: resistiv (Standard) oder kapazitiv (andere Software!)
- [ ] Zubehör im Auge behalten: USB-Kabel, Touch-Pen, Gehäuse
- [ ] USB-Seriell-Wandler (CH340C) → evtl. ist ein Treiber nötig
- [ ] Bewertungen des Verkäufers lesen, insbesondere Fotos in Kundenrezensionen

### 4. Erwartungs-Management

- Der Bildschirm belegt rund **20 GPIOs** des ESP32. Als universelles Dev-Board mit vielen Sensoren taugt das Board kaum. Faustregel: **Kaufen, wenn der Bildschirm das Projekt ist – nicht, wenn er ein Teil davon ist.**
- Resistiver Touch reagiert auf Druck, nicht auf Berührung wie ein Smartphone; die 240×320-Auflösung ist bescheiden.
- 5-V-Betrieb über USB; für Batteriebetrieb braucht es zusätzliche Überlegungen (z. B. Laderegler).

---

## GPIO-Belegung (schaltplanbestätigt, P2-Stecker, 05.11.2026)

| Funktion | Pins |
|---|---|
| Display (TFT, SPI) | CS: GPIO 15, DC/RS: GPIO 2, MOSI/SDI: GPIO 13, MISO/SDO: GPIO 12, SCLK: GPIO 14, Backlight: GPIO 21 |
| Touch (XPT2046, eigener SPI) | CS: GPIO 33, MOSI: GPIO 32, MISO: GPIO 39, CLK: GPIO 25, IRQ: GPIO 36 |
| microSD (VSPI) | CS: GPIO 5, MOSI: GPIO 23, MISO: GPIO 19, SCK: GPIO 18 |
| RGB-LED | Rot: GPIO 4, Grün: GPIO 16, Blau: GPIO 17 (aktiv LOW) |
| Lichtsensor (LDR) | GPIO 34 |
| Lautsprecher | GPIO 26 |
| Freie/herausgeführte Pins | GPIO 35 (nur Input), GPIO 22, GPIO 27, GPIO 1/3 (auf Stiftleiste P1) |
| 4-Pin-Buchse P1 | 5 V (rot), TX (weiss), RX (grün), GND (schwarz) – DHT11-/Seriell-Schnittstelle |

*Pinout gemäss Community-Repo [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display), bestätigt am Hersteller-Schaltplan (P2, 18-polig, passt zum Panel-Datenblatt JC2432A028N).*

---

## Debugging-Chronik: vom blanken Bildschirm zum Testbild (05.11.2026)

Der Weg zur lauffähigen Konfiguration – als Lehrgeld-Dokumentation:

| # | Konfiguration | Ergebnis | Erkenntnis |
|---|---|---|---|
| 1 | ST7789_DRIVER, RST −1, BGR, Inversion an (Community-Faustregel «2 USB = ST7789») | blank, Backlight an | Treiberthese beginnt zu wanken |
| 2 | ST7789, RST 12, MISO aus (Hersteller-Arduino-Datei) | blank | RST-These entlastet |
| 3 | ILI9341_2, RST 12 (Hersteller-User_Setup.h 1:1) | blank | Hersteller-Datei selbst schuldig! |
| 4 | #error-Test in User_Setup.h | Compiler stolpert | Datei wird definitiv benutzt |
| 5 | Schaltplan-Zoom am Display-Stecker P2 | Pins 13/14/15/2/12/21 bestätigt; RST-Quelle unklar | Pins waren nie das Problem; GPIO 12 = SDO! |
| 6 | Hersteller-IDF-Quellcode (lcd.c/lcd.h der Werksfirmware) | **ILI9341-Init, RST unbenutzt (−1), BGR, keine Inversion, 30 MHz** | Die funktionierende Referenz |
| 7 | Genau diese Einstellungen in TFT_eSPI übertragen | ✅ **6 Farbstreifen + Text** | Fall gelöst |

**Drei Lehren:**
1. Community-Faustregeln («2 USB = ST7789») sind Statistik, kein Naturgesetz – **am eigenen Board verifizieren**.
2. Automatische Bild-/Schaltplan-Lesung (KI-Vision) ist bei feinen Netznamen unzuverlässig (las uns u. a. «GC9A01» und Flash-Netze als Display-Signale «lesen») – **menschliches Hineinzoomen schlägt sie**.
3. Bei widersprüchlichen Hersteller-Unterlagen ist der **Firmware-Quellcode** die höchste Instanz.

---

## Hersteller-Demo-Paket (Google Drive «CYD_Handbuch»)

Die komplette Hersteller-Ablage (JCZN) liegt im Google Drive im Ordner **CYD_Handbuch → 2.8inch_ESP32-2432S028R**:

| Ordner | Inhalt | Nutzen für uns |
|---|---|---|
| 1-Demo | Demo_Arduino (30+ Beispiele), Demo_LVGL(idf-4.3) – **dort liegt die funktionierende Referenz lcd.c/lcd.h** | Beispiel-Code; IDF-Quellcode als Pin-/Treiber-Referenz |
| 2-Specification | Specifications-EN | Datenblatt |
| 3-Structure_Diagram | Dimensions.png | Mechanische Masse |
| 4-Driver_IC_Data_Sheet | JC2432A028N (LCM, ILI9341V), ESP32-WROOM-32, FM8002A, 25Q32, DHT11 | Chip-Datenblätter |
| 5-Schematic | MCU-/LCM-Schaltplan, WROOM-Pindefinition | Schaltpläne (P2-Stecker!) |
| 6-User_Manual | Getting started 2.8 Inch.pdf | Arduino-Einstieg |
| 7-Character&Picture_Molding_Tool | CH340-Treiber, JPGCompact, WinHex, Serial-Tools | PC-Werkzeuge |
| 8-Burn operation | LVGL-DEMO2.8R.bin (Werks-Demo), flash_download_tool 3.9.3, Anleitung | Wiederherstellung der Werksfirmware |

**Achtung bei der mitgelieferten Arduino-User_Setup.h (TFT_eSPI bottom layer replacement file):** Sie steht auf ILI9341_2 mit **TFT_RST 12 – das ist fehlerhaft** (GPIO 12 ist laut Schaltplan die Panel-Datenleitung SDO; die Werksfirmware steuert nie einen Reset an). Stattdessen die angepasste [arduino/User_Setup.h](arduino/User_Setup.h) aus diesem Repo verwenden.

---

## Fertige Arduino-Dateien in diesem Repo

- **[arduino/User_Setup.h](arduino/User_Setup.h)** – **lauffähig getestete** TFT_eSPI-Konfiguration (05.11.2026): ILI9341_DRIVER, TFT_RGB_ORDER TFT_BGR, keine Inversion, TFT_RST −1, Pins MISO 12 / MOSI 13 / SCLK 14 / CS 15 / DC 2 / BL 21, TOUCH_CS 33, 30 MHz. Ersetzt die Datei gleichen Namens im TFT_eSPI-Bibliotheksordner (macOS: ~/Dokumente/Arduino/libraries/TFT_eSPI/).
- **[arduino/CYD_DisplayTest/CYD_DisplayTest.ino](arduino/CYD_DisplayTest/CYD_DisplayTest.ino)** – bestandener Farbtreue-Test: sechs Farbstreifen + Text; unten blinkt die rote Board-LED (GPIO 4, aktiv LOW).

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

- [x] Board erhalten, Varianten-Check → **2-USB-Variante (JCZN), ILI9341-Panel** (05.11.2026)
- [x] Hersteller-Demo-Paket gesichtet, Doku-Widersprüche identifiziert (05.11.2026)
- [x] Arduino-IDE eingerichtet, User_Setup.h gefunden und getestet → **Display-Test bestanden: 6 Farbstreifen + Text** (05.11.2026)
- [ ] Touch-Test und -Kalibrierung (XPT2046_Touchscreen-Bibliothek, eigene SPI-Pins SCK 25 / MOSI 32 / MISO 39 / CS 33 / IRQ 36)
- [ ] Konkrete Projekte: Statusdisplay (z. B. MeshCore-Station), Sensor-Dashboard (INA226), Touch-Bedienpanel
