# How to build PlatformIO based project

## Setup Instructions

1. [Install PlatformIO Core](https://docs.platformio.org/page/core.html)
2. Download [development platform with examples](https://github.com/platformio/platform-espressif32/archive/develop.zip)
3. Extract ZIP archive
4. Run these commands:

```shell
# Change directory to example
$ cd platform-espressif32/examples/arduino-blink

# Build project
$ pio run

# Upload firmware
$ pio run --target upload

# Build specific environment
$ pio run -e esp32dev

# Upload firmware for the specific environment
$ pio run -e esp32dev --target upload

# Clean build files
$ pio run --target clean
```

## Hardware Notes

### Board
- ESP32 S3 WROOM 1 N16R8

### Display
- 2.4" SPI TFT 240x320 with SD card (no touch)

#### Display Pins
| Function | GPIO Pin | Description |
|----------|----------|-------------|
| SCK (CLK) | GPIO14 | SPI Clock |
| MOSI | GPIO13 | SPI Data |
| CS | GPIO15 | Chip Select display |
| DC | GPIO2 | Data/Command |
| RESET | GPIO4 | Reset |
| GND | GND | Ground |
| VCC | 3.3V | Power |

### Buttons
- Uses [resistor ladder setup](https://www.instructables.com/How-to-access-5-buttons-through-1-Arduino-input/)
- Connected to GPIO1

### SD Card Pins
#### Shared SPI pins
- MOSI → GPIO 13
- MISO → GPIO 11
- SCK → GPIO 14

#### Chip select pins
- TFT_CS → GPIO 15
- SD_CS → GPIO 12

==============
/*
 * The board: esp32 s3 wroom 1 n16r8
 *
 * Display: 2.4" SPI TFT 240x320, cd card, no touch
 * Pins:
  SCK (CLK)	GPIO14	SPI Clock
  MOSI	    GPIO13	SPI Data
  CS	      GPIO15	Chip Select display
  DC	      GPIO2	  Data/Command
  RESET	    GPIO4	  Reset
  GND	      GND	    Ground
  VCC	      3.3V
 *
 * Buttons: https://www.instructables.com/How-to-access-5-buttons-through-1-Arduino-input/
 * Pins: GPIO1
 *
 * CD Card pins:
  // SPI спільні:
  MOSI → GPIO 13
  MISO → GPIO 11
  SCK  → GPIO 14

  // CS різні:
  TFT_CS → GPIO 15
  SD_CS  → GPIO 12
 */

// Button positions:
//    __________
// S3|          |S4
// S2|  display |S5
// S1|__________|S6
// | - battery  + |
//
// --- Notes ---
// #ifndef LED_BUILTIN
//   #define LED_BUILTIN 2
// #endif
//
// Default text size 5x7+1 (space)

