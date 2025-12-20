<h1 align="center">
  <br>
  <a href="https://github.com/warwick320/Nova-X-5G-Deauther"><img src="https://github.com/warwick320/Nova-X-5G-Deauther/blob/main/img/logo.png" alt="Nova-X" width="200"></a>
  <br>
  Nova-X • ᚾᚬᚢᛅ ᛋ
  <br>
</h1>

![C++](https://img.shields.io/badge/C%2B%2B-00599C?logo=c%2B%2B&logoColor=white)
![ESP32](https://img.shields.io/badge/ESP32-2C2D72?logo=espressif&logoColor=white)
![Arduino](https://img.shields.io/badge/Arduino-00979D?logo=arduino&logoColor=white)
![License:  BSD 2-Clause](https://img.shields.io/badge/License-BSD%202--Clause-orange)
![Version](https://img.shields.io/badge/version-0.1.0--beta-blue)
![GitHub stars](https://img.shields.io/github/stars/warwick320/Nova-X-5G-Deauther?style=social)
![GitHub forks](https://img.shields.io/github/forks/warwick320/Nova-X-5G-Deauther?style=social)
![GitHub issues](https://img.shields.io/github/issues/warwick320/Nova-X-5G-Deauther)
![GitHub last commit](https://img.shields.io/github/last-commit/warwick320/Nova-X-5G-Deauther)

WiFi security testing and BLE advertisement tool specifically designed for ESP32C5 platform with U8g2 OLED display support.

## Table of Contents

- [Overview](#overview)
- [Features](#features)
- [Hardware Requirements](#hardware-requirements)
- [Pin Configuration](#pin-configuration)
- [Software Requirements](#software-requirements)
- [Installation](#installation)
- [Project Structure](#project-structure)
- [Usage](#usage)
- [Menu System](#menu-system)
- [Technical Details](#technical-details)
- [Legal Disclaimer](#legal-disclaimer)
- [License](#license)
- [Contact](#contact)

## Features

### WiFi Security Testing

**Deauthentication Attacks**
- All Access Points mode
- Channel-specific targeting
- Selected AP targeting

**Authentication Flooding**
- Mass authentication frame transmission
- Channel-based authentication attacks
- Selective AP authentication flooding

**Association Flooding**
- Association request flood attacks
- Channel-specific association attacks
- Targeted association frame injection

**Beacon Frame Manipulation**
- All SSID duplication
- Selected SSID cloning
- Random SSID generation
- Channel-specific beacon flooding

### BLE Advertisement Spoofing

- iOS device emulation support
- Samsung device emulation support

### User Interface

- SSD1306 OLED display (128x64)
- 4-button tactile navigation system

### Network Scanning

**Dual-Band WiFi Support**
- 2.4GHz:  Channels 1-14
- 5GHz:  Channels 36-165
- Progressive scanning algorithm
- RSSI measurement
- Channel mapping
- Encryption detection

## Hardware Requirements

### Microcontroller
- ESP32C5 module

### Display
- SSD1306 OLED (128x64 pixels)
- I2C interface

### Buttons
- 4x Tactile push buttons

## Pin Configuration

```cpp
// I2C Display Pins
I2C_SDA = GPIO 26
I2C_SCL = GPIO 25

// Button Pins
BTN_UP   = GPIO 24
BTN_DOWN = GPIO 23
BTN_OK   = GPIO 28
BTN_BACK = GPIO 10

// OLED Reset
OLED_RESET = -1 (not used)
```

### Wiring Diagram

```
ESP32C5          SSD1306 OLED
GPIO 26  ------>  SDA
GPIO 25  ------>  SCL
3.3V     ------>  VCC
GND      ------>  GND

ESP32C5          Buttons
GPIO 24  ------>  UP Button    ----> GND
GPIO 23  ------>  DOWN Button  ----> GND
GPIO 28  ------>  OK Button    ----> GND
GPIO 10  ------>  BACK Button  ----> GND
```

## Software Requirements

### Required Libraries

```cpp
// Core Libraries
- U8g2lib              // OLED display driver
- NimBLE-Arduino       // Bluetooth Low Energy
- WiFi                 // ESP32 WiFi library
- esp_wifi             // Low-level WiFi functions

// Standard Libraries
- Arduino.h
- Wire.h (I2C)
- vector (STL)
- string (STL)
- map (STL)
```

### Library Installation

**Method 1: Arduino Library Manager**
```
Tools > Manage Libraries
Search:  "U8g2"        Install:  U8g2 by oliver
Search: "NimBLE"      Install: NimBLE-Arduino by h2zero
```

**Method 2: Manual Installation**
```bash
cd ~/Arduino/libraries/
git clone https://github.com/olikraus/u8g2.git
git clone https://github.com/h2zero/NimBLE-Arduino.git
```

## Installation

### Step 1: Board Setup

1. Open Arduino IDE
2. Go to `File > Preferences`
3. Add ESP32 board URL to Additional Board Manager URLs: 
```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
```
4. Open `Tools > Board > Board Manager`
5. Search "esp32" and install "esp32 by Espressif Systems"
6. Select `Tools > Board > ESP32 Arduino > ESP32C5 Dev Module`

### Step 2: Upload Sketch

> Before upload sketch you need to patch your esp32 compiler - [patcher](https://github.com/7h30th3r0n3/Evil-M5Project/tree/main/utilities/deauth_prerequisites) - ex) [my platform.txt](https://github.com/warwick320/Nova-X-5G-Deauther/blob/main/platform.txt)

1. Open `nova-x-esp32c5/nova-x-esp32c5.ino`
2. Configure board settings:
   - Board: ESP32C5 Dev Module
   - Upload Speed: 115200
   - Flash Frequency:  80MHz
   - Flash Mode: QIO
   - Partition Scheme:  Huge APP (3MB No OTA/1MB SPIFFS)
3. Select correct COM port
4. Click Upload

### Step 3: Serial Monitor

```
Tools > Serial Monitor
Baud Rate: 115200
```


## Usage

### Navigation

- **UP Button**: Move selection up
- **DOWN Button**: Move selection down
- **OK Button**:  Confirm selection / Start action
- **BACK Button**: Return to previous menu / Stop action

### Basic Workflow

**1. Scanning Networks**
```
Main Menu > Scan
```

**2. Selecting Targets**
```
Main Menu > Settings > Select APs
```

**3. Running Attacks**
```
Main Menu > Exploit > [Attack Type] > [Mode]
```

## Menu System

```
Nova-X ESP32C5
│
├── Exploit
│   ├── Deauth
│   │   ├── All APs
│   │   ├── Channel
│   │   └── Selected
│   │
│   ├── Auth
│   │   ├── All APs
│   │   ├── Channel
│   │   └── Selected
│   │
│   ├── Assoc
│   │   ├── All APs
│   │   ├── Channel
│   │   └── Selected
│   │
│   ├── Beacon
│   │   ├── All SSIDs Dupe
│   │   ├── Selected Dupe
│   │   ├── Random
│   │   └── Channel
│   │
│   └── B. T Adv
│       ├── Samsung
│       └── IOS
│
├── Scan
│
├── Settings
│   └── Select APs
│
└── About
```

## Technical Details

### WiFi Frame Structures

**Deauthentication Frame (26 bytes)**
```cpp
Frame Control:     0xC0 0x00
Duration:          0x3A 0x01
Destination MAC:   FF:FF:FF:FF:FF:FF (broadcast)
Source MAC:        [Target AP BSSID]
BSSID:             [Target AP BSSID]
Sequence:          0x00 0x00
Reason Code:       0x07 0x00
```

**Authentication Frame**
```cpp
Frame Control:     0xB0 0x00
Algorithm:         0x00 0x00 (Open System)
Sequence Number:   0x01 0x00
Status Code:       0x00 0x00
```

**Association Request Frame**
```cpp
Frame Control:     0x00 0x00
Capability Info:   0x21 0x04
Listen Interval:   0x0A 0x00
Tagged Parameters:  SSID, Supported Rates
```

**Beacon Frame**
```cpp
Frame Control:     0x80 0x00
Fixed Parameters: 
  - Timestamp
  - Beacon Interval:  0x64 0x00
  - Capability Info:  0x21 0x00
Tagged Parameters: SSID, Rates, Channel
```

### Channel Configuration

**2.4GHz Band (14 channels)**
```cpp
1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14
```

**5GHz Band (23 channels)**
```cpp
36, 40, 44, 48, 52, 56, 60, 64,
100, 104, 108, 112, 116, 124, 128,
132, 136, 140,
149, 153, 157, 161, 165
```

**Total: 37 channels**

### BLE Advertisement Data

**iOS Device Structure (31 bytes)**
```cpp
Size:          0x1e
Type:         0xFF (Manufacturer Specific)
Company ID:   0x4C 0x00 (Apple Inc.)
Subtype:      0x07 0x19
Device Type:  [varies by device]
```

**Samsung Device Structure (15 bytes)**
```cpp
Size:         0x0E
Type:         0xFF (Manufacturer Specific)
Company ID:   0x75 0x00 (Samsung Electronics)
Subtype:      0x01
Model:        [device-specific byte]
```

### MAC Address Management

```cpp
#define STORE_LEN 64  // MAC address cache size

bool checkedMac(const uint8_t* mac);
void storeMac(const uint8_t* mac);
void clearMacStored();
```

### Attack Configuration

```cpp
#define STORE_LEN 64        // MAC address cache
#define PER_PKT 3           // Packets per target
#define MAX_TX_POWER ESP_PWR_LVL_P20  // 20dBm transmit power
```

## Legal Disclaimer

**IMPORTANT: READ BEFORE USE**

This tool is designed exclusively for: 
- Authorized penetration testing
- Educational purposes in controlled environments
- Network administration on owned infrastructure
- Security research with proper authorization

**Legal Warnings**

Unauthorized use of this software may violate laws including: 

- **United States**: Computer Fraud and Abuse Act (CFAA), Federal Communications Act
- **European Union**: GDPR, Computer Misuse Act, national telecommunications laws
- **International**: Local wireless communication regulations and computer crime statutes

WiFi deauthentication, association flooding, and unauthorized network interference are **illegal activities** in most jurisdictions without explicit written permission from network owners.

**Penalties may include**:
- Criminal prosecution
- Civil liability
- Significant fines
- Imprisonment

**User Responsibility**

By using this software, you acknowledge: 
1. You have explicit written authorization to test target networks
2. You understand applicable laws in your jurisdiction
3. You accept full legal responsibility for your actions
4. The authors bear no liability for misuse or legal consequences

**Use this tool only on networks you own or have explicit written permission to test.**

## License

This project is licensed under the **BSD 2-Clause "Simplified" License**.

```
Copyright (c) 2024, warwick320
All rights reserved. 

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
THE POSSIBILITY OF SUCH DAMAGE.
```

See the [LICENSE](../LICENSE) file for full details.

## Contact

- **Repository**: [github.com/warwick320/Nova-X-5G-Deauther](https://github.com/warwick320/Nova-X-5G-Deauther)
- **Issues**: [GitHub Issues](https://github.com/warwick320/Nova-X-5G-Deauther/issues)
- **Discord**: zw.warwick

---

*Remember: With great power comes great responsibility. Use this tool ethically and legally.*
