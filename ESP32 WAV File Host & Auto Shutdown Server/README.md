# ESP32 WAV File Host & Auto Shutdown Server

This project runs on an **ESP32** to:

- Host the most recently recorded WAV file via a local Wi-Fi access point
- Provide endpoints for downloading and confirming the transfer of the file
- Automatically shut down and enter deep sleep to conserve power

> ⚠️ This sketch **does not record audio**. It assumes a WAV file is already available on the SD card (e.g., created by another ESP32 audio recording sketch).

---

## Features

- 🔌 Boots and initializes SD card
- 📂 Finds the most recent WAV file (`record_N.wav`)
- 📡 Creates a Wi-Fi Access Point for file download
- 🌐 Hosts a lightweight web server with:
  - `/download` – Streams the WAV file
  - `/confirm` – Stops server and enters deep sleep
- 💤 Enters deep sleep after:
  - File download confirmation
  - Timeout of 30 minutes (no confirmation)

---

## Hardware Requirements

- ESP32 Dev board
- SD Card module
- SD card with recorded WAV file (e.g., `record_1.wav`)
- Power source (battery/USB)

### Pin Configuration

| Component     | GPIO |
|---------------|------|
| SD Card CS    | 5    |

---

## Wi-Fi Access Point

| Property      | Value           |
|---------------|-----------------|
| SSID          | `ESP32-WAV-AP`  |
| Password      | `12345678`      |
| IP Address    | `192.168.4.1`   |
| Channel       | 6               |
| TX Power      | 19.5 dBm        |

---

## HTTP API Endpoints

| Endpoint     | Method | Description                            |
|--------------|--------|----------------------------------------|
| `/download`  | GET    | Streams the most recent WAV file       |
| `/confirm`   | GET    | Confirms download, triggers deep sleep |

---

## Behavior Summary

1. **Startup**:
   - Initializes SD card (retries 3x).
   - Scans for the latest `record_*.wav` file.
   - Starts Wi-Fi AP and web server.

2. **File Transfer**:
   - User connects to AP and fetches filename via `/download`.
   - WAV file is streamed in chunks (16 KB) to match client buffer.

3. **Confirmation or Timeout**:
   - If `/confirm` is received → Server shuts down and enters deep sleep.
   - If no confirmation in 30 minutes → Enters deep sleep automatically.

4. **Power Saving**:
   - Uses watchdog to reset on failure.
   - Wi-Fi automatically turned off before deep sleep.
   - Deep sleep duration: 10 minutes (configurable).

---

## Configuration

You can change:

- `sleep_time_us` – Deep sleep duration (in microseconds)
- `CONFIRMATION_TIMEOUT` – Timeout before deep sleep if no confirmation
- `ssid`, `password` – Wi-Fi network details
- `chipSelect` – SD card CS pin

---

## Example Use Case

This code is designed to work **after an ESP32 finishes recording audio**. A second ESP32 (or the same one rebooted) can run this sketch to:

- Share the latest file over Wi-Fi
- Shut down safely once the file is downloaded

---

