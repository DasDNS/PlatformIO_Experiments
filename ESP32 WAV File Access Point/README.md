# 📻 ESP32 WAV File Access Point

This project turns your **ESP32** into a **Wi-Fi access point and web server** that allows a client device (e.g., Raspberry Pi) to **download the most recent WAV file** stored on an **SD card**. After a successful file transfer or a timeout, the device enters **deep sleep** to save power.

---

## ⚙️ Features

- ✅ Hosts a Wi-Fi Access Point (SSID: `ESP32-WAV-AP`)
- ✅ Serves the most recently recorded `record_*.wav` file via `/download`
- ✅ Waits for confirmation from the client on `/confirm`
- ✅ Enters deep sleep after file transfer or 3-minute timeout
- ✅ Automatic SD card retry initialization (3 times)
- ✅ Power-saving with watchdog timer and deep sleep control

---

## 📦 Hardware Requirements

- ESP32 (with WiFi and SD card support)
- microSD Card Module (SPI connected)
- microSD Card with WAV files (e.g., `record_1.wav`, `record_2.wav`, etc.)

---

## 🔌 Wiring (Default SPI on ESP32)

| SD Card Pin | ESP32 Pin |
|-------------|-----------|
| CS          | GPIO 5    |
| MOSI        | GPIO 23   |
| MISO        | GPIO 19   |
| SCK         | GPIO 18   |
| GND         | GND       |
| VCC         | 3.3V or 5V|

---

## 📡 Wi-Fi Access Point Configuration

- **SSID**: `ESP32-WAV-AP`
- **Password**: `12345678`
- **Channel**: 6
- **Power**: 19.5 dBm
- **IP Address**: `192.168.4.1`

---

## 🌐 Web Endpoints

| Route       | Method | Description |
|-------------|--------|-------------|
| `/download` | GET    | Streams the latest `record_*.wav` file in chunks |
| `/confirm`  | GET    | Client notifies that the download is complete. Device enters deep sleep |

---

## 🔁 Workflow

1. ESP32 boots and initializes SD card.
2. Scans for the latest WAV file (`record_*.wav`).
3. Starts a Wi-Fi AP and web server.
4. Client connects and downloads the file from `/download`.
5. Client sends a confirmation request to `/confirm`.
6. ESP32 shuts down Wi-Fi and enters deep sleep (54 minutes).
7. If no confirmation is received within 3 minutes, ESP32 also goes to deep sleep.

---

## ⏱ Power Management

- **Watchdog Timer**: 20-minute timeout to avoid hangs
- **Deep Sleep Time**: 54 minutes (`10 * 60 * 1,000,000` microseconds)
- **Timeout**: If no confirmation received in 3 minutes, shutdown

---

## 🛠 Dependencies

Make sure to install these Arduino libraries:

- `ESPAsyncWebServer`
- `ESPAsyncTCP`
- `SD`
- `SPI`
- `WiFi`

---

## 📄 File Naming Convention

Files must follow the format:  
`record_<number>.wav`  
Example: `record_5.wav`

The script will automatically select the file with the highest number for transfer.

---

## 🧪 Testing

1. Insert a valid WAV file on the SD card (e.g., `record_1.wav`).
2. Power on the ESP32.
3. Connect your PC or Raspberry Pi to `ESP32-WAV-AP`.
4. Open browser and visit:  
   `http://192.168.4.1/download` to download  
   `http://192.168.4.1/confirm` to send completion signal
5. ESP32 will shut down and sleep.

---

## 📋 Notes

- Designed for **low-bandwidth offline environments**.
- Ideal for **energy-efficient field recording devices**.
- Ensure your SD card uses FAT32 and has proper `record_*.wav` files.

---

## 💤 Power Saving Mode

After completing its task, the ESP32 goes into **deep sleep** to conserve power, especially in solar-powered or battery-backed applications.

---


