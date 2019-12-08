#include <math.h>

#include <FastLED.h>
#include <Wire.h>

#include "Pixel.h"

Pixel Pixel::initWithImageCoordinates(const uint8_t x, const uint8_t y, const CRGB color) {
    Pixel p;

    const double centeredX = x - 26;
    const double centeredY = -y + 26;

    const double theta = atan2(centeredY, centeredX);
    const double r = sqrt(centeredX * centeredX + centeredY * centeredY);

    p.radius = (uint8_t) r;
    p.tick = convertToTick(theta, 50);
    p.color = color;
    p.hold = true;

    Serial.println(p.tick);

    return p;
}

uint8_t convertToTick(const double radians, const uint8_t ticksPerRevolution) {
    return radians * (double) ticksPerRevolution / (2 * M_PI);
}
