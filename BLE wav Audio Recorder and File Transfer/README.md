# ESP32 BLE WAV Audio Recorder & File Transfer

This project demonstrates how to record audio using the ESP32 with an I2S microphone and transmit the recorded `.wav` file to a client device over Bluetooth Low Energy (BLE).

## Features

- Records 10 seconds of audio using I2S and stores it as a `.wav` file in SPIFFS.
- Sets up a BLE server and sends the recorded audio file in chunks.
- Notifies client devices when the full transfer is complete.
- Includes basic data rate reporting and connection handling.

## Hardware Requirements

- **ESP32 Development Board**
- **I2S Microphone (e.g., ICS43434, INMP441)**
- **Optional: Serial monitor for debugging**

### I2S Pin Configuration

| I2S Signal | ESP32 GPIO |
|------------|------------|
| BCLK       | GPIO 26    |
| LRCLK (WS) | GPIO 25    |
| DATA       | GPIO 22    |

## Getting Started

### 1. Install Arduino Libraries

Ensure you have the following libraries in your Arduino IDE:
- `BLEDevice` (built into ESP32 core)
- `FS` and `SPIFFS` for file system access
- `driver/i2s.h` for I2S operations

### 2. Flash the Code

Upload the sketch to your ESP32 using the Arduino IDE or PlatformIO.

### 3. Connect to BLE

After booting:
- ESP32 advertises as **ESP32-WAV-Transfer**
- Use a BLE client app (e.g., **nRF Connect**) to scan and connect.
- The recorded `.wav` file will be automatically sent in chunks.
- At the end of transfer, `"END_OF_FILE"` is sent as a marker.

## File Format Details

The `.wav` file is:
- Mono (1 channel)
- 16-bit PCM
- 16,000 Hz sample rate

## BLE Characteristics

- **Service UUID**: `4fafc201-1fb5-459e-8fcc-c5c9c331914b`
- **Characteristic UUID**: `beb5483e-36e1-4688-b7f5-ea07361b26a8`
- Notifications are used to send audio chunks to the client.

## Serial Output Example

Recording audio...
Recording complete
WAV file opened successfully
Waiting for a client connection to start WAV transfer...
Client connected...
Starting transfer...
File transfer complete! Data Rate: 14.23 kB/sec


## Customization

You can change the following constants:
- `recordDuration` to record more/less time.
- `sampleRate`, `bitsPerSample`, and `channelCount` to adjust quality.
- `chunkSize` and MTU to improve BLE throughput.

## Troubleshooting

- Ensure BLE client app supports notifications and MTU size is set correctly.
- Make sure SPIFFS is properly formatted if mounting fails.
- Monitor serial logs for debug output.


