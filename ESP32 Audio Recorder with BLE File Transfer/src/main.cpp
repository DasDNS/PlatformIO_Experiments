#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include "driver/i2s.h"

// SD Card Configuration
const int chipSelect = 5;
#define WAV_FILE_PATH "/recorded_audio.wav"

// BLE Configuration
#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"

BLEServer *pServer = NULL;
BLECharacteristic *pCharacteristic = NULL;
BLEDescriptor *pDescr;
BLE2902 *pBLE2902;
bool deviceConnected = false, oldDeviceConnected = false, delayBeforeTransfer = true;
unsigned long connectionTime = 0, startTime = 0, totalBytesSent = 0;

// I2S Configuration
#define I2S_NUM I2S_NUM_0
#define I2S_BCK_IO 26
#define I2S_WS_IO 25
#define I2S_DATA_IO 22
const size_t chunkSize = 512;
const int sampleRate = 20000; // Adjust as needed
const int bitsPerSample = 16;
const int channelCount = 1;
File wavFile;

// BLE Callbacks
class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer *pServer) {
        deviceConnected = true;
        connectionTime = millis();
        delayBeforeTransfer = true;
    }
    void onDisconnect(BLEServer *pServer) {
        deviceConnected = false;
    }
};

// Configure I2S for audio recording
void i2sConfig() {
    i2s_config_t i2s_config = {
        .mode = i2s_mode_t(I2S_MODE_MASTER | I2S_MODE_RX),
        .sample_rate = sampleRate,
        .bits_per_sample = i2s_bits_per_sample_t(bitsPerSample),
        .channel_format = I2S_CHANNEL_FMT_ONLY_LEFT,
        .communication_format = I2S_COMM_FORMAT_STAND_I2S,
        .intr_alloc_flags = 0,
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

// Record audio and save as WAV on SD card
void recordWavFile() {
    wavFile = SD.open(WAV_FILE_PATH, FILE_WRITE);
    if (!wavFile) {
        Serial.println("Failed to create WAV file");
        return;
    }

    // Prepare a blank WAV header placeholder
    uint8_t wavHeader[44] = {0};
    wavFile.write(wavHeader, 44); // Reserve space for WAV header

    uint8_t buffer[chunkSize];
    size_t bytesRead, totalDataSize = 0;
    unsigned long recordStart = millis();
    unsigned long recordDuration = 300000; // 60 seconds
    unsigned long lastTime = millis();

    Serial.println("Recording audio...");
    while ((millis() - recordStart) < recordDuration) {
        unsigned long loopStart = millis();
        
        // Read audio data from I2S
        esp_err_t result = i2s_read(I2S_NUM, buffer, chunkSize, &bytesRead, portMAX_DELAY);
        if (result != ESP_OK) {
            Serial.printf("I2S read error: %d\n", result);
            break;
        }
        unsigned long readEnd = millis();

        // Write audio data to the SD card
        wavFile.write(buffer, bytesRead);
        unsigned long writeEnd = millis();

        // Accumulate total data size
        totalDataSize += bytesRead;

        // Print timing for each operation
        //Serial.printf("Read time: %lu ms, Write time: %lu ms\n", readEnd - loopStart, writeEnd - readEnd);

        // Track and print recording progress
        if (millis() - lastTime >= 1000) { // Update every second
            lastTime = millis();
            Serial.printf("Recording... %lu ms elapsed\n", millis() - recordStart);
        }
    }
    Serial.println("Recording complete");

    // Finalize WAV header
    wavFile.seek(0); // Go back to the start of the file
    uint32_t dataSize = totalDataSize; // Total audio data size
    uint32_t fileSize = dataSize + 36; // Total file size (header + data)

    // Create WAV header
    memcpy(wavHeader, "RIFF", 4);
    memcpy(wavHeader + 4, &fileSize, 4);
    memcpy(wavHeader + 8, "WAVEfmt ", 8);
    uint32_t subchunk1Size = 16;
    memcpy(wavHeader + 16, &subchunk1Size, 4);
    uint16_t audioFormat = 1; // PCM format
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

    // Write WAV header
    wavFile.write(wavHeader, 44);
    wavFile.close();
    Serial.printf("WAV file saved: %u bytes\n", totalDataSize);
}

// Send WAV file over BLE
void sendNextChunk() {
    if (!wavFile || !deviceConnected){
        //Serial.println("No WAV file or device not connected");
        return;
    }

    static uint8_t buffer[chunkSize];
    size_t bytesRead = wavFile.read(buffer, chunkSize);
    if (bytesRead > 0) {
        pCharacteristic->setValue(buffer, bytesRead);
        pCharacteristic->notify();
        totalBytesSent += bytesRead;
        delay(200); // Adjust for BLE throughput
    } else {
        // Send end marker
        const char *endMarker = "END_OF_FILE";
        pCharacteristic->setValue((uint8_t *)endMarker, strlen(endMarker));
        pCharacteristic->notify();
        wavFile.close();
        unsigned long elapsedTime = millis() - startTime;
        float dataRate = (float)totalBytesSent / elapsedTime * 1000.0;
        Serial.printf("Transfer complete! Data Rate: %.2f kB/sec\n", dataRate / 1024.0);
    }
    yield();
}

void setup() {
    Serial.begin(115200);
    if (!SD.begin(chipSelect)) {
        Serial.println("Failed to initialize SD card");
        return;
    }
    i2sConfig();
    recordWavFile();

    wavFile = SD.open(WAV_FILE_PATH, "r");
    if (!wavFile) {
        Serial.println("Failed to open WAV file");
        return;
    }

    BLEDevice::init("ESP32-WAV-Transfer");
    BLEDevice::setMTU(517);
    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    BLEService *pService = pServer->createService(SERVICE_UUID);
    pCharacteristic = pService->createCharacteristic(
        CHARACTERISTIC_UUID,
        BLECharacteristic::PROPERTY_NOTIFY
    );
    pDescr = new BLEDescriptor((uint16_t)0x2901);
    pDescr->setValue("WAV File Transfer");
    pCharacteristic->addDescriptor(pDescr);
    pBLE2902 = new BLE2902();
    pBLE2902->setNotifications(true);
    pCharacteristic->addDescriptor(pBLE2902);
    pService->start();
    BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(false);
    pAdvertising->setMinPreferred(0x0);
    BLEDevice::startAdvertising();
    Serial.println("Waiting for a client connection to start WAV transfer...");
    // pCharacteristic->addDescriptor(new BLE2902());
    // pService->start();
    // BLEDevice::startAdvertising();
    // Serial.println("Waiting for client connection...");
}

void loop() {
    if (deviceConnected) {
        if (delayBeforeTransfer && millis() - connectionTime >= 5000) {
            delayBeforeTransfer = false;
            startTime = millis();
            Serial.println("Starting transfer...");
        }
        if (!delayBeforeTransfer) {
            sendNextChunk();
            delay(100); //delay between chunks
        }
    }
    if (!deviceConnected && oldDeviceConnected) {
        pServer->startAdvertising();
        oldDeviceConnected = deviceConnected;
        Serial.println("Advertising restarted...");
    }
    if (deviceConnected && !oldDeviceConnected) {
        oldDeviceConnected = deviceConnected;
        Serial.println("Client connected...");
    }
}
