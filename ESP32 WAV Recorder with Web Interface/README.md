# ESP32 WAV Audio Recorder with Web Interface

This project allows the ESP32 to record audio using the SPH0645LMH MEMS microphone via the I2S interface and store it as a WAV file in the internal filesystem (LittleFS). The ESP32 also runs a simple web server to download the recorded audio.

## Features

- Records 10-second audio clips at 22.05 kHz sample rate
- Converts 32-bit I2S input to 16-bit PCM WAV format
- Stores audio file in SPI flash using LittleFS
- Hosts a web server to download the recording
- Operates in Wi-Fi Access Point mode for easy access

## Hardware Required

- ESP32 Development Board
- SPH0645LMH MEMS microphone (I2S)
- Micro-USB cable
- Optional: Capacitors and resistors for noise reduction

## Wiring (I2S Microphone to ESP32)

| Microphone Pin | ESP32 Pin |
|----------------|-----------|
| BCLK           | GPIO 26   |
| LRCLK          | GPIO 25   |
| DOUT           | GPIO 22   |
| GND            | GND       |
| VDD            | 3.3V      |

## How It Works

1. On boot, the ESP32:
   - Initializes LittleFS
   - Starts in Wi-Fi Access Point mode (`ESP32_Recorder` / `12345678`)
   - Initializes the I2S interface
   - Records 10 seconds of audio
   - Saves the file as `audio.wav`
2. A web server is launched on port 80:
   - Navigate to `http://192.168.4.1` to access the homepage
   - Click "Download Recorded WAV" to save the file to your computer

## File System

The WAV file is saved in `LittleFS`, so no SD card is required.

## File Name

- `audio.wav`: Recorded audio file (mono, 16-bit PCM, 22.05 kHz)

## Installation

1. Install [Arduino IDE](https://www.arduino.cc/en/software)
2. Add the ESP32 board package to the IDE
3. Install the following libraries:
   - `LittleFS` (for ESP32)
   - `WebServer` (comes with ESP32 core)
4. Upload the sketch to your ESP32
5. Connect to the ESP32 Wi-Fi network:  
   SSID: `ESP32_Recorder`  
   Password: `12345678`
6. Open `http://192.168.4.1` in your browser to download the WAV file

## File Format

The output WAV file uses the following format:
- Format: PCM
- Channels: Mono
- Sample rate: 22050 Hz
- Bit depth: 16-bit

## Customization

To change recording duration or sample rate:
```cpp
#define SAMPLE_RATE 22050     // Change for higher/lower quality
#define RECORD_TIME 10        // Duration in seconds

Notes
The SPH0645LMH microphone outputs 24-bit data inside a 32-bit word. This implementation converts it to 16-bit samples.

The recorded WAV is mono (left channel only).

WAV headers are manually written and updated to ensure file compatibility.


