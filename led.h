#pragma once

#include <Adafruit_NeoPixel.h>

void setupLed(Adafruit_NeoPixel *led);
void ledConnecting(Adafruit_NeoPixel *led);
void ledOk(Adafruit_NeoPixel *led);
void ledError(Adafruit_NeoPixel *led);
void ledOff(Adafruit_NeoPixel *led);