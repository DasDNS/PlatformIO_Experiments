#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include "esp_task_wdt.h"
#include "esp_sleep.h"

// SD Card Configuration
const int chipSelect = 5;
String lastRecordedFile = "";  // Stores the most recent file name

// Wi-Fi Configuration
const char *ssid = "ESP32-WAV-AP";
const char *password = "12345678";
const int wifi_channel = 6;    // Fixed WiFi channel
const int wifi_power = 20;     // WiFi power level in dBm (max 20)
AsyncWebServer server(80);
const size_t transfer_chunk_size = 16384; // Define a fixed chunk size (matching the Raspberry Pi client)
bool stopServer = false;
bool sdInitialized = false;

// Convert seconds to microseconds for deep sleep time (54 minutes)
uint64_t sleep_time_us = 10ULL * 60 * 1000000;

// Timeout Configuration (3 minutes)
const unsigned long CONFIRMATION_TIMEOUT = 30 * 60 * 1000;  // 1 minute in milliseconds
unsigned long serverStartTime;

void enterDeepSleep() {
    Serial.println("Entering deep sleep for 20 seconds...");
    delay(100);
    esp_sleep_enable_timer_wakeup(20 * 1000000);
    esp_deep_sleep_start();
}

void findLastWavFile() {
    File root = SD.open("/");
    if (!root) {
        Serial.println("Failed to open root directory");
        enterDeepSleep();
    }
    
    File file;
    String lastFileName = "";
    int highestNumber = 0;
    
    while (file = root.openNextFile()) {
        String fileName = String(file.name());
        if (fileName.endsWith(".wav") && fileName.startsWith("record_")) {
            // Extract the file number
            int underscorePos = fileName.indexOf('_');
            int dotPos = fileName.lastIndexOf('.');
            if (underscorePos != -1 && dotPos != -1) {
                String numberStr = fileName.substring(underscorePos + 1, dotPos);
                int fileNumber = numberStr.toInt();
                if (fileNumber > highestNumber) {
                    highestNumber = fileNumber;
                    lastFileName = fileName;
                }
            }
        }
        file.close();
    }
    
    root.close();
    
    if (lastFileName != "") {
        lastRecordedFile = "/" + lastFileName;
        Serial.printf("Last recorded file found: %s\n", lastRecordedFile.c_str());
    } else {
        Serial.println("No WAV files found");
    }
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
    
    // Find the last recorded WAV file
    findLastWavFile();

    // Configure WiFi with fixed channel and power level
    WiFi.softAP(ssid, password, wifi_channel);
    WiFi.setTxPower(WIFI_POWER_19_5dBm); // Set to maximum allowed power
    WiFi.softAPConfig(IPAddress(192,168,4,1), IPAddress(192,168,4,1), IPAddress(255,255,255,0));
    Serial.println("Wi-Fi AP started");
    Serial.print("IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.printf("WiFi channel: %d, Power: 19.5dBm\n", wifi_channel);

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
                // Limit reading to our fixed chunk size
                size_t bytesToRead = file.available() ? 
                                     min(transfer_chunk_size, min(maxLen, static_cast<size_t>(file.available()))) : 0;
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
        WiFi.softAP(ssid, password, wifi_channel);
        WiFi.setTxPower(WIFI_POWER_19_5dBm); // Set to maximum allowed power
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