#include <FastLED.h>

#ifndef Pixel_h
#define Pixel_h

struct Pixel {
    uint8_t radius;
    uint8_t tick;
    CRGB color;
    bool hold;

    static Pixel initWithImageCoordinates(const uint8_t x, const uint8_t y, const CRGB color);
};

typedef struct Pixel Pixel;

uint8_t convertToTick(const double radians, const uint8_t ticksPerRevolution);

#endif
