#include <math.h>

#include <FastLED.h>
#include <Wire.h>

#include "Pixel.h"

Pixel Pixel::initWithImageCoordinates(const uint8_t x, const uint8_t y, const CRGB color) {
    const double centeredX = (double) x - 26;
    const double centeredY = -((double) y) + 26;

    const double theta = atan2(centeredY, centeredX) + ((centeredY < 0) ? 2 * M_PI : 0);
    const double r = sqrt(centeredX * centeredX + centeredY * centeredY);

    Pixel p = {
        round(r) - 1, 
        convertToTick(theta, 50),
        color,
        true,
        x,
        y
    };

    return p;
}

Pixel Pixel::initWithPolarCoordinates(const uint8_t radius, const uint8_t tick, const CRGB color) {
    Pixel p = {
        radius, 
        tick,
        color,
        true,
        -1,
        -1
    };

    return p;
}

Pixel Pixel::convertImageCoordinatesToMapProjection(const Pixel pixel) {
    Pixel p = {
        26 - pixel.y - 1,
        50 - pixel.x - 1,
        pixel.color,
        true,
        pixel.x,
        pixel.y
    };

    return p;
}

uint8_t convertToTick(const double radians, const uint8_t ticksPerRevolution) {
    return round(radians * (double) ticksPerRevolution / (2 * M_PI));
}
