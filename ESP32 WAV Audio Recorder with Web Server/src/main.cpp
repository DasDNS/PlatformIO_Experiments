#include <Arduino.h>
#include <WiFi.h>
#include <ESPAsyncWebServer.h>
#include <SPIFFS.h>

// Wi-Fi Configuration
const char *ssid = "ESP32-WAV-AP";
const char *password = "12345678";

// File Path
#define WAV_FILE_PATH "/recorded_audio.wav"

// Create AsyncWebServer instance on port 80
AsyncWebServer server(80);

void setup() {
    Serial.begin(115200);

    // Initialize SPIFFS
    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS Mount Failed!");
        return;
    }

    // Check if the WAV file exists
    if (!SPIFFS.exists(WAV_FILE_PATH)) {
        Serial.println("Error: WAV file not found in SPIFFS!");
        return;
    }
    Serial.println("WAV file found!");

    // Start Wi-Fi Access Point
    WiFi.softAP(ssid, password);
    Serial.println("Wi-Fi AP started");
    Serial.print("IP Address: ");
    Serial.println(WiFi.softAPIP());

    // Serve the WAV file
    server.on("/download", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!SPIFFS.exists(WAV_FILE_PATH)) {
            request->send(404, "text/plain", "File not found");
            return;
        }

        File file = SPIFFS.open(WAV_FILE_PATH, "r");
        if (!file) {
            request->send(500, "text/plain", "Failed to open file");
            return;
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

    // Start the server
    server.begin();
    Serial.println("Server started!");
}

void loop() {
    // Nothing needed in the loop, handled by AsyncWebServer
}
