#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Fonts/Picopixel.h>
#include <Audio.h>
#include <SPIFFS.h>

const char* ssid = "your_wifi_ssid";
const char* password = "your_wifi_password";

const char* supabaseUrl = "your_supabase_url";
const char* apiKey = "your_supabase_api_key";

String username = "xyz_user";
Audio audio;

bool playNotification = false;

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

#define JOY_X A0
#define JOY_Y A1
#define JOY_BTN D2

const int updateDelay = 60000;
const int pressDelay = 250;
const int moveDelay = 200;
const int frameDelay = 33;
const int popupDuration = 2000;
const int longPressDelay = 700;
const int nudgeCheckDelay = 60000;

bool firstSync = true;
bool loadingPopup = false;
bool lastButtonState = HIGH;
bool longPressTriggered = false;

struct Pet {
  int hunger;
  int happy;
  int energy;
  unsigned long age;
  int mood;
};

Pet pet = {0,0,0,0,0};

struct Event {
  String text;
};

Event events[5];

String users[10];

int eventCount = 0;
int selectedEvent = 0;

int userCount = 0;
int selectedUser = 0;

unsigned long textScrollTimer = 0;
int textScrollX = 0;

unsigned long lastUpdate = 0;
unsigned long lastPrint = 0;
unsigned long lastPress = 0;
unsigned long lastMove = 0;
unsigned long lastFrame = 0;
unsigned long popupStart = 0;
unsigned long buttonHoldStart = 0;
unsigned long lastNudgeCheck = 0;

const char* options[] = {"Feed", "Rest", "Play"};
int selected = 1;
const int total = 3;
int xPositions[] = {2,53,103};

const char* petOptions[] = {"Stats","Nudge","Activity"};
int selectedPetOption = 0;
const int petTotal = 3;

enum Screen {
  MAIN,
  PET_MENU,
  STATS,
  ACTIVITY,
  NUDGE
};

Screen currentScreen = MAIN;

enum Focus {
  MENU,
  PET
};

Focus currentFocus = MENU;

String popupMessage = "";

void petSync();
void eventSync();
void fetchUsers();
void sendNudge();
void checkForNudges();

void drawPopup() {

  if (loadingPopup) {

    display.fillRect(14,20,100,20, WHITE);

    display.setTextColor(BLACK);

    display.setCursor(34,27);

    display.print("Loading...");

    display.setTextColor(WHITE);
  }

  else if (millis() - popupStart < popupDuration) {

    display.fillRect(8,20,112,20, WHITE);

    display.setTextColor(BLACK);

    display.setCursor(18,27);

    display.print(popupMessage);

    display.setTextColor(WHITE);
  }
}

void drawBootScreen() {

  Serial.println("Drawing boot screen");

  display.clearDisplay();

  display.setTextColor(WHITE);

  display.setTextSize(1);

  display.setCursor(5,23);

  display.print("Bubbl is starting");

  display.setCursor(26,33);

  display.print("Please wait..");

  display.display();
}

void drawMainUI() {

  display.clearDisplay();

  display.setFont(&Picopixel);

  display.setTextColor(WHITE);

  display.setCursor(56,4);

  display.print("bubbl");

  display.setFont();

  if (currentFocus == PET) {

    display.fillCircle(64,28,20, WHITE);

    display.setTextColor(BLACK);

    display.setCursor(59,32);

    if (pet.mood == 0) display.print(":(");
    else if (pet.mood == 1) display.print(":|");
    else display.print(":)");
  }

  else {

    display.drawCircle(64,28,20, WHITE);
  }

  for (int i = 0; i < total; i++) {

    if (currentFocus == MENU && i == selected) {
      display.setTextColor(BLACK,WHITE);
    }

    else {
      display.setTextColor(WHITE);
    }

    display.setCursor(xPositions[i],54);

    display.print(options[i]);
  }

  drawPopup();

  display.display();
}

void drawPetMainUI() {

  display.clearDisplay();

  display.setTextColor(WHITE);

  display.setCursor(38,5);

  display.print("Pet Menu");

  for (int i = 0; i < petTotal; i++) {

    int y = 20 + (i * 12);

    if (i == selectedPetOption) {

      display.fillRect(18, y - 1, 92, 10, WHITE);

      display.setTextColor(BLACK);
    }

    else {

      display.setTextColor(WHITE);
    }

    display.setCursor(28,y);

    display.print(petOptions[i]);
  }

  drawPopup();

  display.display();
}

void drawStatsUI() {

  display.clearDisplay();

  display.setTextColor(WHITE);

  display.setTextWrap(false);

  display.setCursor(29,44);

  display.print("Hunger - ");

  display.print(pet.hunger);

  display.setCursor(20,24);

  display.print("Happiness - ");

  display.print(pet.happy);

  display.setCursor(29,34);

  display.print("Energy - ");

  display.print(pet.energy);

  display.setTextColor(BLACK,WHITE);

  display.setCursor(5,9);

  if (pet.mood == 0) display.print(" Your pet is sad ");
  else if (pet.mood == 1) display.print(" Your pet is ok ");
  else display.print(" Your pet is happy ");

  drawPopup();

  display.display();
}

void drawActivityUI() {

  display.clearDisplay();

  display.setTextWrap(false);

  display.setTextColor(BLACK,WHITE);

  display.setCursor(0,0);

  display.print("    Recent Actions    ");

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

          if (textScrollX < -(textWidth - 120)) {
            textScrollX = 0;
          }
        }

        display.setCursor(textScrollX + 2, y);
      }

      else {

        display.setCursor(0,y);
      }
    }

    else {

      display.setTextColor(WHITE);

      display.setCursor(0,y);
    }

    display.print(text);
  }

  drawPopup();

  display.display();
}

void drawNudgeUI() {

  display.clearDisplay();

  display.setTextColor(WHITE);

  display.setCursor(28,5);

  display.print("Nudge Friend");

  for (int i = 0; i < userCount; i++) {

    int y = 20 + (i * 10);

    if (i == selectedUser) {

      display.fillRect(8, y - 1, 112, 9, WHITE);

      display.setTextColor(BLACK);
    }

    else {

      display.setTextColor(WHITE);
    }

    display.setCursor(16,y);

    display.print(users[i]);
  }

  drawPopup();

  display.display();
}

void petStatus() {

  Serial.println("Updating pet mood");

  if (pet.hunger < 30 || pet.happy < 30 || pet.energy < 30) {
    pet.mood = 0;
  }

  else if (
    pet.hunger > 65 &&
    pet.happy > 65 &&
    pet.energy > 65
  ) {
    pet.mood = 2;
  }

  else {
    pet.mood = 1;
  }

  Serial.print("Pet mood: ");
  Serial.println(pet.mood);
}

void setup() {

  Serial.begin(115200);

  Serial.println();
  Serial.println("========== BOOT ==========");

  pinMode(JOY_BTN, INPUT_PULLUP);

  Serial.println("Joystick button configured");

  Wire.begin(5,6);

  Serial.println("I2C started");

  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {

    Serial.println("OLED FAILED");

    while (1);
  }

  Serial.println("OLED initialized");

  drawBootScreen();

  Serial.print("Connecting to WiFi: ");
  Serial.println(ssid);

  WiFi.begin(ssid,password);

  while (WiFi.status() != WL_CONNECTED) {

    Serial.print(".");
    delay(500);
  }

  Serial.println();
  Serial.println("WiFi connected");

  if (!SPIFFS.begin(true)) {
  Serial.println("SPIFFS Mount Failed");

  audio.connecttoFS(
  SPIFFS,
  "/bubbl.wav"
  )

}

audio.setPinout(
  9,   // BCLK
  10,  // LRC / WS
  8    // DIN
);

audio.setVolume(15);

  Serial.print("IP: ");
  Serial.println(WiFi.localIP());

  petSync();

  fetchUsers();

  display.clearDisplay();

  display.display();

  Serial.println("Setup complete");
}

void callFunction(String functionName) {

  Serial.print("Calling RPC function: ");
  Serial.println(functionName);

  loadingPopup = true;

  if (currentScreen == MAIN) {
    drawMainUI();
  }

  else if (currentScreen == STATS) {
    drawStatsUI();
  }

  HTTPClient http;

  String url =
    String(supabaseUrl) +
    "/rest/v1/rpc/" +
    functionName;

  Serial.print("RPC URL: ");
  Serial.println(url);

  http.begin(url);

  http.setTimeout(2000);

  http.addHeader("Content-Type","application/json");

  http.addHeader("apikey",apiKey);

  http.addHeader(
    "Authorization",
    "Bearer " + String(apiKey)
  );

  String body =
    "{\"user_name\":\"" +
    username +
    "\"}";

  Serial.print("RPC Body: ");
  Serial.println(body);

  int httpResponseCode = http.POST(body);

  Serial.print("RPC Response Code: ");
  Serial.println(httpResponseCode);

  loadingPopup = false;

  if (httpResponseCode == 204) {

    popupMessage = "Success!";

    popupStart = millis();

    Serial.println("RPC success");
  }

  else {

    popupMessage = "Server Error";

    popupStart = millis();

    Serial.println("RPC failed");
  }

  http.end();

  Serial.println("RPC request ended");
}

void feedPet() {

  Serial.println("Feed pet selected");

  callFunction("feed_pet");

  petSync();
}

void restPet() {

  Serial.println("Rest pet selected");

  callFunction("rest_pet");

  petSync();
}

void playPet() {

  Serial.println("Play pet selected");

  callFunction("play_pet");

  petSync();
}

void petSync() {

  Serial.println("Starting pet sync");

  if (!firstSync &&
      millis() - lastUpdate < updateDelay) {

    Serial.println("Pet sync skipped due to cooldown");
    return;
  }

  firstSync = false;

  lastUpdate = millis();

  HTTPClient http;

  String url =
    String(supabaseUrl) +
    "/rest/v1/pet?id=eq.1&select=*";

  Serial.print("Pet sync URL: ");
  Serial.println(url);

  http.begin(url);

  http.addHeader("apikey", apiKey);

  http.addHeader(
    "Authorization",
    "Bearer " + String(apiKey)
  );

  int httpResponseCode = http.GET();

  Serial.print("Pet sync response: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {

    String response = http.getString();

    Serial.print("Pet sync raw response: ");
    Serial.println(response);

    DynamicJsonDocument doc(2048);

    deserializeJson(doc, response);

    JsonObject petData = doc[0];

    pet.hunger = petData["hunger"];

    pet.happy = petData["happiness"];

    pet.energy = petData["energy"];

    Serial.print("Hunger: ");
    Serial.println(pet.hunger);

    Serial.print("Happy: ");
    Serial.println(pet.happy);

    Serial.print("Energy: ");
    Serial.println(pet.energy);

    petStatus();
  }

  else {

    Serial.println("Pet sync failed");
  }

  http.end();

  Serial.println("Pet sync ended");
}

void eventSync() {

  Serial.println("Starting event sync");

  loadingPopup = true;

  drawActivityUI();

  HTTPClient http;

  String url =
    String(supabaseUrl) +
    "/rest/v1/recent_events?select=*&limit=5";

  Serial.print("Event URL: ");
  Serial.println(url);

  http.begin(url);

  http.addHeader("apikey", apiKey);

  http.addHeader(
    "Authorization",
    "Bearer " + String(apiKey)
  );

  int httpResponseCode = http.GET();

  Serial.print("Event response code: ");
  Serial.println(httpResponseCode);

  loadingPopup = false;

  if (httpResponseCode > 0) {

    String response = http.getString();

    Serial.print("Event response: ");
    Serial.println(response);

    DynamicJsonDocument doc(2048);

    deserializeJson(doc, response);

    eventCount = 0;

    for (JsonObject item : doc.as<JsonArray>()) {

      if (eventCount >= 5) break;

      String eventUser =
        item["username"].as<String>();

      String action =
        item["action"].as<String>();

      String ago =
        item["time_ago"].as<String>();

      String actionText = "";

      if (action == "feed") actionText = "fed";
      else if (action == "rest") actionText = "rested";
      else actionText = "played with";

      events[eventCount].text =
        "\"" +
        eventUser +
        "\" " +
        actionText +
        " your bubbl (" +
        ago +
        ")";

      Serial.print("Event ");
      Serial.print(eventCount);
      Serial.print(": ");
      Serial.println(events[eventCount].text);

      eventCount++;
    }
  }

  else {

    Serial.println("Event sync failed");
  }

  http.end();

  Serial.println("Event sync ended");
}

void fetchUsers() {

  Serial.println("Fetching users");

  HTTPClient http;

  String url =
    String(supabaseUrl) +
    "/rest/v1/users?select=username";

  Serial.print("Users URL: ");
  Serial.println(url);

  http.begin(url);

  http.addHeader("apikey", apiKey);

  http.addHeader(
    "Authorization",
    "Bearer " + String(apiKey)
  );

  int httpResponseCode = http.GET();

  Serial.print("Users response code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {

    String response = http.getString();

    Serial.print("Users response: ");
    Serial.println(response);

    DynamicJsonDocument doc(2048);

    deserializeJson(doc, response);

    userCount = 0;

    for (JsonObject item : doc.as<JsonArray>()) {

      if (userCount >= 10) break;

      String fetchedUser =
        item["username"].as<String>();

      if (fetchedUser != username) {

        users[userCount] = fetchedUser;

        Serial.print("Added user: ");
        Serial.println(users[userCount]);

        userCount++;
      }
    }

    Serial.print("Total users loaded: ");
    Serial.println(userCount);
  }

  else {

    Serial.println("Fetch users failed");
  }

  http.end();

  Serial.println("Fetch users ended");
}

void sendNudge() {

  Serial.println("sendNudge() called");

  if (userCount == 0) {

    Serial.println("No users available");
    return;
  }

  Serial.print("Selected user index: ");
  Serial.println(selectedUser);

  Serial.print("Sending nudge to: ");
  Serial.println(users[selectedUser]);

  HTTPClient http;

  String url =
    String(supabaseUrl) +
    "/rest/v1/nudges";

  Serial.print("Nudge URL: ");
  Serial.println(url);

  http.begin(url);

  http.setTimeout(1000);

  http.addHeader("Content-Type","application/json");

  http.addHeader("apikey", apiKey);

  http.addHeader(
    "Authorization",
    "Bearer " + String(apiKey)
  );

  String body =
    "{\"sender\":\"" +
    username +
    "\",\"receiver\":\"" +
    users[selectedUser] +
    "\"}";

  Serial.print("Nudge body: ");
  Serial.println(body);

  Serial.println("Sending POST request");

  int httpResponseCode = http.POST(body);

  Serial.print("Nudge response code: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {

    String response = http.getString();

    Serial.print("Nudge response body: ");
    Serial.println(response);
  }

  if (httpResponseCode == 201) {

    popupMessage = "Buzz Sent!";

    Serial.println("Nudge success");
  }

  else {

    popupMessage = "Buzz Failed!";

    Serial.println("Nudge failed");
  }

  popupStart = millis();

  http.end();

  Serial.println("sendNudge() ended");
}

void checkForNudges() {

  if (millis() - lastNudgeCheck < nudgeCheckDelay) return;

  Serial.println("Checking for nudges");

  lastNudgeCheck = millis();

  if (WiFi.status() != WL_CONNECTED) {

    Serial.println("WiFi disconnected");
    return;
  }

  HTTPClient http;

  String url =
    String(supabaseUrl) +
    "/rest/v1/nudges?receiver=eq." +
    username +
    "&seen=eq.false&order=created_at.desc&limit=1";

  Serial.print("Check nudge URL: ");
  Serial.println(url);

  http.begin(url);

  http.setTimeout(2000);

  http.addHeader("apikey", apiKey);

  http.addHeader(
    "Authorization",
    "Bearer " + String(apiKey)
  );

  int httpResponseCode = http.GET();

  Serial.print("Check nudge response: ");
  Serial.println(httpResponseCode);

  if (httpResponseCode > 0) {

    String response = http.getString();

    Serial.print("Check nudge body: ");
    Serial.println(response);

    DynamicJsonDocument doc(1024);

    deserializeJson(doc, response);

    JsonArray arr = doc.as<JsonArray>();

    Serial.print("Nudge count: ");
    Serial.println(arr.size());

    if (arr.size() > 0) {

      JsonObject nudge = arr[0];

      String sender =
        nudge["sender"].as<String>();

      int nudgeId =
        nudge["id"];

      popupMessage =
        "\"" + sender + "\" nudged you";

      popupStart = millis();

      Serial.println("Nudge received");

      HTTPClient seenHttp;

      String seenUrl =
        String(supabaseUrl) +
        "/rest/v1/nudges?id=eq." +
        String(nudgeId);

      Serial.print("Seen URL: ");
      Serial.println(seenUrl);

      seenHttp.begin(seenUrl);

      seenHttp.setTimeout(2000);

      seenHttp.addHeader(
        "Content-Type",
        "application/json"
      );

      seenHttp.addHeader("apikey", apiKey);

      seenHttp.addHeader(
        "Authorization",
        "Bearer " + String(apiKey)
      );

      int patchCode = seenHttp.PATCH("{\"seen\":true}");

      Serial.print("Seen PATCH response: ");
      Serial.println(patchCode);

      seenHttp.end();
    }
  }

  else {

    Serial.println("Check nudge failed");
  }

  http.end();

  Serial.println("Finished checking nudges");
}

void handleJoystick() {

  int x = analogRead(JOY_X);

  int y = 4095 - analogRead(JOY_Y);

  if (millis() - lastMove < moveDelay) return;

  if (currentScreen == ACTIVITY) {

    if (eventCount == 0) return;

    if (y < 1200) {

      selectedEvent--;

      if (selectedEvent < 0) {
        selectedEvent = eventCount - 1;
      }

      Serial.print("Selected event: ");
      Serial.println(selectedEvent);

      textScrollX = 0;

      lastMove = millis();

      return;
    }

    else if (y > 2800) {

      selectedEvent++;

      if (selectedEvent >= eventCount) {
        selectedEvent = 0;
      }

      Serial.print("Selected event: ");
      Serial.println(selectedEvent);

      textScrollX = 0;

      lastMove = millis();

      return;
    }
  }

  if (currentScreen == PET_MENU) {

    if (y < 1200) {

      selectedPetOption--;

      if (selectedPetOption < 0) {
        selectedPetOption = petTotal - 1;
      }

      Serial.print("Pet menu selection: ");
      Serial.println(selectedPetOption);

      lastMove = millis();

      return;
    }

    else if (y > 2800) {

      selectedPetOption++;

      if (selectedPetOption >= petTotal) {
        selectedPetOption = 0;
      }

      Serial.print("Pet menu selection: ");
      Serial.println(selectedPetOption);

      lastMove = millis();

      return;
    }
  }

  if (currentScreen == NUDGE) {

    if (userCount == 0) return;

    if (y < 1200) {

      selectedUser--;

      if (selectedUser < 0) {
        selectedUser = userCount - 1;
      }

      Serial.print("Selected user: ");
      Serial.println(users[selectedUser]);

      lastMove = millis();

      return;
    }

    else if (y > 2800) {

      selectedUser++;

      if (selectedUser >= userCount) {
        selectedUser = 0;
      }

      Serial.print("Selected user: ");
      Serial.println(users[selectedUser]);

      lastMove = millis();

      return;
    }
  }

  if (currentScreen == MAIN) {

    if (y < 1200 &&
        currentFocus == MENU) {

      currentFocus = PET;

      Serial.println("Focus switched to PET");

      lastMove = millis();

      return;
    }

    if (y > 2800 &&
        currentFocus == PET) {

      currentFocus = MENU;

      Serial.println("Focus switched to MENU");

      lastMove = millis();

      return;
    }

    if (currentFocus == MENU) {

      if (x > 2800) {

        selected--;

        if (selected < 0) {
          selected = total - 1;
        }

        Serial.print("Main selection: ");
        Serial.println(options[selected]);

        lastMove = millis();

        return;
      }

      else if (x < 1200) {

        selected++;

        if (selected >= total) {
          selected = 0;
        }

        Serial.print("Main selection: ");
        Serial.println(options[selected]);

        lastMove = millis();

        return;
      }
    }
  }
}

void handleBtn() {

  bool currentState =
    digitalRead(JOY_BTN);

  if (currentState == LOW &&
      lastButtonState == HIGH) {

    Serial.println("Button pressed");

    buttonHoldStart = millis();

    longPressTriggered = false;
  }

  if (currentState == LOW &&
      !longPressTriggered &&
      millis() - buttonHoldStart > longPressDelay) {

    Serial.println("Long press triggered");

    longPressTriggered = true;

    if (currentScreen == NUDGE) {

      currentScreen = PET_MENU;

      Serial.println("Returning to PET_MENU");

      lastPress = millis();
    }

    else if (
      currentScreen == STATS ||
      currentScreen == ACTIVITY ||
      currentScreen == PET_MENU
    ) {

      currentScreen = MAIN;

      Serial.println("Returning to MAIN");

      lastPress = millis();
    }

    lastButtonState = currentState;

    return;
  }

  if (lastButtonState == LOW &&
      currentState == HIGH &&
      !longPressTriggered &&
      millis() - lastPress > pressDelay) {

    Serial.println("Short press detected");

    lastPress = millis();

    if (currentScreen == PET_MENU) {

      Serial.print("Pet menu enter: ");
      Serial.println(selectedPetOption);

      switch (selectedPetOption) {

        case 0:

          Serial.println("Opening STATS");

          petSync();

          currentScreen = STATS;

          break;

        case 1:

          Serial.println("Opening NUDGE");

          selectedUser = 0;

          currentScreen = NUDGE;

          break;

        case 2:

          Serial.println("Opening ACTIVITY");

          eventSync();

          selectedEvent = 0;

          textScrollX = 0;

          currentScreen = ACTIVITY;

          break;
      }

      return;
    }

    if (currentScreen == NUDGE) {

      Serial.println("Attempting sendNudge");

      sendNudge();

      return;
    }

    if (currentScreen == MAIN) {

      if (currentFocus == PET) {

        Serial.println("Opening PET_MENU");

        currentScreen = PET_MENU;

        selectedPetOption = 0;
      }

      else {

        Serial.print("Executing action: ");
        Serial.println(options[selected]);

        switch (selected) {

          case 0:
            feedPet();
            break;

          case 1:
            restPet();
            break;

          case 2:
            playPet();
            break;
        }
      }
    }
  }

  lastButtonState = currentState;
}

void loop() {

  audio.loop();

if (playNotification) {

  audio.connecttoFS(
    SPIFFS,
    "/nudge.wav"
  );

  playNotification = false;
}

  handleJoystick();

  handleBtn();

  checkForNudges();

  if (millis() - lastFrame >= frameDelay) {

    lastFrame = millis();

    if (currentScreen == MAIN) {
      drawMainUI();
    }

    else if (currentScreen == PET_MENU) {
      drawPetMainUI();
    }

    else if (currentScreen == STATS) {
      drawStatsUI();
    }

    else if (currentScreen == ACTIVITY) {
      drawActivityUI();
    }

    else if (currentScreen == NUDGE) {
      drawNudgeUI();
    }
  }

  if (millis() - lastPrint > 2500) {

    lastPrint = millis();

    Serial.println("Loop heartbeat");
  }
}