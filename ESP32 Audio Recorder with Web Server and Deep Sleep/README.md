# ESP32 Audio Recorder with Web Server and Deep Sleep

This project uses an **ESP32** to:
- Record audio from an **I2S microphone**
- Save the audio as a **WAV file** on an **SD card**
- Host a **Wi-Fi access point** with a web server
- Allow clients to **download** the recorded file
- Enter **deep sleep** to conserve power after confirmation or timeout

## Features

- Records 1-minute WAV audio using I2S
- Stores recordings on SD card with unique filenames
- Hosts a Wi-Fi AP (`ESP32-WAV-AP`) for clients to connect
- Serves the latest audio file via HTTP
- Waits up to 3 minutes for a confirmation (`/confirm`)
- Sleeps for 54 minutes if no confirmation is received or after successful transfer

## Hardware Required

- **ESP32 development board**
- **I2S microphone** (e.g., ICS43434)
- **MicroSD card module** (connected to `CS` pin 5)
- Optional: **Battery** and **solar charger** for field deployment

## Wiring (ESP32 Pinout)

| Component       | ESP32 Pin |
|----------------|-----------|
| I2S BCK         | GPIO 26   |
| I2S WS (LRCK)   | GPIO 25   |
| I2S DATA        | GPIO 22   |
| SD Card CS      | GPIO 5    |
| SD Card SPI     | Default SPI Pins (MOSI/MISO/SCK) |

## How It Works

1. On boot, the ESP32 initializes the SD card and I2S interface.
2. It records audio for **1 minute**, saving it as `/record_X.wav`.
3. It then creates a **Wi-Fi access point** (`ESP32-WAV-AP`, password: `12345678`).
4. A **web server** is started with two endpoints:
    - `/download`: Serves the last recorded WAV file
    - `/confirm`: Signals the server to shut down and enter deep sleep
5. If no confirmation is received in **3 minutes**, the device goes to sleep.
6. The device sleeps for **54 minutes**, then reboots to repeat the process.

## API Endpoints

| Endpoint     | Method | Description                                |
|--------------|--------|--------------------------------------------|
| `/download`  | GET    | Streams the most recent WAV file           |
| `/confirm`   | GET    | Signals completion and initiates shutdown  |

## Power Optimization

- Watchdog timer is reset regularly to avoid crashes
- Uses `esp_deep_sleep_start()` to save energy between recordings
- Ideal for solar-powered, battery-operated applications

## Compilation & Upload

1. Open the code in **Arduino IDE** or **PlatformIO**.
2. Select **ESP32 Dev Module**.
3. Make sure required libraries are installed:
    - `ESPAsyncWebServer`
    - `AsyncTCP`
4. Upload the sketch to your ESP32.

## Notes

- Ensure your SD card is formatted as **FAT32**.
- You may adjust the recording duration or sleep time by modifying:
    - `recordDuration`
    - `sleep_time_us`
- The filename is auto-incremented as `/record_1.wav`, `/record_2.wav`, etc.


