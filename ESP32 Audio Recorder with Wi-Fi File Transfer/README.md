# ESP32 Audio Recorder with Web Interface

This project allows an **ESP32** to record high-quality audio in **WAV format** using an **I2S microphone**, save it to an **SD card**, and host a **Wi-Fi access point** with a web server to allow downloading the recording. After a recording session and file transfer, the device enters **deep sleep mode** to conserve power.

## Features

- Records 5-minute audio clips at 44.1 kHz, 16-bit, mono using I2S.
- Saves the recording to an SD card in WAV format.
- Hosts a Wi-Fi Access Point (AP) and web server for file download.
- Automatically shuts down server and enters deep sleep after:
  - Successful download confirmation.
  - 5 minutes of inactivity.
  - Error conditions (e.g., failed SD write or I2S read).
- Designed for low-power, remote IoT applications.

## Hardware Requirements

- ESP32 (tested with ESP32-WROOM-32)
- I2S Microphone (e.g., ICS43434)
- MicroSD card module (CS pin connected to GPIO 5)
- Optional: Power source (battery + solar for remote deployment)

## Pin Configuration

| Function     | GPIO |
|--------------|------|
| I2S BCK      | 26   |
| I2S WS (LRCL)| 25   |
| I2S DATA IN  | 22   |
| SD Card CS   | 5    |

## How It Works

1. On boot, the ESP32 initializes the SD card and I2S microphone.
2. It records a 5-minute audio clip and saves it as `record_N.wav`.
3. The ESP32 starts a Wi-Fi AP (`ESP32-WAV-AP`, password: `12345678`) and web server.
4. You can connect to the ESP32 and download the latest recording via:

- http://192.168.4.1/download

5. After successful download, access the following URL to shut down the server:

- http://192.168.4.1/confirm


6. If no confirmation is received within 5 minutes, the ESP32 shuts down the server and enters deep sleep.

## Deep Sleep

- After recording and file transfer, the ESP32 enters **deep sleep for 40 minutes**.
- If an error occurs (e.g., SD failure), it enters deep sleep for **20 seconds** before retrying.

## Folder Structure

project/
├── src/
│ └── main.cpp # Main source code
├── include/
├── platformio.ini # PlatformIO configuration
└── README.md # Project documentation


## PlatformIO Configuration

```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
monitor_speed = 115200

## Known Limitations

- Only the latest file is accessible via the web interface.
- Audio recording duration and sleep times are fixed in code.
- No retry mechanism for failed file downloads.


