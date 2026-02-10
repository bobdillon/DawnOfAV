# DawnOfAV

A sketch and instructions for emitting NTSC video color patterns and mono audio via Arduino ESP32.

## Mandatory Setup for Arduino IDE

**One-time setup:**

1. Add the ESP32 URL to Arduino IDE:
   - Go to `File → Preferences → Additional Boards Manager URLs`
   - Add: `https://dl.espressif.com/dl/package_esp32_index.json`

2. Install **esp32 by Espressif Systems V1.0.4** in Boards Manager

3. Select partition scheme: **"No OTA (2MB APP/2MB SPIFFS)"** or **"Huge APP (3MB No OTA/1MB SPIFFS)"**

4. In Arduino IDE, select **"ESP32 Dev Module"** as the board

## Bill of Materials

- 2× momentary push buttons
- 1× project box
- 1× toggle switch
- 1× 3.7V rechargeable LiPo battery
- 1× J5019 (or similar) LiPo charge circuit
- 1× Arduino ESP32 Dev Kit V1
- 1× USB female jack
- 2× phono jacks (composite video and audio)

## Wiring

### Composite Video Jack
| Pin | Connection |
|-----|------------|
| Center pin | GPIO 25 |
| Ground/shell | GND |

### Audio Jack
| Pin | Connection |
|-----|------------|
| Tip (pin 1) | GND |
| Ring (pin 2) | GPIO 27 |

### Pattern Button
Cycles through 11 test patterns in a loop, one move per press.

| Pin | Connection |
|-----|------------|
| Pin 1 | GND |
| Pin 2 | GPIO 14 |

### Audio Button
Toggle on/off a continuous 1 kHz tone.

| Pin | Connection |
|-----|------------|
| Pin 1 | GND |
| Pin 2 | GPIO 13 |

### J5019 Charge Circuit
Read the PCB markings for proper connections. The toggle switch should bridge the ground of the voltage source to the Arduino ground when on. The USB female jack goes here for charging and should be mounted externally.
