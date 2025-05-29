#include <Arduino.h>

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Chip Information
  esp_chip_info_t chip_info;
  esp_chip_info(&chip_info);

  Serial.println("===== ESP32 Chip Information =====");
  Serial.printf("Chip Model: %s\n", ESP.getChipModel());
  Serial.printf("Chip Revision: %d\n", chip_info.revision);
  Serial.printf("Chip Cores: %d\n", chip_info.cores);
  Serial.printf("Chip Features: %s\n", chip_info.features & CHIP_FEATURE_EMB_FLASH ? "Embedded Flash" : "External Flash");
  Serial.printf("Bluetooth Supported: %s\n", chip_info.features & CHIP_FEATURE_BLE ? "Yes" : "No");
  Serial.printf("Wi-Fi Supported: Yes\n");

  // RAM Information
  Serial.println("\n===== RAM Information =====");
  Serial.printf("Total Heap Size: %u bytes\n", ESP.getHeapSize());
  Serial.printf("Free Heap Size: %u bytes\n", ESP.getFreeHeap());
  Serial.printf("Minimum Free Heap Size: %u bytes\n", ESP.getMinFreeHeap());
  Serial.printf("PSRAM Size: %u bytes\n", ESP.getPsramSize());
  Serial.printf("Free PSRAM Size: %u bytes\n", ESP.getFreePsram());

  // Flash Information
  Serial.println("\n===== Flash Information =====");
  Serial.printf("Flash Size: %u bytes\n", ESP.getFlashChipSize());
  Serial.printf("Flash Speed: %u Hz\n", ESP.getFlashChipSpeed());
  Serial.printf("Flash Mode: %s\n", ESP.getFlashChipMode() == FM_QIO ? "QIO" : "DIO");
}

void loop() {
  // Nothing to do in the loop
}
