# Blink LED Example

This is a basic Arduino sketch that blinks an LED connected to a digital pin on and off at 1-second intervals. It's a common "Hello World" program for microcontrollers and is useful to verify that your board and development environment are working correctly.

## Features

- Turns an LED on for one second
- Turns the LED off for one second
- Repeats the cycle indefinitely

## Hardware Requirements

- ESP32 or compatible development board
- On-board LED or external LED connected to GPIO 14 (changeable)

## Pin Configuration

The LED is configured to use GPIO **14**:

```cpp
#define LED_BUILTIN 14

If your board has a built-in LED on a different pin (e.g., pin 2), update the LED_BUILTIN definition accordingly.

## Code
#include <Arduino.h>

// Set LED_BUILTIN if it is not defined by Arduino framework
#define LED_BUILTIN 14

void setup()
{
  // initialize LED digital pin as an output.
  pinMode(LED_BUILTIN, OUTPUT);
}

void loop()
{
  // turn the LED on (HIGH is the voltage level)
  digitalWrite(LED_BUILTIN, HIGH);
  // wait for a second
  delay(1000);
  // turn the LED off by making the voltage LOW
  digitalWrite(LED_BUILTIN, LOW);
  // wait for a second
  delay(1000);
}

How to Use
Open this code in the Arduino IDE or PlatformIO.

Connect your ESP32 board to your computer.

Make sure the correct board and COM port are selected.

Upload the code.

The LED connected to GPIO 14 should start blinking at 1-second intervals.
