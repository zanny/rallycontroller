# RallyNavi Controller — ESP32-S3 Super Mini BLE Keyboard

A Bluetooth LE keyboard controller for rally and enduro navigation apps (DMD2, TwoNav, Scenic, etc.), built on an ESP32-S3 Super Mini.

Originally based on work by **NordicRally**, fixed and extended by **Stathis**.

---

## Features

- **5 button BLE keyboard** — connects to Android tablets/phones as a wireless keyboard
- **Tap and long press** — each button sends a different keystroke depending on press duration
- **Two profiles** — Profile 1 for navigation with distinct tap/hold keys, Profile 2 for rapid repeat mode
- **Profile memory** — active profile saved to flash and restored on next boot
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

## Enclosure 3D Files

This enclosure design was originally created by Stig Hansen on Thingiverse:
https://www.thingiverse.com/thing:4370615

Licensed under Creative Commons Attribution (CC-BY).
A copy is included in this repository for archival purposes in case the original source becomes unavailable.

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

## Operating Instructions

### Overview
The RallyNavi Controller connects to your Android tablet or phone as a Bluetooth keyboard. Once paired, it sends keystrokes to your navigation app when buttons are pressed. Each button supports a short tap and a long press, each sending a different key. Two profiles are available with different key mappings, switchable on the fly.

### Bluetooth device name
The controller broadcasts different names depending on the active profile, visible in your Android Bluetooth settings:

| Profile | Broadcast name |
|---------|---------------|
| Profile 1 | **RallyNavi P1** |
| Profile 2 | **RallyNavi P2** |

This allows you to confirm the active profile at a glance from your tablet's Bluetooth settings or notification shade.

### Profile 1 — Navigation mode (default)
Tap and long press send different keys, designed for mapping to navigation functions in apps like DMD2.

| Button | Short tap | Long press (holds and repeats) |
|--------|-----------|-------------------------------|
| Button 1 | `1` | `6` |
| Button 2 | `2` | `+` |
| Button 3 | `3` | `-` |
| Button 4 | `4` | `7` |
| Button 5 | `5` | `8` |

### Profile 2 — Rapid repeat mode
Tap and long press send the same key. Long press repeats the key continuously while held.

| Button | Short tap | Long press (holds and repeats) |
|--------|-----------|-------------------------------|
| Button 1 | `1` | `1` (repeats) |
| Button 2 | `2` | `2` (repeats) |
| Button 3 | `3` | `3` (repeats) |
| Button 4 | `4` | `4` (repeats) |
| Button 5 | `5` | `5` (repeats) |

### Switching profiles
Hold **Button 1 + Button 5 simultaneously for 3 seconds** then release. The profile switches immediately, the new profile is saved to flash, and the broadcast name updates. Serial Monitor confirms the switch:
```
* Switched to Profile 2 — broadcasting as RallyNavi P2 *
< Profile saved to flash >
```

### Using with DMD2
1. Pair the controller with your Android device — look for **RallyNavi P1** in Bluetooth settings
2. Open DMD2 → Settings → Keyboard mapping
3. Press each button when prompted to assign it
4. Map each key to your desired function (pan, zoom, recalculate, follow toggle, etc.)

The controller presents itself as a standard Bluetooth keyboard so it works with any app that supports keyboard shortcuts.

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

// Profile 2 — tap and hold keys (same key, hold repeats)
#define KEY_P2_BTN1       '1'
#define KEY_P2_BTN1_HOLD  '1'
// etc.
```

### Timing
```cpp
#define RECONNECT_DELAY_MS   1000  // startup delay before advertising (ms)
#define DEBOUNCE_MS          100   // button debounce interval (ms)
#define LONG_PRESS_MS        500   // hold duration to trigger long press (ms)
#define HOLD_REPEAT_MS       300   // repeat speed while held (ms)
#define PROFILE_SWITCH_MS    3000  // hold time to switch profile (ms)
```

### Pin assignments
```cpp
const int Button1 = 1;
const int Button2 = 2;
const int Button3 = 4;
const int Button4 = 5;
const int Button5 = 6;
```

### Device names
```cpp
#define DeviceNameP1  "RallyNavi P1"
#define DeviceNameP2  "RallyNavi P2"
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

| Library | Version | Install via |
|---------|---------|-------------|
| ESP32-BLE-Keyboard | **0.2.3** | Download from https://github.com/T-vK/ESP32-BLE-Keyboard/releases/tag/0.2.3 → Sketch → Include Library → Add .ZIP |
| Bounce2 | Latest | Arduino Library Manager |
| Preferences | Built-in | No install needed — part of ESP32 core |

### ESP32 core version
Use **esp32 by Espressif Systems version 2.0.17** — the library is not compatible with core 3.x.

---

## Uploading

If the board doesn't enter bootloader mode automatically:
1. Click **Upload** in Arduino IDE
2. When you see `Connecting...` hold the **BOOT** button on the board
3. Release once you see `Configuring flash size...`

### First upload after name change
If upgrading from a previous version with a different device name (e.g. "RCntrl P.1"), you will need to:
1. Go to Android Bluetooth settings
2. Forget the old device
3. Pair fresh with **RallyNavi P1**

This is a one-time step only.

---

## Serial Monitor

Connect via USB and open Serial Monitor at **115200 baud** to debug:

```
<! Starting RallyNavi Controller (ESP32-S3) !>
< Loaded profile: 1 >
< BLE Keyboard started — waiting 1s before advertising >
< Now advertising as RallyNavi P1 >
[ Status: waiting for connection... ]
[ Status: CONNECTED — Profile 1 ]
Button1 TAP → '1'
Button2 HOLD → '+'
Button2 HOLD repeat → '+'
* Switched to Profile 2 — broadcasting as RallyNavi P2 *
< Profile saved to flash >
```

---

## Known issues and notes

- **BleKeyboard library version** — must use version **0.2.3** specifically. Versions 0.3.0 and above have a reconnection bug where Android reconnects successfully but buttons stop working until the device is forgotten and re-paired. Version 0.2.3 fixes this.
- **Android stale session** — a 1 second startup delay (`RECONNECT_DELAY_MS`) is included to give Android time to clear any stale BLE session after an abrupt power cut. Increase if reconnection issues occur.
- **Profile memory** — profile is saved to flash on every profile switch. Survives power cycles and code uploads. Erased only if uploading with "Erase All Flash" enabled in Arduino IDE.
- **ESP32 core 3.x** — not compatible. Use core 2.0.17.

---

## Credits

- Original code: **NordicRally**
- Hardware adaptation and fixes: **Stathis**
- Bug fixes, long press, profile system, reconnect handling: **Claude (Anthropic)**
- BLE Keyboard library: **T-vK** — https://github.com/T-vK/ESP32-BLE-Keyboard
- Bounce2 library: **Thomas O Fredericks**
