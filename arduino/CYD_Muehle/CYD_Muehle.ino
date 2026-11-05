/*
  CYD_Muehle.ino - Muehlespiel (Nine Men's Morris) auf dem CYD
  fuer das CYD (Cheap Yellow Display) ESP32-2432S028R,
  2-USB-Variante von JCZN, ILI9341-Display, resistiver Touch XPT2046.

  Version 1.0: Hot-Seat (zwei Spieler am Geraet). KI-Gegner geplant.
  Spielregeln: Anleitung-Muehlespiel.pdf (5 Seiten, Upload vom 05.11.2026)
    - Setzphase: je 9 Steine abwechselnd setzen (Weiss beginnt, S. 1/2)
    - Zugphase:  zu freien Nachbarpunkten ziehen (S. 2)
    - Endphase:  ab 3 Steinen auf freie Punkte springen (S. 3)
    - Muehle:    3 Steine einer Farbe auf einer Geraden -> Gegner-Stein
                 entfernen, kein Stein aus bestehender Muehle, ausser es
                 gibt keinen anderen (Turnierregel, S. 4/5)
    - Doppel-Muehle in der Setzphase: trotzdem nur 1 Stein (S. 5)
    - Ende: <3 Steine (S. 5), kein legaler Zug (S. 5), 20 Zuege ohne
      Muehle = Remis (S. 5), dreimal gleiche Stellung = Remis (S. 5)

  Bedienung:
   - Statusleiste oben sagt immer, was zu tun ist
   - Setzen: freien Punkt antippen
   - Ziehen/Springen: eigenen Stein antippen (wird orange markiert,
     moegliche Ziele gruen), dann Ziel antippen; nochmal antippen = abwaehlen
   - Muehle: Gegner-Stein antippen (entfernbarer Bereich)
   - Rechts: noch zu setzende Steine; NEU = neue Partie
   - Serial Monitor (115200 Baud) protokolliert jeden Zug

  Voraussetzung: User_Setup.h + Bibliotheken wie in SKETCHE.md beschrieben.

  Autor: Paul Simmen mit Vibe (GLM-5-latest-short), 05.11.2026
*/

#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// ---------- Touch-Pins (XPT2046, separater SPI-Bus) ----------
#define XPT2046_IRQ  36
#define XPT2046_MOSI 32
#define XPT2046_MISO 39
#define XPT2046_CLK  25
#define XPT2046_CS   33

// ---------- Kalibrierwerte (bei Bedarf eigene Werte aus CYD_TouchTest eintragen) ----------
#define TS_X_AT_0    200
#define TS_X_AT_319  3700
#define TS_Y_AT_0    240
#define TS_Y_AT_239  3800

#define W 320
#define H 240

TFT_eSPI tft = TFT_eSPI();
SPIClass touchSPI = SPIClass(HSPI);
XPT2046_Touchscreen touch(XPT2046_CS, XPT2046_IRQ);

// ---------- Geometrie: 24 Punkte (Landscape 320x240) ----------
// Index: 0-7 aeusseres Quadrat (Start Ecke oben links, im Uhrzeigersinn),
//        8-15 mittleres Quadrat, 16-23 inneres Quadrat
const int PX[24] = {  62,150,238,238,238,150, 62, 62,
                      93,150,207,207,207,150, 93, 93,
                     124,150,176,176,176,150,124,124 };
const int PY[24] = {  43, 43, 43,131,219,219,219,131,
                      74, 74, 74,131,188,188,188,131,
                     105,105,105,131,157,157,157,131 };

// Nachbarn (fuer Zugphase), -1 = kein weiterer
const int NB[24][4] = {
  { 1, 7,-1,-1}, { 0, 2, 9,-1}, { 1, 3,-1,-1}, { 2, 4,11,-1},
  { 3, 5,-1,-1}, { 4, 6,13,-1}, { 5, 7,-1,-1}, { 0, 6,15,-1},
  { 9,15,-1,-1}, { 1, 8,10,17}, { 9,11,-1,-1}, { 3,10,12,19},
  {11,13,-1,-1}, { 5,12,14,21}, {13,15,-1,-1}, { 7, 8,14,23},
  {17,23,-1,-1}, { 9,16,18,-1}, {17,19,-1,-1}, {11,18,20,-1},
  {19,21,-1,-1}, {13,20,22,-1}, {21,23,-1,-1}, {15,16,22,-1}
};

// Die 16 Muehlen: 8 horizontale + 8 vertikale Reihen
const int MILLS[16][3] = {
  { 0, 1, 2}, { 8, 9,10}, {16,17,18}, { 7,15,23},   // horizontale Reihen
  { 3,11,19}, {20,21,22}, {12,13,14}, { 4, 5, 6},
  { 0, 7, 6}, { 8,15,14}, {16,23,22}, { 1, 9,17},   // vertikale Reihen
  { 5,13,21}, {18,19,20}, {10,11,12}, { 2, 3, 4}
};

// ---------- Spielzustand ----------
// brett: 0 = leer, 1 = Weiss, 2 = Schwarz
int brett[24];
int toPlace[2];   // noch zu setzende Steine (Start je 9)
int onBoard[2];   // Steine auf dem Brett
int cur = 0;      // aktueller Spieler: 0 = Weiss (beginnt), 1 = Schwarz
int sel = -1;     // gewaehlter Stein in der Zug-/Sprungphase
bool canJump = false;
int noMill = 0;   // Spielzuege ohne Muehle (Remis ab 20, Anleitung S. 5)
uint64_t hist[300];
int histN = 0;
int winner = -1;  // -1 laufend, 0/1 Spieler, 2 = Unentschieden
bool oppAllInMill = false;  // Turnierregel: Gegner-Steine alle in Muehlen

const char *NAME[2] = { "Weiss", "Schwarz" };
const uint16_t COL[2] = { TFT_WHITE, TFT_BLACK };

enum Mode { M_SETZEN, M_WAEHLEN, M_ZIEL, M_ENTFERNEN, M_ENDE };
Mode mode = M_SETZEN;

// ---------------------------------------------------------------
// Hilfsfunktionen (Spiellogik)
// ---------------------------------------------------------------

int mapRaw(long raw, float atMin, float atMax, int scrMax) {
  float v = (raw - atMin) * (float)scrMax / (atMax - atMin);
  int r = (int)v;
  if (r < 0) r = 0;
  if (r > scrMax) r = scrMax;
  return r;
}

bool isAdj(int a, int b) {
  for (int k = 0; k < 4; k++) if (NB[a][k] == b) return true;
  return false;
}

bool inMillPoint(int p) {  // Stein auf p liegt in irgendeiner vollstaendigen Muehle?
  if (brett[p] == 0) return false;
  for (int m = 0; m < 16; m++) {
    bool hat = false;
    for (int k = 0; k < 3; k++) if (MILLS[m][k] == p) hat = true;
    if (!hat) continue;
    if (brett[MILLS[m][0]] == brett[p] &&
        brett[MILLS[m][1]] == brett[p] &&
        brett[MILLS[m][2]] == brett[p]) return true;
  }
  return false;
}

int closedMillAt(int p) {  // durch Stein auf p neu geschlossene Muehle? (Index oder -1)
  for (int m = 0; m < 16; m++) {
    bool hat = false;
    for (int k = 0; k < 3; k++) if (MILLS[m][k] == p) hat = true;
    if (!hat) continue;
    if (brett[MILLS[m][0]] == brett[p] &&
        brett[MILLS[m][1]] == brett[p] &&
        brett[MILLS[m][2]] == brett[p]) return m;
  }
  return -1;
}

bool hasAnyMove(int pl) {  // legaler Zug in der Zugphase vorhanden?
  for (int p = 0; p < 24; p++) {
    if (brett[p] != pl + 1) continue;
    for (int k = 0; k < 4; k++) {
      int n = NB[p][k];
      if (n >= 0 && brett[n] == 0) return true;
    }
  }
  return false;
}

bool legalTarget(int f, int t) {
  if (brett[t] != 0) return false;
  return canJump || isAdj(f, t);  // Sprungphase: Anleitung S. 3
}

uint64_t hashPos() {  // Stellung + Spieler am Zug als eindeutige Zahl
  uint64_t h = 0;
  for (int i = 0; i < 24; i++) h = h * 3 + (uint64_t)brett[i];
  h = h * 2 + (uint64_t)cur;
  return h;
}

// ---------------------------------------------------------------
// Zeichnen
// ---------------------------------------------------------------

void drawPoint(int i) {
  int x = PX[i], y = PY[i];
  if (brett[i] == 0) {
    tft.fillCircle(x, y, 9, TFT_BLACK);        // alte Markierung loeschen
    tft.fillCircle(x, y, 3, TFT_LIGHTGREY);
  } else {
    tft.fillCircle(x, y, 9, COL[brett[i] - 1]);
    tft.drawCircle(x, y, 9, brett[i] == 1 ? TFT_BLACK : TFT_WHITE);
  }
}

void drawSelRing(int p) { tft.drawCircle(PX[p], PY[p], 13, TFT_ORANGE); }
void clearSelRing(int p) { tft.drawCircle(PX[p], PY[p], 13, TFT_BLACK); }

void showTargets() {
  for (int t = 0; t < 24; t++)
    if (legalTarget(sel, t)) tft.drawCircle(PX[t], PY[t], 5, TFT_GREEN);
}

void clearTargets() {
  for (int t = 0; t < 24; t++)
    if (legalTarget(sel, t)) drawPoint(t);
}

void updateStatus() {
  tft.fillRect(0, 0, W, 22, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  String s = "";
  switch (mode) {
    case M_SETZEN:    s = String(NAME[cur]) + " setzt (Rest " + toPlace[cur] + ")"; break;
    case M_WAEHLEN:   s = String(NAME[cur]) + (canJump ? ": Stein zum Springen" : ": Stein waehlen"); break;
    case M_ZIEL:      s = String(NAME[cur]) + ": Ziel antippen"; break;
    case M_ENTFERNEN: s = "Muehle! " + String(NAME[1 - cur]) + "-Stein weg"; break;
    case M_ENDE:
      if (winner == 2) s = "Unentschieden!  NEU = neue Partie";
      else s = String(NAME[winner]) + " gewinnt!  NEU = neue Partie";
      break;
  }
  tft.drawString(s, 4, 2, 2);
}

void drawPanel() {  // rechte Spalte: noch zu setzende Steine + NEU-Knopf
  tft.fillRect(246, 22, 74, 218, TFT_BLACK);
  tft.drawFastVLine(246, 22, 218, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Weiss", 254, 26, 1);
  tft.drawString("Schwarz", 254, 110, 1);
  for (int i = 0; i < 9; i++) {
    int y1 = 40 + i * 7;
    if (i < toPlace[0]) tft.fillCircle(272, y1, 3, TFT_WHITE);
    else tft.fillCircle(272, y1, 2, TFT_DARKGREY);
    int y2 = 124 + i * 7;
    if (i < toPlace[1]) {
      tft.fillCircle(272, y2, 3, TFT_BLACK);
      tft.drawCircle(272, y2, 3, TFT_WHITE);
    } else tft.fillCircle(272, y2, 2, TFT_DARKGREY);
  }
  tft.fillRect(250, 212, 68, 26, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.drawCentreString("NEU", 284, 219, 2);
}

void drawBoardAll() {
  tft.fillScreen(TFT_BLACK);
  tft.drawRect( 62,  43, 176, 176, TFT_DARKGREY);   // aeusseres Quadrat
  tft.drawRect( 93,  74, 114, 114, TFT_DARKGREY);   // mittleres Quadrat
  tft.drawRect(124, 105,  52,  52, TFT_DARKGREY);   // inneres Quadrat
  tft.drawLine(150,  43, 150, 105, TFT_DARKGREY);  // Verbindungslinien
  tft.drawLine(150, 157, 150, 219, TFT_DARKGREY);
  tft.drawLine( 62, 131, 124, 131, TFT_DARKGREY);
  tft.drawLine(176, 131, 238, 131, TFT_DARKGREY);
  for (int i = 0; i < 24; i++) drawPoint(i);
  drawPanel();
  updateStatus();
}

void flashMill(int m) {  // Muehle kurz aufblitzen lassen
  Serial.println(">>> Muehle! (" + String(NAME[cur]) + ")");
  for (int k = 0; k < 3; k++) tft.drawCircle(PX[MILLS[m][k]], PY[MILLS[m][k]], 13, TFT_ORANGE);
  delay(700);
  for (int k = 0; k < 3; k++) {
    tft.drawCircle(PX[MILLS[m][k]], PY[MILLS[m][k]], 13, TFT_BLACK);
    drawPoint(MILLS[m][k]);
  }
}

// ---------------------------------------------------------------
// Spielablauf
// ---------------------------------------------------------------

void gameOver(int w) {
  winner = w;
  mode = M_ENDE;
  updateStatus();
  if (w == 2) Serial.println("=== Unentschieden ===");
  else Serial.println("=== " + String(NAME[w]) + " gewinnt ===");
}

void newTurn() {
  sel = -1;
  if (toPlace[cur] > 0) {
    canJump = false;
    mode = M_SETZEN;
  } else {
    canJump = (onBoard[cur] <= 3);                       // Sprungphase (Anleitung S. 3)
    if (!canJump && !hasAnyMove(cur)) { gameOver(1 - cur); return; }  // kein legaler Zug (S. 5)
    mode = M_WAEHLEN;
  }
  drawPanel();
  updateStatus();
}

void endTurn() {
  cur = 1 - cur;
  // 20 Zuege ohne Muehle = Unentschieden (Anleitung S. 5; wir zaehlen
  // einzelne Spielerzuege, da die Anleitung "insgesamt 20" sagt)
  if (noMill >= 20) { gameOver(2); return; }
  // dreimal gleiche Stellung = Unentschieden (Anleitung S. 5)
  if (histN < 300) {
    uint64_t h = hashPos();
    hist[histN++] = h;
    int c = 0;
    for (int i = 0; i < histN; i++) if (hist[i] == h) c++;
    if (c >= 3) { gameOver(2); return; }
  }
  newTurn();
}

void startRemoval(int m) {
  flashMill(m);
  noMill = 0;
  bool anyOpp = false, anyFree = false;
  for (int i = 0; i < 24; i++) {
    if (brett[i] == (1 - cur) + 1) {
      anyOpp = true;
      if (!inMillPoint(i)) anyFree = true;
    }
  }
  if (!anyOpp) { endTurn(); return; }   // kein Stein zu holen
  oppAllInMill = !anyFree;              // Turnierregel (Anleitung S. 5, Sonderfall 1)
  mode = M_ENTFERNEN;
  updateStatus();
}

void setStone(int i) {
  brett[i] = cur + 1;
  toPlace[cur]--;
  onBoard[cur]++;
  drawPoint(i);
  Serial.println(String(NAME[cur]) + " setzt auf " + i);
  int m = closedMillAt(i);
  if (m >= 0) startRemoval(m);          // Doppel-Muehle: trotzdem nur 1 Stein (S. 5)
  else { noMill++; endTurn(); }
}

void doMove(int f, int t) {
  clearTargets();
  clearSelRing(f);
  sel = -1;
  brett[f] = 0;
  brett[t] = cur + 1;
  drawPoint(f);
  drawPoint(t);
  Serial.println(String(NAME[cur]) + " zieht " + f + " -> " + t + (canJump ? " (Sprung)" : ""));
  int m = closedMillAt(t);
  if (m >= 0) startRemoval(m);
  else { noMill++; endTurn(); }
}

void removeStone(int p) {
  Serial.println(String(NAME[cur]) + " entfernt " + p + " (" + NAME[1 - cur] + ")");
  brett[p] = 0;
  onBoard[1 - cur]--;
  drawPoint(p);
  // weniger als 3 Steine auf dem Brett = verloren (Anleitung S. 5;
  // gilt erst, wenn keine Steine mehr zu setzen sind)
  if (toPlace[1 - cur] == 0 && onBoard[1 - cur] < 3) { gameOver(cur); return; }
  endTurn();
}

void resetGame() {
  for (int i = 0; i < 24; i++) brett[i] = 0;
  toPlace[0] = toPlace[1] = 9;
  onBoard[0] = onBoard[1] = 0;
  cur = 0;              // Weiss beginnt (Anleitung S. 2)
  sel = -1;
  canJump = false;
  noMill = 0;
  histN = 0;
  winner = -1;
  mode = M_SETZEN;
  drawBoardAll();
  Serial.println("=== Neue Partie: Weiss beginnt ===");
}

// ---------------------------------------------------------------
// Eingabe
// ---------------------------------------------------------------

int nearestPoint(int x, int y) {
  int best = -1, bd = 16 * 16;
  for (int i = 0; i < 24; i++) {
    int dx = x - PX[i], dy = y - PY[i];
    int d = dx * dx + dy * dy;
    if (d < bd) { bd = d; best = i; }
  }
  return best;
}

void handleTap(int x, int y) {
  // NEU-Knopf: jederzeit neue Partie
  if (x >= 250 && x <= 318 && y >= 212 && y <= 238) { resetGame(); return; }
  if (mode == M_ENDE) return;

  int p = nearestPoint(x, y);

  // neben einen Punkt getippt: Auswahl aufheben
  if (p < 0) {
    if (mode == M_ZIEL) {
      clearTargets();
      clearSelRing(sel);
      sel = -1;
      mode = M_WAEHLEN;
      updateStatus();
    }
    return;
  }

  switch (mode) {
    case M_SETZEN:
      if (brett[p] == 0) setStone(p);
      break;

    case M_WAEHLEN:
      if (brett[p] == cur + 1) {
        sel = p;
        drawSelRing(p);
        showTargets();
        mode = M_ZIEL;
        updateStatus();
      }
      break;

    case M_ZIEL:
      if (p == sel) {                       // gleicher Stein: abwaehlen
        clearTargets();
        clearSelRing(sel);
        sel = -1;
        mode = M_WAEHLEN;
        updateStatus();
      } else if (brett[p] == cur + 1) {     // anderer eigener Stein: neu waehlen
        clearTargets();
        clearSelRing(sel);
        sel = p;
        drawSelRing(p);
        showTargets();
      } else if (legalTarget(sel, p)) {
        doMove(sel, p);
      }
      break;

    case M_ENTFERNEN:
      // kein Stein aus bestehender Muehle - ausser keiner frei (Anleitung S. 4/5)
      if (brett[p] == (1 - cur) + 1 && (oppAllInMill || !inMillPoint(p))) {
        removeStone(p);
      }
      break;

    default:
      break;
  }
}

// ---------------------------------------------------------------
// Setup und Hauptschleife
// ---------------------------------------------------------------

void setup() {
  Serial.begin(115200);
  touchSPI.begin(XPT2046_CLK, XPT2046_MISO, XPT2046_MOSI, XPT2046_CS);
  touch.begin(touchSPI);
  touch.setRotation(1);
  tft.init();
  tft.setRotation(1);
  resetGame();
  Serial.println("CYD Muehle bereit. NEU rechts unten = neue Partie.");
}

void loop() {
  if (touch.tirqTouched() && touch.touched()) {
    TS_Point p = touch.getPoint();
    int x = mapRaw(p.x, TS_X_AT_0, TS_X_AT_319, W - 1);
    int y = mapRaw(p.y, TS_Y_AT_0, TS_Y_AT_239, H - 1);
    handleTap(x, y);
    while (touch.touched()) delay(20);   // auf Loslassen warten (entprellt)
    delay(30);
  }
}
