#include <Arduino.h>
#include <WiFi.h>
#include <driver/i2s.h>
#include <LittleFS.h>
#include <WebServer.h>

// WiFi credentials
const char* ssid = "SenzMate_L0. 3G";
const char* password = "Fiber@Senz23L0_3G";

// I2S Pins
#define I2S_SCK 26 //BCLK
#define I2S_WS 25 //LRCSL
#define I2S_SD 22 //DOUT

// I2S Configuration
#define SAMPLE_RATE 30000
#define SAMPLE_BITS 16
#define I2S_CHANNEL I2S_CHANNEL_FMT_ONLY_LEFT
#define BUFFER_SIZE 1024

// WAV File Parameters
#define RECORD_TIME 11 // seconds
const char* fileName = "/audio.wav";

// Web Server on port 80
WebServer server(80);

// Function declarations
void initializeWiFi();
void initializeI2S();
void recordAudio();
void writeWAVHeader(File& file, int sampleRate, int bitsPerSample, int dataSize);
void updateWAVHeader(File& file);

// Setup function (runs once)
void setup() {
  Serial.begin(115200);

  // Initialize LittleFS
  if (!LittleFS.begin(true)) {
    Serial.println("LittleFS initialization failed!");
    return;
  }

  // Initialize Wi-Fi
  initializeWiFi();

  // Initialize I2S
  initializeI2S();

  // Record Audio and Save to LittleFS
  recordAudio();

  // Set up HTTP server
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

// Loop function (runs repeatedly)
void loop() {
  server.handleClient();
}

// Function definitions

void initializeWiFi() {
  Serial.println("Connecting to Wi-Fi...");
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

void initializeI2S() {
  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX),
      .sample_rate = SAMPLE_RATE,
      .bits_per_sample = i2s_bits_per_sample_t(SAMPLE_BITS),
      .channel_format = I2S_CHANNEL,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
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
  int16_t buffer[BUFFER_SIZE / 2];  // 16-bit samples
  unsigned long startMillis = millis();
  unsigned long elapsedMillis = 0;


  while (elapsedMillis < RECORD_TIME * 1000) {
      i2s_read(I2S_NUM_0, buffer, sizeof(buffer), &bytesRead, portMAX_DELAY);

      // Serial.print("Bytes Read: ");
      // Serial.println(bytesRead);

      file.write((uint8_t*)buffer, bytesRead);
      // file.flush(); // Ensure data is actually written

      elapsedMillis = millis() - startMillis;
  }

    Serial.println();

    file.write((uint8_t*)buffer, bytesRead);
    elapsedMillis = millis() - startMillis;


  updateWAVHeader(file);
  file.close();
  Serial.println("Recording complete.");
} 


void writeWAVHeader(File& file, int sampleRate, int bitsPerSample, int dataSize) {
  file.write((uint8_t*)"RIFF", 4); // RIFF chunk
  uint32_t fileSizeMinus8 = dataSize + 36; // Total file size - 8 bytes
  file.write((uint8_t*)&fileSizeMinus8, 4);
  file.write((uint8_t*)"WAVE", 4); // WAVE chunk
  file.write((uint8_t*)"fmt ", 4); // fmt subchunk
  uint32_t subChunk1Size = 16; // PCM header size
  file.write((uint8_t*)&subChunk1Size, 4);
  uint16_t audioFormat = 1; // PCM format
  uint16_t numChannels = 1; // Mono audio
  file.write((uint8_t*)&audioFormat, 2);
  file.write((uint8_t*)&numChannels, 2);
  file.write((uint8_t*)&sampleRate, 4);
  uint32_t byteRate = sampleRate * numChannels * bitsPerSample / 8;
  uint16_t blockAlign = numChannels * bitsPerSample / 8;
  file.write((uint8_t*)&byteRate, 4);
  file.write((uint8_t*)&blockAlign, 2);
  file.write((uint8_t*)&bitsPerSample, 2);
  file.write((uint8_t*)"data", 4); // Data subchunk
  file.write((uint8_t*)&dataSize, 4); // Data size
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
