# Die Versuchs-Sketches: Funktion, Installation und Bibliotheken

> **Autor:** Paul Simmen mit Vibe (GLM-5-latest-short), 05.11.2026

Diese Anleitung beschreibt die Arduino-Dateien dieses Repos: was jeder Sketch tut,
welche Bibliotheken er braucht und wie man ihn Schritt für Schritt installiert.
Sie ist die Ergänzung zur [README.md](README.md), die Hardware, Kauf-Fallstricke
und Debugging-Chronik abdeckt.

**Zielgerät:** CYD ESP32-2432S028R, **2-USB-Variante von JCZN** (ILI9341-Display,
resistiver Touch XPT2046). Beide Tests wurden auf genau diesem Board am
**05.11.2026** bestanden.

---

## Übersicht

| Datei | Zweck | Status |
|---|---|---|
| [arduino/User_Setup.h](arduino/User_Setup.h) | TFT_eSPI-Konfiguration (Treiber, Pins) – Grundlage für beide Sketches | ✅ getestet |
| [arduino/CYD_DisplayTest/CYD_DisplayTest.ino](arduino/CYD_DisplayTest/CYD_DisplayTest.ino) | Display-Test: 6 Farbstreifen + Text, blinkende rote LED | ✅ bestanden |
| [arduino/CYD_TouchTest/CYD_TouchTest.ino](arduino/CYD_TouchTest/CYD_TouchTest.ino) | Touch-Test: Mal-Modus + geführte 4-Punkte-Kalibrierung | ✅ bestanden |

**Reihenfolge:** zuerst die User_Setup.h installieren (Abschnitt 2), dann
CYD_DisplayTest, dann CYD_TouchTest. Die User_Setup.h ist für beide Sketches
identisch.

---

## 1. Voraussetzungen (einmalig)

### 1.1 Arduino IDE und ESP32-Board-Support

- Arduino IDE 2.x installieren
- **Datei → Einstellungen → Zusätzliche Boardverwalter-URLs:**
  `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
- Boardverwalter: **«esp32 by Espressif Systems»** installieren
- **Werkzeuge → Board → ESP32 Arduino → ESP32 Dev Module** wählen
- Serial Monitor auf **115200 Baud** stellen (beide Sketches)

### 1.2 USB-Treiber (CH340C)

Das Board nutzt den USB-Seriell-Wandler CH340C. macOS erkennt ihn meist ohne
weiteres; unter Windows installiert man notfalls den CH340-Treiber (liegt auch
im Hersteller-Paket, Ordner 7-Character&Picture_Molding_Tool, siehe README).
Zeigt die IDE keinen Port, hilft oft ein anderes USB-Kabel (mit Datenleitung!)
oder der zweite USB-Anschluss des Boards.

### 1.3 Diese Dateien besorgen

Repo klonen (`git clone https://github.com/Paul-3400/Das-CYD-ESP32-2432S028R-erkunden`)
oder als ZIP herunterladen und entpacken.

---

## 2. Notwendige Bibliotheken

| Bibliothek | Autor | Wofür | Konfiguration nötig? |
|---|---|---|---|
| **TFT_eSPI** | Bodmer | Display-Ansteuerung (ILI9341) | **JA** – User_Setup.h ersetzen (2.1) |
| **XPT2046_Touchscreen** | Paul Stoffregen | Touch-Erfassung | Nein – alle Pins stehen im Sketch (2.2) |

Beide über den **Bibliotheksverwalter** installieren (Arduino IDE: Sketch →
Bibliothek einbinden → Bibliotheken verwalten, Name suchen, «Installieren»).

### 2.1 TFT_eSPI: User_Setup.h ersetzen (der entscheidende Schritt!)

TFT_eSPI weiss nach der Installation **nicht**, an welchen Pins unser Display
hängt und welchen Treiber es braucht. Und – Achtung – **die mitgelieferte
Arduino-User_Setup.h des Herstellers ist fehlerhaft** (setzt TFT_RST 12; GPIO 12
ist aber die Panel-Datenleitung SDO). Niemals die Hersteller-Datei verwenden!

Deshalb die **getestete** [arduino/User_Setup.h](arduino/User_Setup.h) aus
diesem Repo über die Datei gleichen Namens im Bibliotheksordner kopieren:

- **macOS:** `~/Dokumente/Arduino/libraries/TFT_eSPI/User_Setup.h` überschreiben
- **Windows:** `Dokumente\Arduino\libraries\TFT_eSPI\User_Setup.h` überschreiben

Inhalt in Kurzform (Kommentare in der Datei erklären jede Zeile):

| Einstellung | Wert | Bedeutung |
|---|---|---|
| Treiber | `ILI9341_DRIVER` | NICHT ST7789 (Faustregel «2 USB = ST7789» gilt für dieses Board nicht) |
| Auflösung | 240 × 320 | |
| Farbreihenfolge | `TFT_RGB_ORDER TFT_BGR` | sonst vertauschte Farben |
| Inversion | **aus** | kein `TFT_INVERSION_ON` |
| Reset | `TFT_RST -1` | Panel-Reset hängt an keinem GPIO |
| Display-Pins | MISO 12, MOSI 13, SCLK 14, CS 15, DC 2 | ⚠️ GPIO 12 = SDO, niemals als Reset nutzen |
| Backlight | GPIO 21 | |
| Touch-CS | GPIO 33 | Information; Touch läuft über eigene Bibliothek |
| SPI-Takt | 30 MHz | stabil getestet |

### 2.2 XPT2046_Touchscreen: nur installieren

Keine Konfiguration nötig – der Sketch CYD_TouchTest setzt alle Pins selbst.
Der Touch-Controller hängt an **eigenen SPI-Pins** (SCK 25, MOSI 32, MISO 39,
CS 33, IRQ 36) und wird im Sketch über den **zweiten SPI-Baustein (HSPI)**
angesprochen, unabhängig vom Display-SPI. Genau deshalb funktionieren die
TFT_eSPI-eigenen Touchfunktionen nicht – wir brauchen diese Bibliothek.

---

## 3. CYD_DisplayTest – der Display-Test

### Funktion

- Zeichnet **sechs Farbstreifen** (Rot, Grün, Blau, Gelb, Magenta, Cyan) in
  Portrait-Ansicht (240 × 320)
- Darunter eine schwarze Zone mit dem Text «CYD Display-Test» / «ILI9341 2USB OK»
- In der Endlosschleife blinkt der **rote Kanal der RGB-LED** (GPIO 4, aktiv
  LOW) im Sekundentakt – beweist auch bei leerem Display, dass der Sketch läuft

### Was der Test beweist

Der Sketch ist minimal (nur TFT_eSPI, keine Konfiguration im Sketch selbst) –
er zeigt ausschliesslich die Wirkung der User_Setup.h: Treiberwahl,
Farbreihenfolge, Pinbelegung und SPI-Takt stimmen.

### Erwartetes Ergebnis

- Sechs Streifen von oben in genau dieser Reihenfolge: Rot, Grün, Blau, Gelb,
  Magenta, Cyan – in sattem, klarem Farbton (keine invertierten/vertauschten Farben)
- Text lesbar in weiss auf schwarz
- Rote LED blinkt im Sekundentakt

**Bestanden (05.11.2026)** – dieser Test hat die finale Konfiguration bestätigt
(nach der in der README beschriebenen Debugging-Odyssee).

---

## 4. CYD_TouchTest – der Touch-Test

### Funktion

Landscape-Ansicht (320 × 240) mit drei Zonen:

| Zone | Funktion |
|---|---|
| Statusleiste (oben) | zeigt live die Bildschirm- und RAW-Koordinaten der Fingerposition |
| Malfläche (Mitte, schwarz) | Finger/Pen zeichnet Linien in der gewählten Farbe |
| Farbleiste (unten) | 5 Farbfelder (Stiftfarbe wählen, Auswahl mit orangem Rahmen) + CLR (Fläche leeren) |

Dazu die **geführte Kalibrierung** (Serial Monitor, Taste `c`): vier Kreuze
nacheinander anfassen; der Sketch nimmt pro Kreuz 9 Messungen (Median gegen
Ausreisser), rechnet die Kalibrierwerte aus, prüft sie am vierten Punkt und
druckt fertige `#define`-Zeilen zum Übernehmen in den Sketch.

### Bedienung

| Aktion | Wirkung |
|---|---|
| Finger/Pen auf Malfläche | zeichnen – der Strich sollte **genau unter der Fingerspitze** liegen |
| Farbfeld unten antippen | Stiftfarbe wechseln |
| CLR antippen | Malfläche leeren |
| Serial Monitor: `c` | Kalibrierung starten |
| Serial Monitor: `x` | Malfläche leeren |

### Erwartetes Ergebnis

- Malen funktioniert sofort mit den Startwerten (Mittelwerte aus Tutorials)
- Erscheinen die Striche **versetzt oder gespiegelt**: Kalibrierung `c`
  ausführen und die vier vorgeschlagenen `#define TS_…`-Zeilen in den Sketch
  kopieren, neu hochladen – fertig ist die individuelle Kalibrierung des
  konkreten Panels
- Die Prüfpunkt-Abweichung nach der Kalibrierung sollte im einstelligen
  Pixelbereich liegen (bei resistivem Touch sind wenige px normal)

**Bestanden (05.11.2026)** – Touch läuft am eigenen SPI-Bus (HSPI) ohne
Konflikt mit dem Display.

---

## 5. Troubleshooting

| Symptom | Wahrscheinliche Ursache | Lösung |
|---|---|---|
| Bildschirm bleibt blank (Backlight an) | falsche/nicht ersetzte User_Setup.h | getestete [arduino/User_Setup.h](arduino/User_Setup.h) wirklich im Bibliotheksordner? (Nach Bibliotheks-Updates wieder überschreiben!) |
| Farben vertauscht (Rot/Blau) | RGB- statt BGR-Reihenfolge | `TFT_RGB_ORDER TFT_BGR` in User_Setup.h |
| Farben invertiert (wie Negativ) | Inversion an | Inversion deaktivieren (`TFT_INVERSION_ON` entfernen) |
| Bildschirm blank, obwohl User_Setup stimmt | Hersteller-User_Setup verwendet (TFT_RST 12) | Hersteller-Datei verwerfen, Repo-Version einsetzen |
| Touch reagiert gar nicht | SPI-Baustein-Zuordnung | im Sketch `SPIClass(HSPI)` durch `SPIClass(VSPI)` ersetzen |
| Striche versetzt oder gespiegelt | Standard-Kalibrierwerte passen nicht | Kalibrierung mit `c` ausführen, Werte übernehmen |
| Serial Monitor zeigt nichts | falsche Baudrate oder Port | 115200 Baud; richtigen Port/USB-Anschluss wählen |

---

## 6. Nächste Schritte

Mit beiden bestandenen Tests ist die Grundlage komplett. Nun können die
Projektideen aus der README angegangen werden: Statusdisplay (z. B. für eine
MeshCore-Station), Sensor-Dashboard (INA226-Werte gross ablesen) oder ein
Touch-Bedienpanel. Die persönlichen Kalibrierwerte aus CYD_TouchTest gehören in
jeden künftigen Touch-Sketch kopiert.
