# ESP32 WAV Recorder with Access Point and Web Server

This project uses an ESP32 microcontroller to:

- Record high-quality WAV audio files using an I2S microphone (e.g., ICS43434)
- Save recordings to an SD card
- Host a Wi-Fi Access Point
- Provide a web server for downloading the latest recorded file
- Automatically shut down after successful file transfer confirmation
- Enter deep sleep to conserve power between recordings

## Features

- 📢 Records WAV audio files at 44.1 kHz, 16-bit mono
- 💾 Stores files on SD card with dynamic naming (`/record_N.wav`)
- 📶 Creates its own Wi-Fi network (AP mode)
- 🌐 Simple web interface to get filename and download file
- 💤 Enters deep sleep if no confirmation is received or button is pressed
- 🔘 Single button to start/stop recording
- 💡 LED indicator for status

## Hardware Requirements

- ESP32-WROOM-32 or similar
- I2S microphone (e.g., ICS43434)
- SD card module
- Push button connected to GPIO33 (with pull-up)
- LED connected to GPIO32 (status indicator)
- Power source (battery + solar supported)

### Wiring

| Component     | GPIO Pin |
|---------------|----------|
| I2S BCK       | 26       |
| I2S WS (LRCK) | 25       |
| I2S DATA      | 22       |
| SD Card CS    | 5        |
| Button        | 33       |
| LED           | 32       |

## Software Setup

### Arduino Libraries Required

- `ESPAsyncWebServer`
- `AsyncTCP`
- `SD`
- `SPI`
- `WiFi`
- `driver/i2s.h`

### Compilation

1. Install the required libraries.
2. Connect the ESP32 board to your PC.
3. Upload the sketch via Arduino IDE or PlatformIO.

## How It Works

1. On startup, the ESP32 initializes I2S and SD card.
2. Waits for button press to start recording.
3. Button press starts recording and LED turns ON.
4. Button press again stops recording, finalizes WAV file, and stores it.
5. ESP32 starts a Wi-Fi Access Point and web server:
   - `/filename` returns the last recorded file name.
   - `/download` serves the WAV file in chunks.
   - `/confirm` triggers a double LED blink and shuts down the server.
6. After timeout (5 minutes) or successful confirmation, ESP32 enters deep sleep (for 20 seconds or configurable).

## Access Point Details

- **SSID:** `ESP32-WAV-AP`
- **Password:** `12345678`
- **IP Address:** `192.168.4.1`

## HTTP Endpoints

| Endpoint     | Method | Description                        |
|--------------|--------|------------------------------------|
| `/filename`  | GET    | Returns the name of the latest file |
| `/download`  | GET    | Downloads the WAV file              |
| `/confirm`   | GET    | Stops the server and blinks the LED |

## Power Saving

- Watchdog timer prevents lockups (30 minutes timeout)
- If recording or SD initialization fails, enters deep sleep
- After successful transfer or timeout, enters deep sleep



