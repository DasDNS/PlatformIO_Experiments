# ESP32 Audio Recorder and BLE WAV File Transfer

This project implements a complete audio recording and wireless file transfer system using an **ESP32** microcontroller. It records audio via **I2S**, saves it as a **WAV file** to an **SD card**, and transfers the file to a connected client over **Bluetooth Low Energy (BLE)**.

---

## Features

- 📦 **Records audio** in WAV format using an I2S MEMS microphone.
- 💾 **Stores** the WAV file on an SD card.
- 📡 **Transfers** the recorded file over BLE using notifications.
- 📱 **Compatible** with BLE-capable mobile or desktop clients.

---

## Hardware Requirements

- **ESP32 Development Board**
- **I2S MEMS Microphone** (e.g., ICS43434)
- **microSD Card Module**
- **microSD Card**
- **Wires & Breadboard**

### Pin Configuration

| ESP32 Pin | Function       | Peripheral   |
|-----------|----------------|--------------|
| GPIO26    | I2S BCLK       | Microphone   |
| GPIO25    | I2S WS (LRCLK) | Microphone   |
| GPIO22    | I2S Data In    | Microphone   |
| GPIO5     | SD Card CS     | SD Card      |

---

## Software Requirements

- Arduino IDE with ESP32 board support
- Required libraries:
  - `SPI.h`
  - `SD.h`
  - `BLEDevice.h`
  - `driver/i2s.h`

---

## Setup & Usage

1. **Connect the hardware** as per the pin configuration.
2. **Insert a formatted microSD card** into the module.
3. **Flash the code** to your ESP32 using the Arduino IDE.
4. Upon startup, the ESP32:
   - Records 5 minutes (300 seconds) of audio.
   - Saves it as `/recorded_audio.wav`.
   - Starts advertising as `ESP32-WAV-Transfer` over BLE.
5. **Connect with a BLE client** (e.g., nRF Connect app or a custom BLE app).
6. The ESP32 automatically begins sending chunks of the WAV file as BLE notifications after a short delay (5 seconds).

---

## BLE Details

- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Characteristic UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- **MTU Size**: 517 bytes
- BLE Notifications are used to send chunks of the WAV file.
- The string `"END_OF_FILE"` is sent at the end of the transmission.

---

## File Structure

- **recorded_audio.wav** — WAV file stored in SD card root

---

## WAV File Format

The ESP32 writes a minimal WAV header:
- **Sample rate**: 20 kHz
- **Channels**: Mono (1 channel)
- **Bits per sample**: 16 bits

---

## Performance

- **Recording duration**: ~5 minutes
- **Chunk size**: 512 bytes
- **Transfer delay**: 100–200 ms between BLE notifications (adjustable)
- **Transfer speed**: Displayed at end (kB/s)

---

## Limitations

- BLE has limited bandwidth; transfers are slow compared to Wi-Fi.
- BLE client must reassemble the chunks to reconstruct the WAV file.
- No file browsing or selection—automatically transfers the latest recording.

---


