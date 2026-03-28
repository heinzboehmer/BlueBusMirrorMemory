# **_NOTE:_**
This fork adds memory seat/mirror functionality to the BlueBus when used in an E46 with the CD53 radio. 

This overloads the functionality of button `5` on the CD53. Repeatedly pressing will cycle through the following modes:

1. Recall Mirror Memory
2. Save Mirror Memory
3. Device selection mode

When in modes (1) or (2), use buttons `1`, `2` or `3` to recall/save memory 1, 2 or 3. Mode (3) works like stock.

Additionally, a handler is added to fix an E46 bug where the passenger mirror won't exactly return to its previous position after auto dipping in reverse.

The handler is scheduled after the transmission is taken out of reverse. When the timer elapses, the handler checks that the transmission is still out of reverse and issues the "Recall Mem X" IBus command (where "X" is the selected memory index). This feature can be configured to recall any mirror memory or turned off entirely in the settings.

Please keep in mind that these features are hacked in and are very much E46 specific. This code should not be merged back to the BlueBus upstream source unless refactored to be cleaner and universal.

# 

View this project on [CADLAB.io](https://cadlab.io/project/1479). 

# BlueBus

BlueBus is a Bluetooth interface module for BMW vehicles equipped with the I-Bus. It emulates a CD changer on the I-Bus to provide seamless Bluetooth audio, hands-free telephony, and comfort feature integration while maintaining full compatibility with original vehicle systems.

## Supported Vehicles

BlueBus is compatible with BMW vehicles using the I-Bus architecture:

| Platform | Models | Years |
|----------|--------|-------|
| E38 | 7 Series | 1994-2001 |
| E39 | 5 Series | 1995-2004 |
| E46 | 3 Series | 1997-2006 |
| E52 | Z8 | 2000-2003 |
| E53 | X5 | 1999-2006 |
| E83 | X3 | 2003-2010 |
| E85 | Z4 | 2003-2008 |
| E86 | Z4 | 2005-2008 |
| R50/R52/R53 | MINI | 2000-2006 |

## Features

### Bluetooth Audio
* A2DP streaming with high-quality audio output over AAC
* AVRCP metadata display (artist, title, album) on vehicle screens
* Full playback control via vehicle buttons

### Hands-Free Telephony
* HFP (Hands-Free Profile) with factory microphone integration
* Caller ID display on vehicle screen
* Answer/end calls via vehicle controls
* Telephone LED control (Red = Device Disconnected, Green = Connected)
* Voice recognition support via Siri / Google Assistant
* Emulation of OE telephone system (Minus the Menus, for now)

### Vehicle Integration
* Native DSP compatibility (analog and S/PDIF inputs)
* On-Board Computer data display (coolant temp, ambient temp, oil temp)
* GPS time synchronization for vehicles with factory Nav
* Park Distance Control integration

### Comfort Features
* Auto-lock at configurable speed threshold
* Auto-unlock on ignition off
* Comfort turn signals (Tap to signal)
* Parking lamp control

### System
* Firmware upgradable via USB
* Multi-language support (9 languages)
* Up to 8 paired Bluetooth devices

## Supported Display Units

| Unit | Description | Display |
|------|-------------|---------|
| BMBT | Board Monitor (Navigation) | Full graphical menus |
| MID | Multi-Information Display | 24-character single line |
| CD53 | Business CD Radio | 11-character single line |
| MIR | Multi-Information Radio | Single line display |

## Hardware

### Core Components
| Component | Description |
|-----------|-------------|
| PIC24FJ1024GA606 | 16-bit MCU @ 16MHz, 1MB Flash |
| BM83 | Microchip Bluetooth 5.0 Module |
| Melexis TH3122.4 | I-Bus Transceiver |

### Audio Chain
| Component | Description |
|-----------|-------------|
| PCM5122 | Line-Level DAC, 2.1V RMS, 113 dB SNR |
| DIT4096 | S/PDIF Encoder |
| PAM8406 | 5W Class-D/Class-AB Amplifier |

### Interfaces
| Component | Description |
|-----------|-------------|
| FT231XS | USB to UART (firmware updates) |


## Configuration

Settings are stored in EEPROM and can be configured through the vehicle display menus. Categories include:

* **Audio** - DAC gain, DSP input source, volume offsets
* **Telephony** - HFP mode, microphone gain, volume
* **Comfort** - Auto-lock speed, blinker count, lighting options
* **UI** - Language, temperature units, display preferences
* **Navigation** - Auto-zoom, map display mode

