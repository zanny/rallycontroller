# Rally Controller — ESP32-S3 Super Mini BLE Keyboard

A Bluetooth LE keyboard controller for rally and enduro navigation apps (DMD2, TwoNav, Scenic, etc.), built on an ESP32-S3 Super Mini.

Originally based on work by **NordicRally**, fixed and extended by **Thomas Styles**.

---

## Features

- **5 button BLE keyboard** — connects to Android tablets/phones as a wireless keyboard
- **Tap and long press** — each button sends a different keystroke depending on press duration
- **Two profiles** — switch between navigation keys and media keys without re-flashing
- **Auto reconnect** — reconnects automatically after power loss or Bluetooth drop
- **Configurable** — all key assignments and timing settings at the top of the code
- **Debounce** — clean single-fire on all buttons, no accidental multi-press

---

## Hardware

| Component | Details |
|-----------|---------|
| Microcontroller | ESP32-S3 Super Mini |
| Buttons | 5x momentary push buttons |
| Connection | Shared GND, one wire per button to GPIO |
| Power | USB-C or 5V pin |

### Recommended additions for field use
- **CD42 module** — LiPo charger + 5V boost converter
- **LiPo battery** — 3.7V 1000mAh
- **ABS enclosure** — for weatherproofing and vibration resistance

---

## Wiring

```
ESP32-S3 Super Mini
┌──────────────────────┐
│  GPIO1  ─────────────── Button 1
│  GPIO2  ─────────────── Button 2
│  GPIO4  ─────────────── Button 3
│  GPIO5  ─────────────── Button 4
│  GPIO6  ─────────────── Button 5
│                          (all other button legs → shared GND)
│  GND    ─────────────── Shared negative
└──────────────────────┘
```

No resistors needed — buttons use internal pull-up resistors via `INPUT_PULLUP`.

---

## Button Mapping

### Profile 1 (default)

| Button | Short tap | Long press (hold >500ms, repeats) |
|--------|-----------|----------------------------------|
| Button 1 | `1` | `6` |
| Button 2 | `2` | `+` |
| Button 3 | `3` | `-` |
| Button 4 | `4` | `7` |
| Button 5 | `5` | `8` |

### Profile 2 (media keys)

| Button | Short tap |
|--------|-----------|
| Button 1 | Previous Track |
| Button 2 | Next Track |
| Button 3 | Volume Down |
| Button 4 | Volume Up |
| Button 5 | Mute |

### Switch profiles
Hold **Button 1 + Button 5 simultaneously for 5 seconds**. Serial Monitor confirms the switch.

---

## Configuration

All settings are at the top of the sketch — no need to dig through the code.

### Key assignments
```cpp
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
```

### Timing
```cpp
#define RECONNECT_DELAY_MS  7000  // startup delay before advertising (ms)
#define DEBOUNCE_MS         100   // button debounce interval (ms)
#define LONG_PRESS_MS       500   // hold duration to trigger long press (ms)
#define HOLD_REPEAT_MS      300   // repeat speed while held (ms)
```

### Pin assignments
```cpp
const int Button1 = 1;
const int Button2 = 2;
const int Button3 = 4;
const int Button4 = 5;
const int Button5 = 6;
```

---

## Arduino IDE Setup

### Board settings

| Setting | Value |
|---------|-------|
| Board | ESP32S3 Dev Module |
| USB CDC On Boot | Enabled |
| Partition Scheme | Huge APP (3MB No OTA/1MB SPIFFS) |
| Upload Speed | 921600 |

### Required libraries

| Library | Author | Install via |
|---------|--------|-------------|
| ESP32-BLE-Keyboard | T-vK | [GitHub ZIP](https://github.com/T-vK/ESP32-BLE-Keyboard) → Sketch → Include Library → Add .ZIP |
| Bounce2 | Thomas O Fredericks | Arduino Library Manager |

### ESP32 core version
Use **esp32 by Espressif Systems version 2.0.17** — the library is not compatible with core 3.x.

---

## Uploading

If the board doesn't enter bootloader mode automatically:
1. Click **Upload** in Arduino IDE
2. When you see `Connecting...` hold the **BOOT** button on the board
3. Release once you see `Configuring flash size...`

---

## Usage with DMD2

1. Pair the controller with your Android device via Bluetooth — look for **"RCntrl P.1"**
2. Open DMD2 → Settings → Keyboard mapping
3. Press each button when prompted to assign it
4. Map each key to your desired function (pan, zoom, recalculate, follow toggle, etc.)

The controller presents itself as a standard Bluetooth keyboard so it works with any app that supports keyboard shortcuts.

---

## Serial Monitor

Connect via USB and open Serial Monitor at **115200 baud** to debug:

```
<! Starting Rally Controller (ESP32-S3) !>
< BLE Keyboard started — waiting 7s before advertising >
< Now advertising — waiting for connection >
[ Status: waiting for connection... ]
[ Status: CONNECTED ]
Button1 TAP → '1'
Button2 HOLD → '+'
Button2 HOLD repeat → '+'
```

---

## Known issues and notes

- **Android stale session** — if power is cut abruptly Android may reconnect with a stale BLE session. The 7 second startup delay (`RECONNECT_DELAY_MS`) gives Android time to clear it. Increase this value if the issue persists.
- **Profile memory** — the active profile resets to Profile 1 on power cycle. Profile is not saved to flash.
- **ESP32 core 3.x** — not compatible. Use core 2.0.17.

---

## Credits

- Original code: **NordicRally**
- Hardware adaptation and fixes: **Thomas Styles**
- Bug fixes, long press, profile system, reconnect handling: **Claude (Anthropic)**
- BLE Keyboard library: **T-vK** — https://github.com/T-vK/ESP32-BLE-Keyboard
- Bounce2 library: **Thomas O Fredericks**

