#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "driver/i2s.h"
#include "esp_task_wdt.h"
#include "esp_sleep.h"

// SD Card Configuration
const int chipSelect = 5;
String lastRecordedFile = "";  // Stores the most recent file name

// Wi-Fi Configuration
const char *ssid = "ESP32-WAV-AP";
const char *password = "12345678";
AsyncWebServer server(80);

bool stopServer = false;
bool sdInitialized = false;

// I2S Configuration
#define I2S_NUM I2S_NUM_0
#define I2S_BCK_IO 26
#define I2S_WS_IO 25
#define I2S_DATA_IO 22
const size_t chunkSize = 600;
const int sampleRate = 44100;
const int bitsPerSample = 16;
const int channelCount = 1;
File wavFile;

// Convert seconds to microseconds for deep sleep time (54 minutes)
uint64_t sleep_time_us = 10ULL * 60 * 1000000;

// Timeout Configuration (3 minutes)
const unsigned long CONFIRMATION_TIMEOUT = 5 * 60 * 1000;  // 1 minute in milliseconds
unsigned long serverStartTime;

void enterDeepSleep() {
    Serial.println("Entering deep sleep for 20 seconds...");
    delay(100);
    esp_sleep_enable_timer_wakeup(20 * 1000000);
    esp_deep_sleep_start();
}
void i2sConfig() {
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = sampleRate,
        .bits_per_sample = i2s_bits_per_sample_t(bitsPerSample),
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 8,
        .dma_buf_len = 1024
    };
    i2s_driver_install(I2S_NUM, &i2s_config, 0, NULL);
    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_BCK_IO,
        .ws_io_num = I2S_WS_IO,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_DATA_IO
    };
    i2s_set_pin(I2S_NUM, &pin_config);
}

void recordWavFile() {
    // Generate a unique filename
    int fileNumber = 1;
    String fileName;
    do {
        fileName = "/record_" + String(fileNumber) + ".wav";
        fileNumber++;
    } while (SD.exists(fileName)); // Keep checking until we find an unused filename

    // Open the new file
    wavFile = SD.open(fileName, FILE_WRITE);
    if (!wavFile) {
        Serial.println("Failed to create WAV file");
        enterDeepSleep();
    }

    uint8_t wavHeader[44] = {0};
    wavFile.write(wavHeader, 44);  // Reserve space for the WAV header

    uint8_t buffer[chunkSize];
    size_t bytesRead, totalDataSize = 0;
    unsigned long recordStart = millis();
    unsigned long recordDuration = 1 * 60 * 1000;  // 1 minute

    Serial.println("Recording audio...");
    while ((millis() - recordStart) < recordDuration) {
        esp_task_wdt_reset();  // Reset watchdog timer periodically

        esp_err_t result = i2s_read(I2S_NUM, buffer, chunkSize, &bytesRead, portMAX_DELAY);
        if (result != ESP_OK || bytesRead == 0) {
            Serial.println("Error reading from I2S");
            enterDeepSleep();
        }
        wavFile.write(buffer, bytesRead);
        totalDataSize += bytesRead;
    }
    Serial.println("Recording complete");

    // Write WAV header
    wavFile.seek(0);
    uint32_t dataSize = totalDataSize;
    uint32_t fileSize = dataSize + 36;
    memcpy(wavHeader, "RIFF", 4);
    memcpy(wavHeader + 4, &fileSize, 4);
    memcpy(wavHeader + 8, "WAVEfmt ", 8);
    uint32_t subchunk1Size = 16;
    memcpy(wavHeader + 16, &subchunk1Size, 4);
    uint16_t audioFormat = 1;
    memcpy(wavHeader + 20, &audioFormat, 2);
    memcpy(wavHeader + 22, &channelCount, 2);
    memcpy(wavHeader + 24, &sampleRate, 4);
    uint32_t byteRate = sampleRate * channelCount * (bitsPerSample / 8);
    memcpy(wavHeader + 28, &byteRate, 4);
    uint16_t blockAlign = channelCount * (bitsPerSample / 8);
    memcpy(wavHeader + 32, &blockAlign, 2);
    memcpy(wavHeader + 34, &bitsPerSample, 2);
    memcpy(wavHeader + 36, "data", 4);
    memcpy(wavHeader + 40, &dataSize, 4);

    wavFile.write(wavHeader, 44);
    wavFile.close();
    Serial.printf("WAV file saved: %s (%u bytes)\n", fileName.c_str(), totalDataSize + 44);

    // Store the last recorded file name
    lastRecordedFile = fileName;
    Serial.printf("Last recorded file: %s\n", lastRecordedFile.c_str());
}

void setup() {
    Serial.begin(115200);

     // Increase the watchdog timeout to 1200 seconds/20 min
     esp_task_wdt_init(1200, true);
     esp_task_wdt_add(NULL);

     for (int i = 0; i < 3; i++) {
        if (SD.begin(chipSelect)) {
            Serial.println("SD initialized successfully!");
            sdInitialized = true;
            break;
        }
        Serial.println("Retrying SD initialization...");
        SD.end();  // Reset SD interface
        delay(500);
    }
    
    // If SD initialization failed after 3 attempts, go to deep sleep
    if (!sdInitialized) {
        Serial.println("Failed to initialize SD card. Entering deep sleep...");
        enterDeepSleep();
    }
    i2sConfig();
    recordWavFile();

    WiFi.softAP(ssid, password,6);
    WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
    Serial.println("Wi-Fi AP started");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());

    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!SD.exists(lastRecordedFile)) {
            request->send(404, "text/plain", "File not found");
            Serial.println("File not found");
            enterDeepSleep();
        }

        File file = SD.open(lastRecordedFile, "r");
        if (!file) {
            request->send(500, "text/plain", "Failed to open file");
            Serial.println("Failed to open file");
            enterDeepSleep();
        }

        AsyncWebServerResponse *response = request->beginChunkedResponse(
            "audio/wav",
            [file](uint8_t *buffer, size_t maxLen, size_t index) mutable -> size_t {
                size_t bytesToRead = file.available() ? min(maxLen, static_cast<size_t>(file.available())) : 0;
                if (bytesToRead) {
                    file.read(buffer, bytesToRead);
                } else {
                    file.close();
                }
                return bytesToRead;
            });

        response->addHeader("Connection", "close");
        request->send(response);
    });

    server.on("/confirm", HTTP_GET, [](AsyncWebServerRequest *request) {
        Serial.println("Received confirmation from client. Stopping server...");
        stopServer = true;
        request->send(200, "text/plain", "Server shutting down");
    });   

    server.begin();
    Serial.println("Server started");

    serverStartTime = millis();
}

void loop() {

    esp_task_wdt_reset();  // Prevent watchdog reset
    delay(1000);  // Wait 1 second before checking again

    // Check if 3 minutes have passed without receiving confirmation
    if (millis() - serverStartTime >= CONFIRMATION_TIMEOUT) {
        Serial.println("No confirmation received within 3 minutes. Proceeding to shutdown...");
        stopServer = true;
    }

    // Monitor the Wi-Fi connection status
    if (WiFi.status() != WL_CONNECTED) {
        //Serial.println("Wi-Fi not connected. Trying to reconnect...");
        WiFi.softAP(ssid, password,6);  // Reinitialize AP mode
        WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
        delay(5000);  // Wait for the connection
    }

    if (stopServer) {
        Serial.println("Shutting down server...");
        delay(1000);  // Wait a moment before shutting down
        server.end();
        WiFi.softAPdisconnect(true);
        WiFi.disconnect(true);
        WiFi.mode(WIFI_OFF);
        if (WiFi.softAPgetStationNum() > 0 ) {
            Serial.println("AP still enabled");
        } else{
            Serial.println("AP is down");
        }
        Serial.println("Server stopped. Entering deep sleep...");

        // Set deep sleep time
       esp_sleep_enable_timer_wakeup(sleep_time_us);

        // Deep Sleep (Wake-up only on power reset)
        esp_deep_sleep_start();
    }
}