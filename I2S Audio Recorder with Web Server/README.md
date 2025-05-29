# ESP32 I2S Audio Recorder with Wi-Fi and Web Server

This project records audio from an I2S microphone on the ESP32, saves the recording as a WAV file in the onboard LittleFS filesystem, and serves the recorded audio over Wi-Fi via a simple HTTP web server.

---

## Features

- Connects ESP32 to a specified Wi-Fi network.
- Records audio from an I2S microphone using the ESP32 I2S peripheral.
- Saves audio data as a standard WAV file on LittleFS.
- Hosts a simple web server to download the recorded WAV file.
- Configurable recording duration and sample rate.

---

## Hardware Requirements

- ESP32 development board
- I2S microphone module connected as follows:
  - **I2S_SCK (BCLK):** GPIO 26
  - **I2S_WS (LRCLK):** GPIO 25
  - **I2S_SD (DOUT):** GPIO 22

---

## Software Requirements

- Arduino IDE with ESP32 Board Support installed
- Libraries:
  - `WiFi.h` (comes with ESP32 core)
  - `driver/i2s.h` (ESP32 I2S driver)
  - `LittleFS.h` (ESP32 filesystem)
  - `WebServer.h` (ESP32 web server)

---

## Configuration

Edit the following Wi-Fi credentials in the code before uploading:

```cpp
const char* ssid = "Your_SSID";
const char* password = "Your_WIFI_Password";

Adjust recording parameters as needed:

SAMPLE_RATE — Sample rate in Hz (default 30000)

SAMPLE_BITS — Bits per sample (default 16)

RECORD_TIME — Recording duration in seconds (default 11)

How It Works
Setup:

Initializes LittleFS filesystem.

Connects ESP32 to the configured Wi-Fi network.

Configures I2S peripheral to receive audio data from the microphone.

Recording:

Records audio for the specified duration.

Saves audio samples as a WAV file with proper header formatting.

Web Server:

Starts an HTTP server on port 80.

Serves a simple webpage with a download link to the recorded audio file.

Allows downloading the audio.wav file from the ESP32 via the /download endpoint.

Usage
Upload the sketch to your ESP32 using Arduino IDE.

Open the Serial Monitor at 115200 baud to see debug messages.

ESP32 connects to the Wi-Fi network and records audio.

Once recording completes, note the IP address printed in Serial Monitor.

On a device connected to the same Wi-Fi, open a browser and visit:

cpp
Copy
Edit
http://<ESP32_IP>/
Click the "Download Recorded WAV" link to download the audio file.

Notes
The WAV file is stored in LittleFS, so ensure sufficient free space.

Only mono audio is recorded (single channel).

The code uses blocking recording; ESP32 will not handle other tasks during recording.

Adjust I2S pins if your hardware wiring is different.


