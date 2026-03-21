//  Rally Controller for ESP32-S3 Super Mini
//    Stathis
//    Based on work by NordicRally
//    Fixed & updated by Claude (Anthropic) - Mar 2026
//
//  Button behaviour Profile 1:
//    Button 1 — tap = '1' | hold = '6' (repeats)
//    Button 2 — tap = '2' | hold = '+' (repeats)
//    Button 3 — tap = '3' | hold = '-' (repeats)
//    Button 4 — tap = '4' | hold = '7' (repeats)
//    Button 5 — tap = '5' | hold = '8' (repeats)
//
//  Button behaviour Profile 2:
//    Button 1 — tap = '1' | hold = '1' (repeats)
//    Button 2 — tap = '2' | hold = '2' (repeats)
//    Button 3 — tap = '3' | hold = '3' (repeats)
//    Button 4 — tap = '4' | hold = '4' (repeats)
//    Button 5 — tap = '5' | hold = '5' (repeats)
//
//  Profile toggle — hold Button1 + Button5 for 3 seconds
//  Active profile is saved to flash and restored on next boot

// ---------------------------------------------------------------------------
// Libraries
// ---------------------------------------------------------------------------
#include <BleKeyboard.h>   // ESP32-BLE-Keyboard by T-vK — use version 0.2.3
#include <Bounce2.h>       // Bounce2 by Thomas O Fredericks
#include <Preferences.h>   // ESP32 built-in — for saving profile to flash

// ---------------------------------------------------------------------------
// BLE Device Identity
// Profile names are broadcast to Android so you can see active profile
// in Bluetooth settings
// ---------------------------------------------------------------------------
#define DeviceNameP1       "RallyNavi P1"
#define DeviceNameP2       "RallyNavi P2"
#define DeviceManufacturer "S.R.I."
#define BatteryLevel       69

// ---------------------------------------------------------------------------
// KEY ASSIGNMENTS — Edit these to change what each button sends
//
// Profile 1 — tap and hold send different keys
// Profile 2 — tap and hold send the same key (hold repeats)
//
// Keys must be regular char keys:
//   e.g. '1', '2', '+', '-', 'a', KEY_F1 ... KEY_F12
// ---------------------------------------------------------------------------

// Profile 1 — tap keys
#define KEY_P1_BTN1       '1'
#define KEY_P1_BTN2       '2'
#define KEY_P1_BTN3       '3'
#define KEY_P1_BTN4       '4'
#define KEY_P1_BTN5       '5'

// Profile 1 — hold keys (repeats while held)
#define KEY_P1_BTN1_HOLD  '6'
#define KEY_P1_BTN2_HOLD  '+'
#define KEY_P1_BTN3_HOLD  '-'
#define KEY_P1_BTN4_HOLD  '7'
#define KEY_P1_BTN5_HOLD  '8'

// Profile 2 — tap keys
#define KEY_P2_BTN1       '1'
#define KEY_P2_BTN2       '2'
#define KEY_P2_BTN3       '3'
#define KEY_P2_BTN4       '4'
#define KEY_P2_BTN5       '5'

// Profile 2 — hold keys (same as tap — repeats same key while held)
#define KEY_P2_BTN1_HOLD  '1'
#define KEY_P2_BTN2_HOLD  '2'
#define KEY_P2_BTN3_HOLD  '3'
#define KEY_P2_BTN4_HOLD  '4'
#define KEY_P2_BTN5_HOLD  '5'

// ---------------------------------------------------------------------------
// TIMING SETTINGS
//   RECONNECT_DELAY_MS   — delay on startup before advertising
//                          gives Android time to clear stale BLE session
//   DEBOUNCE_MS          — increase if buttons still multifire
//   LONG_PRESS_MS        — how long to hold before long press activates
//   HOLD_REPEAT_MS       — how fast key repeats while held
//   PROFILE_SWITCH_MS    — how long to hold Button1+Button5 to switch profile
// ---------------------------------------------------------------------------
#define RECONNECT_DELAY_MS   1000
#define DEBOUNCE_MS          100
#define LONG_PRESS_MS        500
#define HOLD_REPEAT_MS       300
#define PROFILE_SWITCH_MS    3000

// ---------------------------------------------------------------------------
// Pin Assignments  —  ESP32-S3 Super Mini
// ---------------------------------------------------------------------------
const int Button1 = 1;
const int Button2 = 2;
const int Button3 = 4;
const int Button4 = 5;
const int Button5 = 6;

// ---------------------------------------------------------------------------
// Button Config
// ---------------------------------------------------------------------------
#define NUM_BUTTONS 5
const uint8_t BUTTON_PINS[NUM_BUTTONS] = {Button1, Button2, Button3, Button4, Button5};

Bounce* buttons = new Bounce[NUM_BUTTONS];

BleKeyboard bleKeyboard(DeviceNameP1, DeviceManufacturer, BatteryLevel);

// ---------------------------------------------------------------------------
// Profile state
// ---------------------------------------------------------------------------
bool toggle_mode = false;   // false = Profile 1, true = Profile 2
Preferences preferences;

void loadProfile() {
  preferences.begin("rally", true);  // read-only
  toggle_mode = preferences.getBool("profile", false);
  preferences.end();
  Serial.println("< Loaded profile: " + String(toggle_mode ? "2" : "1") + " >");
}

void saveProfile() {
  preferences.begin("rally", false);  // read-write
  preferences.putBool("profile", toggle_mode);
  preferences.end();
  Serial.println("< Profile saved to flash >");
}

void applyProfileName() {
  if (toggle_mode) {
    esp_ble_gap_set_device_name(DeviceNameP2);
    Serial.println("* Switched to Profile 2 — broadcasting as " + String(DeviceNameP2) + " *");
  } else {
    esp_ble_gap_set_device_name(DeviceNameP1);
    Serial.println("* Switched to Profile 1 — broadcasting as " + String(DeviceNameP1) + " *");
  }
}

// ---------------------------------------------------------------------------
// Long press state tracking — one set of variables per button
// ---------------------------------------------------------------------------
struct ButtonState {
  unsigned long pressTime     = 0;
  bool longPressActive        = false;
  unsigned long lastRepeat    = 0;
};

ButtonState btnState[NUM_BUTTONS];

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("<! Starting RallyNavi Controller (ESP32-S3) !>");

  // Load saved profile from flash
  loadProfile();

  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].attach(BUTTON_PINS[i], INPUT_PULLUP);
    buttons[i].interval(DEBOUNCE_MS);
  }

  // Start with correct device name for loaded profile
  if (toggle_mode) {
    BleKeyboard tempKeyboard(DeviceNameP2, DeviceManufacturer, BatteryLevel);
    bleKeyboard = tempKeyboard;
  }

  bleKeyboard.begin();
  Serial.println("< BLE Keyboard started — waiting " + String(RECONNECT_DELAY_MS / 1000) + "s before advertising >");
  delay(RECONNECT_DELAY_MS);
  Serial.println("< Now advertising as " + String(toggle_mode ? DeviceNameP2 : DeviceNameP1) + " >");
}

// ---------------------------------------------------------------------------
// Handle a single button — tap and long press logic
// ---------------------------------------------------------------------------
void handleButton(int index, char tapKey, char holdKey) {

  // Button just pressed — start timer
  if (buttons[index].fell()) {
    btnState[index].pressTime       = millis();
    btnState[index].longPressActive = false;
  }

  // Button held — check for long press threshold
  if (buttons[index].read() == LOW) {
    unsigned long heldFor = millis() - btnState[index].pressTime;

    if (!btnState[index].longPressActive && heldFor > LONG_PRESS_MS) {
      // Just crossed threshold — fire once immediately
      btnState[index].longPressActive = true;
      btnState[index].lastRepeat      = millis();
      Serial.println("Button" + String(index + 1) + " HOLD → '" + holdKey + "'");
      bleKeyboard.write(holdKey);

    } else if (btnState[index].longPressActive && (millis() - btnState[index].lastRepeat > HOLD_REPEAT_MS)) {
      // Repeat while still held
      btnState[index].lastRepeat = millis();
      Serial.println("Button" + String(index + 1) + " HOLD repeat → '" + holdKey + "'");
      bleKeyboard.write(holdKey);
    }
  }

  // Button released
  if (buttons[index].rose()) {
    if (!btnState[index].longPressActive) {
      Serial.println("Button" + String(index + 1) + " TAP → '" + tapKey + "'");
      bleKeyboard.write(tapKey);
    }
    btnState[index].longPressActive = false;
  }
}

// ---------------------------------------------------------------------------
// Loop
// ---------------------------------------------------------------------------
void loop() {

  // Print connection status every 3 seconds
  static unsigned long lastStatusPrint = 0;
  if (millis() - lastStatusPrint > 3000) {
    lastStatusPrint = millis();
    if (bleKeyboard.isConnected()) {
      Serial.println("[ Status: CONNECTED — Profile " + String(toggle_mode ? "2" : "1") + " ]");
    } else {
      Serial.println("[ Status: waiting for connection... ]");
    }
  }

  // Update all Bounce instances
  for (int i = 0; i < NUM_BUTTONS; i++) {
    buttons[i].update();
  }

  // Profile toggle: hold Button1 + Button5 for PROFILE_SWITCH_MS
  if ((digitalRead(Button1) == LOW) && (digitalRead(Button5) == LOW)) {
    delay(PROFILE_SWITCH_MS);
    if ((digitalRead(Button1) == LOW) && (digitalRead(Button5) == LOW)) {
      toggle_mode = !toggle_mode;
      applyProfileName();
      saveProfile();
      // Wait for release to avoid re-triggering
      while ((digitalRead(Button1) == LOW) || (digitalRead(Button5) == LOW)) {
        delay(10);
      }
    }
  }

  // Only send keys when BLE is connected
  if (!bleKeyboard.isConnected()) {
    return;
  }

  // Handle all 5 buttons with correct profile keys
  if (!toggle_mode) {
    // Profile 1
    handleButton(0, KEY_P1_BTN1, KEY_P1_BTN1_HOLD);
    handleButton(1, KEY_P1_BTN2, KEY_P1_BTN2_HOLD);
    handleButton(2, KEY_P1_BTN3, KEY_P1_BTN3_HOLD);
    handleButton(3, KEY_P1_BTN4, KEY_P1_BTN4_HOLD);
    handleButton(4, KEY_P1_BTN5, KEY_P1_BTN5_HOLD);
  } else {
    // Profile 2
    handleButton(0, KEY_P2_BTN1, KEY_P2_BTN1_HOLD);
    handleButton(1, KEY_P2_BTN2, KEY_P2_BTN2_HOLD);
    handleButton(2, KEY_P2_BTN3, KEY_P2_BTN3_HOLD);
    handleButton(3, KEY_P2_BTN4, KEY_P2_BTN4_HOLD);
    handleButton(4, KEY_P2_BTN5, KEY_P2_BTN5_HOLD);
  }

}
