#include "led.h"

void setupLed(Adafruit_NeoPixel *led) {
  led->begin();
  led->setBrightness(33);
}

void ledOk(Adafruit_NeoPixel *led) {
  led->setPixelColor(0, led->Color(0, 255, 100));
  led->show();
}

void ledConnecting(Adafruit_NeoPixel *led) {
  led->setPixelColor(0, led->Color(0, 0, 255));
  led->show();
}

void ledError(Adafruit_NeoPixel *led) {
  led->setPixelColor(0, led->Color(255, 0, 0));
  led->show();
}

void ledOff(Adafruit_NeoPixel *led) {
  led->setPixelColor(0, led->Color(0, 0, 0));
  led->show();
}