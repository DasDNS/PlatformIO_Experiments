# ESP32 WAV Audio Recorder with WiFi Access Point and Web Server

This project enables an ESP32 to record audio via an I2S microphone, save it as a WAV file on LittleFS (flash filesystem), and serve it over a WiFi Access Point using a built-in web server. Users can connect to the ESP32's WiFi AP and download the recorded audio file via a simple web page.

---

## Features

- ESP32 acts as a WiFi Access Point with configurable SSID and password.
- Audio recording using I2S interface at 32 kHz, 16-bit, mono.
- Records audio for a fixed duration (default 5 seconds).
- Saves audio as a standard WAV file on LittleFS.
- Embedded HTTP server serves a simple webpage with a download link.
- Recorded audio can be downloaded directly from the ESP32 device.

---

## Hardware Requirements

- ESP32 development board
- I2S microphone or audio input device connected to ESP32 I2S pins:
  - Bit Clock (BCK): GPIO 26
  - Word Select (WS): GPIO 25
  - Data (SD): GPIO 22

---

## Software Requirements

- Arduino IDE or PlatformIO with ESP32 support installed
- Libraries included:
  - WiFi (built-in ESP32 library)
  - driver/i2s.h (ESP-IDF I2S driver)
  - LittleFS (ESP32 file system)
  - WebServer (ESP32 web server)

---

## Pin Configuration

| Signal     | GPIO Pin |
|------------|----------|
| I2S_SCK    | 26       |
| I2S_WS     | 25       |
| I2S_SD     | 22       |

---

## How It Works

1. ESP32 boots up and initializes LittleFS filesystem.
2. ESP32 sets up as a WiFi Access Point with the SSID `ESP32_Audio_Recorder` and password `12345678`.
3. Initializes I2S peripheral to receive audio data from the microphone.
4. Records audio for a duration of 5 seconds.
5. Saves audio data in a WAV format file (`/audio.wav`) on LittleFS.
6. Starts a web server on port 80.
7. When clients connect to the ESP32 AP and open the root URL `/`, they see a simple webpage with a download link.
8. Clicking the download link downloads the recorded audio file.

---

## Usage Instructions

1. Flash the provided code to your ESP32 board.
2. After booting, connect your computer or smartphone to the WiFi network `ESP32_Audio_Recorder` with password `12345678`.
3. Open a web browser and navigate to `http://192.168.4.1/`.
4. Click on the **Download Recorded WAV** link to download the audio file.
5. Play the downloaded `.wav` file using any standard media player.

---

## Customization

- **Access Point Credentials**: Change `ap_ssid` and `ap_password` in the code.
- **Recording Duration**: Modify `RECORD_TIME` (seconds).
- **I2S Pins**: Adjust `I2S_SCK`, `I2S_WS`, `I2S_SD` to match your hardware connections.
- **Sample Rate and Format**: Modify `SAMPLE_RATE`, `SAMPLE_BITS`, and `I2S_CHANNEL` if needed.

---

## Notes

- LittleFS is formatted on first run; existing files will be erased if formatting occurs.
- The code currently records once during setup. To implement continuous recording or recording on demand, modify the code accordingly.
- Make sure your microphone is compatible with the I2S standard and wired correctly.
- The WAV file header is written initially and updated after recording to reflect the actual file size.

---

## Troubleshooting

- **Failed to initialize LittleFS**: Check flash memory availability and ensure no other processes are blocking filesystem access.
- **No Access Point visible**: Verify your ESP32 board is powered and the code uploaded successfully.
- **Download link returns 404**: The recording might have failed or the file was not created. Check serial output for errors.
- **Audio playback issues**: Verify the correct wiring of the microphone and that the sample rate matches your hardware capabilities.

---


