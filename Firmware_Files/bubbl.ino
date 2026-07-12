// ============================================================================
//  bubbl — a tiny networked virtual pet for ESP32-S3
//  OLED (SSD1306) + analog joystick + Supabase backend + I2S audio
// ----------------------------------------------------------------------------
//  Highlights of this build:
//    * One reusable TLS-aware HTTP helper (no more copy-pasted boilerplate)
//    * Proper 2xx status + JSON error checking on every request
//    * Non-blocking WiFi connect with timeout + auto-reconnect
//    * Animated pet: blinking eyes, mood-driven mouth, subtle idle bob
//    * Graphical stat bars + live connection indicator
//    * Nudge notifications actually play a sound now
// ============================================================================

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Fonts/Picopixel.h>
#include <Audio.h>
#include <SPIFFS.h>

// ---- Credentials -----------------------------------------------------------
const char* ssid          = "your_wifi_ssid";
const char* password      = "your_wifi_password";
const char* supabaseUrl   = "your_supabase_url";   // e.g. https://xxxx.supabase.co
const char* apiKey        = "your_supabase_api_key";

String username = "xyz_user";

// ---- Audio -----------------------------------------------------------------
Audio audio;
const char* NOTIFY_SOUND = "/bubbl.wav";   // file present in SPIFFS
bool playNotification = false;

// ---- Display ---------------------------------------------------------------
#define SCREEN_WIDTH  128
#define SCREEN_HEIGHT 64
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

// ---- Pins (ESP32-S3) -------------------------------------------------------
#define JOY_X   A0
#define JOY_Y   A1
#define JOY_BTN D2
#define I2C_SDA 5
#define I2C_SCL 6
#define I2S_BCLK 9
#define I2S_LRC  10
#define I2S_DIN  8

// ---- Timing (ms) -----------------------------------------------------------
const unsigned long updateDelay     = 60000;   // pet sync cooldown
const unsigned long pressDelay      = 250;
const unsigned long moveDelay       = 200;
const unsigned long frameDelay      = 33;      // ~30 fps
const unsigned long popupDuration   = 2000;
const unsigned long longPressDelay  = 700;
const unsigned long nudgeCheckDelay = 60000;
const unsigned long blinkInterval   = 3200;    // blink roughly every few seconds
const unsigned long blinkDuration   = 140;
const unsigned long wifiTimeout     = 15000;   // initial connect budget
const unsigned long wifiRetryDelay  = 10000;   // gap between reconnect attempts
const int           httpTimeout     = 5000;
const int           httpRetries     = 2;

// ---- HTTP joystick thresholds ----------------------------------------------
const int JOY_LOW  = 1200;
const int JOY_HIGH = 2800;

// ---- State flags -----------------------------------------------------------
bool firstSync          = true;
bool loadingPopup       = false;
bool lastButtonState    = HIGH;
bool longPressTriggered = false;

// ---- Model -----------------------------------------------------------------
struct Pet {
  int hunger;
  int happy;
  int energy;
  unsigned long age;
  int mood;       // 0 sad, 1 ok, 2 happy
};
Pet pet = {0, 0, 0, 0, 1};

struct Event { String text; };
Event events[5];
String users[10];

int eventCount    = 0;
int selectedEvent = 0;
int userCount     = 0;
int selectedUser  = 0;

// ---- Animation / scroll ----------------------------------------------------
unsigned long textScrollTimer = 0;
int  textScrollX = 0;

unsigned long lastUpdate      = 0;
unsigned long lastPrint       = 0;
unsigned long lastPress       = 0;
unsigned long lastMove        = 0;
unsigned long lastFrame       = 0;
unsigned long popupStart      = 0;
unsigned long buttonHoldStart = 0;
unsigned long lastNudgeCheck  = 0;
unsigned long lastWifiTry     = 0;
unsigned long blinkTimer      = 0;
bool          eyesClosed      = false;

// ---- Menu data -------------------------------------------------------------
const char* options[]   = {"Feed", "Rest", "Play"};
int selected = 1;
const int total = 3;
int xPositions[] = {2, 53, 103};

const char* petOptions[] = {"Stats", "Nudge", "Activity"};
int selectedPetOption = 0;
const int petTotal = 3;

enum Screen { MAIN, PET_MENU, STATS, ACTIVITY, NUDGE };
Screen currentScreen = MAIN;

enum Focus { MENU, PET };
Focus currentFocus = MENU;

String popupMessage = "";

// ---- Networking ------------------------------------------------------------
WiFiClientSecure secureClient;

// ---- Forward declarations --------------------------------------------------
void petSync(bool force = false);
void eventSync();
void fetchUsers();
void sendNudge();
void checkForNudges();
void connectWiFi();
void ensureWiFi();

// ============================================================================
//  HTTP helper — TLS-aware, retrying, with proper status reporting
// ============================================================================

// Performs a request against `path` (appended to supabaseUrl).
// Returns true only on a 2xx response. The body (if any) lands in `outBody`,
// and the final HTTP status code in `outCode`.
bool supabaseRequest(const char* method,
                     const String& path,
                     const String& body,
                     String& outBody,
                     int& outCode) {
  outBody = "";
  outCode = -1;

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[HTTP] skipped — WiFi down");
    return false;
  }

  String url = String(supabaseUrl) + path;

  for (int attempt = 0; attempt <= httpRetries; attempt++) {
    HTTPClient http;
    if (!http.begin(secureClient, url)) {
      Serial.println("[HTTP] begin() failed");
      continue;
    }

    http.setTimeout(httpTimeout);
    http.addHeader("apikey", apiKey);
    http.addHeader("Authorization", "Bearer " + String(apiKey));
    if (body.length() > 0) {
      http.addHeader("Content-Type", "application/json");
    }

    int code;
    if (strcmp(method, "GET") == 0) {
      code = http.GET();
    } else if (strcmp(method, "POST") == 0) {
      code = http.POST(body);
    } else {
      code = http.sendRequest(method, (uint8_t*)body.c_str(), body.length());
    }

    outCode = code;
    if (code > 0) {
      outBody = http.getString();
    }
    http.end();

    Serial.printf("[HTTP] %s %s -> %d (try %d)\n",
                  method, path.c_str(), code, attempt + 1);

    if (code >= 200 && code < 300) return true;

    // Retry only on transient/connection failures, not on real HTTP errors.
    if (code > 0) return false;
    delay(150);
  }
  return false;
}

// Convenience wrapper: GET + JSON parse with error checking.
bool supabaseGetJson(const String& path, JsonDocument& doc) {
  String resp;
  int code;
  if (!supabaseRequest("GET", path, "", resp, code)) return false;

  DeserializationError err = deserializeJson(doc, resp);
  if (err) {
    Serial.print("[JSON] parse error: ");
    Serial.println(err.c_str());
    return false;
  }
  return true;
}

// ============================================================================
//  Drawing
// ============================================================================

void drawPopup() {
  if (loadingPopup) {
    display.fillRect(14, 20, 100, 20, WHITE);
    display.setTextColor(BLACK);
    display.setCursor(34, 27);
    display.print("Loading...");
    display.setTextColor(WHITE);
  } else if (millis() - popupStart < popupDuration) {
    display.fillRect(8, 20, 112, 20, WHITE);
    display.setTextColor(BLACK);
    display.setCursor(18, 27);
    display.print(popupMessage);
    display.setTextColor(WHITE);
  }
}

// Small WiFi indicator in the top-left corner.
void drawConnIcon() {
  bool up = (WiFi.status() == WL_CONNECTED);
  if (up) {
    // three rising bars
    display.fillRect(2, 6, 2, 2, WHITE);
    display.fillRect(5, 4, 2, 4, WHITE);
    display.fillRect(8, 2, 2, 6, WHITE);
  } else {
    // hollow bars + slash to signal offline
    display.drawRect(2, 6, 2, 2, WHITE);
    display.drawRect(5, 4, 2, 4, WHITE);
    display.drawLine(1, 8, 10, 1, WHITE);
  }
}

void drawBootScreen() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(5, 23);
  display.print("Bubbl is starting");
  display.setCursor(26, 33);
  display.print("Please wait..");
  display.display();
}

// Draws the animated face inside the main circle.
// `filled` = pet circle is highlighted (focused).
void drawPetFace(int cx, int cy, bool filled) {
  uint16_t bg = filled ? WHITE : BLACK;
  uint16_t fg = filled ? BLACK : WHITE;

  if (filled) display.fillCircle(cx, cy, 20, WHITE);
  else        display.drawCircle(cx, cy, 20, WHITE);

  int eyeY = cy - 4;
  int eyeDX = 7;

  if (eyesClosed) {
    // happy/sleepy closed eyes
    display.drawFastHLine(cx - eyeDX - 1, eyeY, 4, fg);
    display.drawFastHLine(cx + eyeDX - 2, eyeY, 4, fg);
  } else {
    display.fillRect(cx - eyeDX - 1, eyeY - 1, 2, 3, fg);
    display.fillRect(cx + eyeDX - 1, eyeY - 1, 2, 3, fg);
  }

  // Mouth reflects mood.
  int mouthY = cy + 7;
  if (pet.mood == 2) {
    // smile
    display.drawLine(cx - 6, mouthY,     cx - 3, mouthY + 2, fg);
    display.drawLine(cx - 3, mouthY + 2, cx + 3, mouthY + 2, fg);
    display.drawLine(cx + 3, mouthY + 2, cx + 6, mouthY,     fg);
  } else if (pet.mood == 1) {
    // flat
    display.drawFastHLine(cx - 5, mouthY + 1, 11, fg);
  } else {
    // frown
    display.drawLine(cx - 6, mouthY + 2, cx - 3, mouthY,     fg);
    display.drawLine(cx - 3, mouthY,     cx + 3, mouthY,     fg);
    display.drawLine(cx + 3, mouthY,     cx + 6, mouthY + 2, fg);
  }
  (void)bg;
}

void drawMainUI() {
  display.clearDisplay();

  drawConnIcon();

  display.setFont(&Picopixel);
  display.setTextColor(WHITE);
  display.setCursor(56, 4);
  display.print("bubbl");
  display.setFont();

  // Subtle idle bob: ±1px over a slow sine-ish cycle.
  int bob = ((millis() / 600) % 2 == 0) ? 0 : 1;
  drawPetFace(64, 28 + bob, currentFocus == PET);

  for (int i = 0; i < total; i++) {
    if (currentFocus == MENU && i == selected) {
      display.setTextColor(BLACK, WHITE);
    } else {
      display.setTextColor(WHITE);
    }
    display.setCursor(xPositions[i], 54);
    display.print(options[i]);
  }

  drawPopup();
  display.display();
}

void drawPetMainUI() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(38, 5);
  display.print("Pet Menu");

  for (int i = 0; i < petTotal; i++) {
    int y = 20 + (i * 12);
    if (i == selectedPetOption) {
      display.fillRect(18, y - 1, 92, 10, WHITE);
      display.setTextColor(BLACK);
    } else {
      display.setTextColor(WHITE);
    }
    display.setCursor(28, y);
    display.print(petOptions[i]);
  }

  drawPopup();
  display.display();
}

// Draws a labelled stat bar at row `y`.
void drawStatBar(int y, const char* label, int value) {
  display.setTextColor(WHITE);
  display.setCursor(2, y);
  display.print(label);

  int barX = 52, barW = 70, barH = 6;
  int barY = y - 1;
  display.drawRect(barX, barY, barW, barH, WHITE);

  int v = constrain(value, 0, 100);
  int fillW = (barW - 2) * v / 100;
  if (fillW > 0) display.fillRect(barX + 1, barY + 1, fillW, barH - 2, WHITE);
}

void drawStatsUI() {
  display.clearDisplay();
  display.setTextWrap(false);

  display.setTextColor(BLACK, WHITE);
  display.setCursor(5, 1);
  if (pet.mood == 0)      display.print(" Your pet is sad   ");
  else if (pet.mood == 1) display.print(" Your pet is ok    ");
  else                    display.print(" Your pet is happy ");

  drawStatBar(22, "Food", pet.hunger);
  drawStatBar(36, "Joy ", pet.happy);
  drawStatBar(50, "Pep ", pet.energy);

  drawPopup();
  display.display();
}

void drawActivityUI() {
  display.clearDisplay();
  display.setTextWrap(false);
  display.setTextColor(BLACK, WHITE);
  display.setCursor(0, 0);
  display.print("    Recent Actions    ");

  if (eventCount == 0) {
    display.setTextColor(WHITE);
    display.setCursor(18, 30);
    display.print("No activity yet");
  }

  for (int i = 0; i < eventCount; i++) {
    int y = 11 + (i * 10);
    bool selectedNow = (i == selectedEvent);
    String text = "> " + events[i].text;

    if (selectedNow) {
      display.fillRect(0, y - 1, 128, 9, WHITE);
      display.setTextColor(BLACK);

      int textWidth = text.length() * 6;
      if (textWidth > 128) {
        if (millis() - textScrollTimer > 120) {
          textScrollTimer = millis();
          textScrollX--;
          if (textScrollX < -(textWidth - 120)) textScrollX = 0;
        }
        display.setCursor(textScrollX + 2, y);
      } else {
        display.setCursor(0, y);
      }
    } else {
      display.setTextColor(WHITE);
      display.setCursor(0, y);
    }
    display.print(text);
  }

  drawPopup();
  display.display();
}

void drawNudgeUI() {
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setCursor(28, 5);
  display.print("Nudge Friend");

  if (userCount == 0) {
    display.setCursor(20, 30);
    display.print("No friends online");
  }

  for (int i = 0; i < userCount; i++) {
    int y = 20 + (i * 10);
    if (i == selectedUser) {
      display.fillRect(8, y - 1, 112, 9, WHITE);
      display.setTextColor(BLACK);
    } else {
      display.setTextColor(WHITE);
    }
    display.setCursor(16, y);
    display.print(users[i]);
  }

  drawPopup();
  display.display();
}

// ============================================================================
//  Pet logic
// ============================================================================

void petStatus() {
  if (pet.hunger < 30 || pet.happy < 30 || pet.energy < 30) {
    pet.mood = 0;
  } else if (pet.hunger > 65 && pet.happy > 65 && pet.energy > 65) {
    pet.mood = 2;
  } else {
    pet.mood = 1;
  }
  Serial.print("[PET] mood: ");
  Serial.println(pet.mood);
}

// Advances the blink animation. Call every frame.
void updateBlink() {
  unsigned long now = millis();
  if (!eyesClosed && now - blinkTimer > blinkInterval) {
    eyesClosed = true;
    blinkTimer = now;
  } else if (eyesClosed && now - blinkTimer > blinkDuration) {
    eyesClosed = false;
    blinkTimer = now;
  }
}

// ============================================================================
//  WiFi
// ============================================================================

void connectWiFi() {
  Serial.print("[WiFi] connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  unsigned long start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < wifiTimeout) {
    Serial.print(".");
    delay(250);
  }
  Serial.println();

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("[WiFi] connected, IP: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("[WiFi] connect timed out — will keep retrying");
  }
  lastWifiTry = millis();
}

// Non-blocking reconnect: nudges WiFi back up without freezing the UI.
void ensureWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;
  if (millis() - lastWifiTry < wifiRetryDelay) return;

  Serial.println("[WiFi] reconnecting...");
  WiFi.disconnect();
  WiFi.begin(ssid, password);
  lastWifiTry = millis();
}

// ============================================================================
//  Backend calls
// ============================================================================

void callFunction(String functionName) {
  Serial.print("[RPC] ");
  Serial.println(functionName);

  loadingPopup = true;
  if (currentScreen == MAIN)       drawMainUI();
  else if (currentScreen == STATS) drawStatsUI();

  String body = "{\"user_name\":\"" + username + "\"}";
  String resp;
  int code;
  bool ok = supabaseRequest("POST", "/rest/v1/rpc/" + functionName, body, resp, code);

  loadingPopup = false;
  popupMessage = ok ? "Success!" : "Server Error";
  popupStart   = millis();
}

void feedPet() { Serial.println("[ACT] feed"); callFunction("feed_pet"); petSync(true); }
void restPet() { Serial.println("[ACT] rest"); callFunction("rest_pet"); petSync(true); }
void playPet() { Serial.println("[ACT] play"); callFunction("play_pet"); petSync(true); }

void petSync(bool force) {
  if (!force && !firstSync && millis() - lastUpdate < updateDelay) {
    Serial.println("[SYNC] skipped (cooldown)");
    return;
  }
  firstSync  = false;
  lastUpdate = millis();

  DynamicJsonDocument doc(2048);
  if (!supabaseGetJson("/rest/v1/pet?id=eq.1&select=*", doc)) {
    Serial.println("[SYNC] pet sync failed");
    return;
  }

  JsonObject petData = doc[0];
  if (petData.isNull()) {
    Serial.println("[SYNC] empty pet payload");
    return;
  }

  pet.hunger = petData["hunger"]    | pet.hunger;
  pet.happy  = petData["happiness"] | pet.happy;
  pet.energy = petData["energy"]    | pet.energy;

  Serial.printf("[SYNC] hunger=%d happy=%d energy=%d\n",
                pet.hunger, pet.happy, pet.energy);
  petStatus();
}

void eventSync() {
  Serial.println("[EVT] sync");
  loadingPopup = true;
  drawActivityUI();

  DynamicJsonDocument doc(2048);
  bool ok = supabaseGetJson("/rest/v1/recent_events?select=*&limit=5", doc);
  loadingPopup = false;
  if (!ok) { Serial.println("[EVT] failed"); return; }

  eventCount = 0;
  for (JsonObject item : doc.as<JsonArray>()) {
    if (eventCount >= 5) break;

    String eventUser = item["username"].as<String>();
    String action    = item["action"].as<String>();
    String ago       = item["time_ago"].as<String>();

    String actionText;
    if (action == "feed")      actionText = "fed";
    else if (action == "rest") actionText = "rested";
    else                       actionText = "played with";

    events[eventCount].text =
      "\"" + eventUser + "\" " + actionText + " your bubbl (" + ago + ")";

    Serial.printf("[EVT] %d: %s\n", eventCount, events[eventCount].text.c_str());
    eventCount++;
  }
}

void fetchUsers() {
  Serial.println("[USR] fetch");

  DynamicJsonDocument doc(2048);
  if (!supabaseGetJson("/rest/v1/users?select=username", doc)) {
    Serial.println("[USR] failed");
    return;
  }

  userCount = 0;
  for (JsonObject item : doc.as<JsonArray>()) {
    if (userCount >= 10) break;
    String fetchedUser = item["username"].as<String>();
    if (fetchedUser != username) {
      users[userCount] = fetchedUser;
      Serial.print("[USR] + ");
      Serial.println(users[userCount]);
      userCount++;
    }
  }
  Serial.printf("[USR] %d loaded\n", userCount);
}

void sendNudge() {
  if (userCount == 0) { Serial.println("[NDG] no users"); return; }

  Serial.print("[NDG] -> ");
  Serial.println(users[selectedUser]);

  String body = "{\"sender\":\"" + username +
                "\",\"receiver\":\"" + users[selectedUser] + "\"}";
  String resp;
  int code;
  bool ok = supabaseRequest("POST", "/rest/v1/nudges", body, resp, code);

  popupMessage = ok ? "Buzz Sent!" : "Buzz Failed!";
  popupStart   = millis();
}

void checkForNudges() {
  if (millis() - lastNudgeCheck < nudgeCheckDelay) return;
  lastNudgeCheck = millis();

  if (WiFi.status() != WL_CONNECTED) {
    Serial.println("[NDG] check skipped — WiFi down");
    return;
  }
  Serial.println("[NDG] checking inbox");

  String path = "/rest/v1/nudges?receiver=eq." + username +
                "&seen=eq.false&order=created_at.desc&limit=1";

  DynamicJsonDocument doc(1024);
  if (!supabaseGetJson(path, doc)) { Serial.println("[NDG] check failed"); return; }

  JsonArray arr = doc.as<JsonArray>();
  if (arr.size() == 0) return;

  JsonObject nudge = arr[0];
  String sender = nudge["sender"].as<String>();
  long   nudgeId = nudge["id"] | -1;

  popupMessage     = "\"" + sender + "\" nudged you";
  popupStart       = millis();
  playNotification = true;     // <- actually trigger the sound now
  Serial.println("[NDG] received!");

  if (nudgeId >= 0) {
    String seenPath = "/rest/v1/nudges?id=eq." + String(nudgeId);
    String resp;
    int code;
    supabaseRequest("PATCH", seenPath, "{\"seen\":true}", resp, code);
  }
}

// ============================================================================
//  Input
// ============================================================================

void handleJoystick() {
  int x = analogRead(JOY_X);
  int y = 4095 - analogRead(JOY_Y);

  if (millis() - lastMove < moveDelay) return;

  if (currentScreen == ACTIVITY) {
    if (eventCount == 0) return;
    if (y < JOY_LOW) {
      selectedEvent = (selectedEvent - 1 + eventCount) % eventCount;
      textScrollX = 0; lastMove = millis();
    } else if (y > JOY_HIGH) {
      selectedEvent = (selectedEvent + 1) % eventCount;
      textScrollX = 0; lastMove = millis();
    }
    return;
  }

  if (currentScreen == PET_MENU) {
    if (y < JOY_LOW) {
      selectedPetOption = (selectedPetOption - 1 + petTotal) % petTotal;
      lastMove = millis();
    } else if (y > JOY_HIGH) {
      selectedPetOption = (selectedPetOption + 1) % petTotal;
      lastMove = millis();
    }
    return;
  }

  if (currentScreen == NUDGE) {
    if (userCount == 0) return;
    if (y < JOY_LOW) {
      selectedUser = (selectedUser - 1 + userCount) % userCount;
      lastMove = millis();
    } else if (y > JOY_HIGH) {
      selectedUser = (selectedUser + 1) % userCount;
      lastMove = millis();
    }
    return;
  }

  if (currentScreen == MAIN) {
    if (y < JOY_LOW && currentFocus == MENU) {
      currentFocus = PET; lastMove = millis(); return;
    }
    if (y > JOY_HIGH && currentFocus == PET) {
      currentFocus = MENU; lastMove = millis(); return;
    }
    if (currentFocus == MENU) {
      if (x > JOY_HIGH) {
        selected = (selected - 1 + total) % total; lastMove = millis();
      } else if (x < JOY_LOW) {
        selected = (selected + 1) % total; lastMove = millis();
      }
    }
  }
}

void handleBtn() {
  bool currentState = digitalRead(JOY_BTN);

  // Falling edge: press begins.
  if (currentState == LOW && lastButtonState == HIGH) {
    buttonHoldStart    = millis();
    longPressTriggered = false;
  }

  // Long press fires while held.
  if (currentState == LOW && !longPressTriggered &&
      millis() - buttonHoldStart > longPressDelay) {
    longPressTriggered = true;

    if (currentScreen == NUDGE) {
      currentScreen = PET_MENU;
      lastPress = millis();
    } else if (currentScreen == STATS || currentScreen == ACTIVITY ||
               currentScreen == PET_MENU) {
      currentScreen = MAIN;
      lastPress = millis();
    }
    lastButtonState = currentState;
    return;
  }

  // Rising edge without a long press = short press (click).
  if (lastButtonState == LOW && currentState == HIGH &&
      !longPressTriggered && millis() - lastPress > pressDelay) {
    lastPress = millis();

    if (currentScreen == PET_MENU) {
      switch (selectedPetOption) {
        case 0: petSync(true); currentScreen = STATS; break;
        case 1: selectedUser = 0; currentScreen = NUDGE; break;
        case 2: eventSync(); selectedEvent = 0; textScrollX = 0;
                currentScreen = ACTIVITY; break;
      }
      lastButtonState = currentState;
      return;
    }

    if (currentScreen == NUDGE) {
      sendNudge();
      lastButtonState = currentState;
      return;
    }

    if (currentScreen == MAIN) {
      if (currentFocus == PET) {
        currentScreen     = PET_MENU;
        selectedPetOption = 0;
      } else {
        switch (selected) {
          case 0: feedPet(); break;
          case 1: restPet(); break;
          case 2: playPet(); break;
        }
      }
    }
  }

  lastButtonState = currentState;
}

// ============================================================================
//  Setup / loop
// ============================================================================

void setup() {
  Serial.begin(115200);
  Serial.println("\n========== BOOT ==========");

  pinMode(JOY_BTN, INPUT_PULLUP);

  Wire.begin(I2C_SDA, I2C_SCL);
  Serial.println("[I2C] started");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println("[OLED] FAILED");
    while (1) delay(10);
  }
  Serial.println("[OLED] ready");
  drawBootScreen();

  connectWiFi();

  // Supabase uses HTTPS; skip cert pinning for simplicity on-device.
  secureClient.setInsecure();

  // Audio: mount SPIFFS first, then wire up I2S. (Mount failure is fatal
  // for sound but not for the rest of the device.)
  if (!SPIFFS.begin(true)) {
    Serial.println("[FS] SPIFFS mount failed — audio disabled");
  }
  audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DIN);
  audio.setVolume(15);

  petSync(true);
  fetchUsers();

  display.clearDisplay();
  display.display();
  Serial.println("[BOOT] setup complete");
}

void loop() {
  audio.loop();

  if (playNotification) {
    audio.connecttoFS(SPIFFS, NOTIFY_SOUND);
    playNotification = false;
  }

  handleJoystick();
  handleBtn();
  ensureWiFi();
  checkForNudges();

  if (millis() - lastFrame >= frameDelay) {
    lastFrame = millis();
    updateBlink();

    switch (currentScreen) {
      case MAIN:     drawMainUI();    break;
      case PET_MENU: drawPetMainUI(); break;
      case STATS:    drawStatsUI();   break;
      case ACTIVITY: drawActivityUI();break;
      case NUDGE:    drawNudgeUI();   break;
    }
  }

  if (millis() - lastPrint > 2500) {
    lastPrint = millis();
    Serial.println("[hb] alive");
  }
}
