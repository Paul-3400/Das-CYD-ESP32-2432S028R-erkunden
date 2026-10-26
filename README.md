# Das CYD (ESP32-2432S028R) erkunden

Dokumentation und Lernprojekt rund um den **Cheap Yellow Display (CYD)** – das ESP32-Entwicklungsboard mit eingebautem Touchscreen, das in der Maker-Community enorm populär ist.

> **Autor:** Paul Simmen mit Vibe (GLM-5-latest-short), 26.10.2026

---

## Was ist der CYD?

Der **Cheap Yellow Display** ist der Community-Spitzname für ESP32-Boards mit aufgelötetem Farb-Touchdisplay. Der Klassiker und Bezugspunkt fast aller Tutorials ist das **ESP32-2432S028R**: ein ESP32-WROOM-32 mit 2,8-Zoll-TFT auf einer Platine – fertig verdrahtet, ohne Lötarbeit. Der Spitzname stammt vom gelben PCB; der Hersteller selbst verwendet ihn nicht.

Zentrale Anlaufstelle der Community ist das GitHub-Repo [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) mit Pinouts, Beispiel-Sketchen und Dutzenden fertigen Projekten.

---

## Fallstricke beim «Kauf ins Blaue»

Wer bei AliExpress, Amazon & Co. einfach das erstbeste «ESP32 Display 2.8 inch» bestellt, kann Pech haben. Die grössten Stolpersteine:

### 1. Gleicher Look, andere Hardware – die Varianten-Falle

Unter der Bezeichnung «CYD» werden mehrere, **nicht kompatible** Boards verkauft:

| Variante | Erkennungsmerkmal | Unterschied |
|---|---|---|
| **ESP32-2432S028R (1× USB)** | Ein Micro-USB-Anschluss | Originalversion: ILI9341, Standard-Pinout – die meisten Tutorials meinen dieses Board |
| **CYD2USB (2× USB)** | Zwei USB-Buchsen | Anderer Display-Controller (ST7789), **Farben vertauscht**, andere Konfiguration nötig |
| Kapazitiver Touch | Angaben wie «CST816» im Listing | Anderer Touch-Chip (I2C statt XPT2046/SPI) – andere Bibliothek nötig |
| Andere Grössen/Chips | 2,4″, 3,5″, ESP32-S3 … | Abweichende Pinouts und Treiber |

**Wichtig:** Unter derselben Modellnummer werden teils **zwei Revisionen** ausgeliefert. Das Listing-Foto genau ansehen – die USB-Anzahl ist das schnellste Unterscheidungsmerkmal.

### 2. Irreführende Listings

- Modellnummer wird oft nur unvollständig angegeben («ESP32-2432S028» statt «…S028**R**»)
- «Touch» wird versprochen, ohne zu sagen ob **resistiv** (Druck, Stift) oder **kapazitiv** (Finger) – die Originalversion ist resistiv
- Bezeichnungen wie «ESP32 Display 2.8 inch» oder «ESP32 Touchscreen LCD» verbergen die tatsächliche Variante
- Der Hersteller liefert kaum Dokumentation mit – die Doku kommt aus der Community, nicht vom Verkäufer

### 3. Was man beim Kauf prüfen sollte (Checkliste)

- [ ] Modellnummer **ESP32-2432S028R** explizit im Listing
- [ ] Treiberchip **ILI9341** angegeben
- [ ] **Ein** USB-Anschluss auf den Fotos (nicht zwei)
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

Das Board belegt die ESP32-GPIOs **vor** dem ersten eigenen Programmcode fest:

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

### Software-Stack

| Ebene | Werkzeug |
|---|---|
| Arduino IDE | **TFT_eSPI** (Bodmer) für Display, **XPT2046_Touchscreen** (Paul Stoffregen) für Touch |
| MicroPython | läuft auf dem Board; Einrichtung in unter 30 Minuten machbar |
| GUI-Frameworks | LVGL (mit TFT_eSPI-Konfiguration) |
| Smart Home | ESPHome / Home-Assistant-Dashboards (fertige Community-Projekte) |

**Konfigurations-Hinweis:** TFT_eSPI erfordert eine "User_Setup.h" mit den obigen Pins. Die falsche Konfiguration (z. B. für die 2-USB-Variante) führt zu invertierten Farben oder schwarzem Bild.

---

## Quellen und weiterführende Links

- [witnessmenow/ESP32-Cheap-Yellow-Display](https://github.com/witnessmenow/ESP32-Cheap-Yellow-Display) – Community-Hub mit Pinouts und Beispielen
- [esp32.diy – CYD Complete Build Guide](https://esp32.diy/esp32-cheap-yellow-display/)
- [cheapyellowdisplay.co.uk – About the CYD](https://cheapyellowdisplay.co.uk/about-cyd)
- [Proje Defteri – CYD Pinout and Setup Guide](https://projedefteri.com/en/blog/esp32-cheap-yellow-display-cyd-pinout-guide/)

---

## Projektziele / nächste Schritte

- [ ] Board erhalten, Varianten-Check anhand der eigenen Hardware (USB-Anzahl, Touch-Test)
- [ ] Entwicklungsumgebung einrichten (MicroPython / Arduino IDE)
- [ ] Erste Sketche: Display-Test, Touch-Kalibrierung
- [ ] Konkrete Projekte: Statusdisplay, Sensor-Dashboard
