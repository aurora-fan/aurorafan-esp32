#include <FastLED.h>

#ifndef Pixel_h
#define Pixel_h

struct Pixel {
    uint8_t radius;
    uint8_t tick;
    CRGB color;
    bool hold;
};

typedef struct Pixel Pixel;

Pixel PixelInitWithImageCoordinates(uint8_t x, uint8_t y, CRGB color);

uint8_t convertToTick(double radians, uint8_t ticksPerRevolution);

#endif

            // "macFrameworkPath": [
            //     "/System/Library/Frameworks",
            //     "/Library/Frameworks"
            // ],