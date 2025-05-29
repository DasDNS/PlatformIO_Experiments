#include <Arduino.h>
#include <WiFi.h>
#include <driver/i2s.h>
#include <LittleFS.h>
#include <WebServer.h>
#include "soc/i2s_reg.h"

// WiFi credentials
const char* ssid = "ESP32_Recorder";
const char* password = "12345678";

// I2S Pins (SPH0645LMH)
#define I2S_SCK 26  // BCLK
#define I2S_WS  25  // LRCLK
#define I2S_SD  22  // DOUT

// I2S Configuration
#define SAMPLE_RATE 22050
#define SAMPLE_BITS 16
#define I2S_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT  // SPH0645LMH outputs on LEFT
#define BUFFER_SIZE 512
#define RECORD_TIME 10  // seconds

const char* fileName = "/audio.wav";
WebServer server(80);

// Function declarations
void initializeWiFi();
void initializeI2S();
void recordAudio();
void writeWAVHeader(File& file, int sampleRate, int bitsPerSample, int dataSize);
void updateWAVHeader(File& file);

void setup() {
    Serial.begin(115200);
    
    if (!LittleFS.begin(true)) {
        Serial.println("LittleFS initialization failed!");
        return;
    }

    initializeWiFi();
    initializeI2S();
    recordAudio();

    // Web server setup
    server.on("/", HTTP_GET, []() {
        server.send(200, "text/html", "<h1>ESP32 WAV Recorder</h1><a href='/download'>Download Recorded WAV</a>");
    });

    server.on("/download", HTTP_GET, []() {
        File file = LittleFS.open(fileName, "r");
        if (!file) {
            server.send(404, "text/plain", "File not found!");
            return;
        }
        server.streamFile(file, "audio/wav");
        file.close();
    });

    server.begin();
    Serial.println("HTTP server started.");
}

void loop() {
    server.handleClient();
}

void initializeWiFi() {
    Serial.println("Starting ESP32 as Access Point...");
    WiFi.softAP(ssid, password);
    Serial.print("ESP32 AP IP Address: ");
    Serial.println(WiFi.softAPIP());
}

void initializeI2S() {
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = SAMPLE_RATE,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_32BIT,  // SPH0645LMH outputs 32-bit
        .channel_format = I2S_CHANNEL,
        .communication_format = (i2s_comm_format_t)(I2S_COMM_FORMAT_I2S | I2S_COMM_FORMAT_I2S_MSB),
        .intr_alloc_flags = 0,
        .dma_buf_count = 8,
        .dma_buf_len = BUFFER_SIZE,
        .use_apll = false,
    };
    i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);

    i2s_pin_config_t pin_config = {
        .bck_io_num = I2S_SCK,
        .ws_io_num = I2S_WS,
        .data_out_num = I2S_PIN_NO_CHANGE,
        .data_in_num = I2S_SD,
    };
    
    REG_SET_BIT(I2S_TIMING_REG(I2S_NUM_0), BIT(9));
    REG_SET_BIT(I2S_CONF_REG(I2S_NUM_0), I2S_RX_MSB_SHIFT);
    i2s_set_pin(I2S_NUM_0, &pin_config);
}

void recordAudio() {
    Serial.println("Recording audio...");
    File file = LittleFS.open(fileName, FILE_WRITE);
    if (!file) {
        Serial.println("Failed to open file!");
        return;
    }

    writeWAVHeader(file, SAMPLE_RATE, SAMPLE_BITS, RECORD_TIME * SAMPLE_RATE * (SAMPLE_BITS / 8));
    
    size_t bytesRead;
    int32_t buffer32[BUFFER_SIZE];  // Read as 32-bit
    int16_t buffer16[BUFFER_SIZE];  // Convert to 16-bit
    unsigned long startMillis = millis();
    unsigned long elapsedMillis = 0;

    while (elapsedMillis < RECORD_TIME * 1000) {
        i2s_read(I2S_NUM_0, buffer32, sizeof(buffer32), &bytesRead, portMAX_DELAY);
        size_t samples = bytesRead / 4;  // 32-bit samples

        for (size_t i = 0; i < samples; i++) {
            buffer16[i] = buffer32[i] >> 11;  // Convert 32-bit to 16-bit
        }

        file.write((uint8_t*)buffer16, samples * 2);
        elapsedMillis = millis() - startMillis;
    }

    updateWAVHeader(file);
    file.close();
    Serial.println("Recording complete.");
}

void writeWAVHeader(File& file, int sampleRate, int bitsPerSample, int dataSize) {
    file.write((uint8_t*)"RIFF", 4);
    uint32_t fileSizeMinus8 = dataSize + 36;
    file.write((uint8_t*)&fileSizeMinus8, 4);
    file.write((uint8_t*)"WAVE", 4);
    file.write((uint8_t*)"fmt ", 4);
    uint32_t subChunk1Size = 16;
    file.write((uint8_t*)&subChunk1Size, 4);
    uint16_t audioFormat = 1;
    uint16_t numChannels = 1;
    file.write((uint8_t*)&audioFormat, 2);
    file.write((uint8_t*)&numChannels, 2);
    file.write((uint8_t*)&sampleRate, 4);
    uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
    uint16_t blockAlign = numChannels * bitsPerSample / 8;
    file.write((uint8_t*)&byteRate, 4);
    file.write((uint8_t*)&blockAlign, 2);
    file.write((uint8_t*)&bitsPerSample, 2);
    file.write((uint8_t*)"data", 4);
    file.write((uint8_t*)&dataSize, 4);
}

void updateWAVHeader(File& file) {
    size_t fileSize = file.size();
    file.seek(4);
    uint32_t fileSizeMinus8 = fileSize - 8;
    file.write((uint8_t*)&fileSizeMinus8, 4);
    file.seek(40);
    uint32_t dataSize = fileSize - 44;
    file.write((uint8_t*)&dataSize, 4);
}
