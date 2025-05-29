# ESP32 WAV Audio Recorder with Web Server and Deep Sleep

This project enables an ESP32 to:
- Record audio using an I2S microphone.
- Save the audio as a `.wav` file on an SD card.
- Host a Wi-Fi Access Point (AP) and web server for file download.
- Enter deep sleep mode after completing the task or timeout.

---

## 📦 Features

- Records high-quality mono audio (44.1 kHz, 16-bit) for 5 minutes.
- Stores the recording in a uniquely named `.wav` file.
- Hosts a web server at `192.168.4.1` for downloading the audio file.
- Waits up to 5 minutes for confirmation via `/confirm` endpoint.
- Enters deep sleep if:
  - File upload confirmation is not received within timeout.
  - Any critical error occurs (e.g., SD card failure, I2S read error).

---

## 🛠️ Hardware Requirements

- **ESP32 Dev Board**
- **I2S Microphone** (e.g., ICS43434)
- **SD Card Module**
- Optional: Solar power module for remote deployment

---

## 🧠 Core Components

### I2S Configuration

```cpp
#define I2S_BCK_IO 26
#define I2S_WS_IO 25
#define I2S_DATA_IO 22

SD Card
SPI-based connection.

chipSelect pin is GPIO 5.

WAV File
Unique filename /record_X.wav generated automatically.

Proper WAV header written after recording.

Web Server
Starts AP: ESP32-WAV-AP, password: 12345678

Endpoints:

/download - Serves the latest WAV file.

/confirm - Acknowledges successful upload and shuts down the server.

💤 Power Management
Deep Sleep Triggers:

Failure to initialize SD card after 3 attempts.

Error during I2S recording.

Missing file or failure to open file during download.

No confirmation received after 5 minutes.

Wakeup Time: 40 minutes (esp_sleep_enable_timer_wakeup)

🌐 How to Use
Power the ESP32.
Wait for the recording (5 minutes).
Connect to Wi-Fi:
SSID: ESP32-WAV-AP
Password: 12345678
Download the file:
Open browser and navigate to http://192.168.4.1/download
Confirm completion:
Visit http://192.168.4.1/confirm
Device will enter deep sleep.

⚠️ Notes
Confirm endpoint (/confirm) must be triggered to prevent timeout.
Only one file is recorded and hosted at a time.
If no confirmation is received, device sleeps and file remains for next session.

📁 File Structure

ESP32-WAV-Recorder/
│
├── main.cpp        # Source code
├── README.md       # This file
└── data/           # Folder for audio files (on SD card)

🧪 Testing
Use a browser or curl to download and confirm:

curl http://192.168.4.1/download -o output.wav
curl http://192.168.4.1/confirm

