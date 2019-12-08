#include <vector>

#include <FastLED.h>
#include <Wire.h>

#include "fanprogram/IFanProgram.h"
#include "FastLED_RGBW.h"
#include "Pixel.h"

#define SIDE_LENGTH 52
#define LEDS_PER_BLADE 24
#define BLADE_1_PORT 15
#define BLADE_2_PORT 27

#define TICK_RESOLUTION 50 // Ticks per rotation
#define MICROS_PER_MINUTE 60000000

#define MICROS_PER_TICK(rpm) MICROS_PER_MINUTE / rpm / TICK_RESOLUTION

int currentRpm = 200;
unsigned long long lastFrameUpdateTime = 0;
unsigned int currentTick = 0;

bool lastStateWasHigh = false;
unsigned long long lastHighStateTime = 0;

CRGB blade1[LEDS_PER_BLADE];
CRGB blade2[LEDS_PER_BLADE];

int rpmReadings[5];
int rpmIndex = 0;

std::vector<Pixel> currentPixels;

void clear() {
    for (int i = 0; i < LEDS_PER_BLADE; i++) {
        blade1[i] = CRGB::Black;
        blade2[i] = CRGB::Black;
    }

    FastLED.show();
}

void setup() {
    FastLED.addLeds<NEOPIXEL, BLADE_1_PORT>(blade1, LEDS_PER_BLADE);
    FastLED.addLeds<NEOPIXEL, BLADE_2_PORT>(blade2, LEDS_PER_BLADE);
    FastLED.setBrightness(64);
    clear();

    Serial.begin(115200);

    pinMode(33, OUTPUT);
    pinMode(35, INPUT_PULLUP);
    pinMode(18, INPUT_PULLUP);

    Pixel testPixel;
    testPixel.radius = 11;
    testPixel.tick = 0;
    testPixel.color = CRGB::Blue;
    testPixel.hold = true;

    Pixel testPixel2 = testPixel;
    testPixel2.tick = 13;
    testPixel2.color = CRGB::Green;

    Pixel testPixel3 = testPixel;
    testPixel3.tick = 27;
    testPixel3.color = CRGB::Red;
    
    currentPixels.insert(currentPixels.begin(), testPixel);
    currentPixels.insert(currentPixels.begin(), testPixel3);
    currentPixels.insert(currentPixels.begin(), testPixel2);
    
    currentPixels.insert(currentPixels.begin(), Pixel::initWithImageCoordinates(28, 7, CRGB::Yellow));
    currentPixels.insert(currentPixels.begin(), Pixel::initWithImageCoordinates(26, 7, CRGB::Yellow));
    currentPixels.insert(currentPixels.begin(), Pixel::initWithImageCoordinates(24, 7, CRGB::Yellow));
    currentPixels.insert(currentPixels.begin(), Pixel::initWithImageCoordinates(22, 7, CRGB::Yellow));
    currentPixels.insert(currentPixels.begin(), Pixel::initWithImageCoordinates(21, 8, CRGB::Yellow));
    currentPixels.insert(currentPixels.begin(), Pixel::initWithImageCoordinates(21, 9, CRGB::Yellow));
    currentPixels.insert(currentPixels.begin(), Pixel::initWithImageCoordinates(21, 10, CRGB::Yellow));
    currentPixels.insert(currentPixels.begin(), Pixel::initWithImageCoordinates(52, 26, CRGB::Yellow));
    currentPixels.insert(currentPixels.begin(), Pixel::initWithImageCoordinates(26, 52, CRGB::Yellow));
    currentPixels.insert(currentPixels.begin(), Pixel::initWithImageCoordinates(0, 26, CRGB::Yellow));
    processTick(0);
}

void processTick(int tick) {
    for (int i = currentPixels.size() - 1; i >= 0; i--) {
        if (currentPixels[i].tick == currentTick || currentPixels[i].tick == (currentTick + TICK_RESOLUTION / 2) % TICK_RESOLUTION) {
            CRGB* blade;
            if (currentPixels[i].tick == currentTick) { // Use blade1
                blade = blade1;
            } else { // Otherwise use blade2
                blade = blade2;
            }

            blade[23 - currentPixels[i].radius + 2] = currentPixels[i].color;

            if (!currentPixels[i].hold) {
                currentPixels.erase(currentPixels.begin() + i);
            }
        }
    }

    // if (tick == 90) {
    //     blade1[5] = CRGB::Blue;
    // }

    FastLED.show();
}

void finishTick(int tick) {
    // if (tick == 90 || tick == 180 || tick == 0 || tick == 270) {
        clear();
        // Serial.println(0);
    // }

    // blade1[15] = CRGB::Black;
    // blade2[15] = CRGB::Black;
    // FastLED.show();
    
}

void loop() {
    unsigned long long currentTime = micros();

    if (lastStateWasHigh && analogRead(A5) == 0) {
        const int rpm = MICROS_PER_MINUTE / (currentTime - lastHighStateTime);
        // Serial.println(rpm);
        rpmReadings[rpmIndex] = rpm;
        rpmIndex = (rpmIndex + 1) % 5;

        int min = NULL;
        for (int i = 0; i < 5; i++) {
            if (rpmReadings[i] < min && rpmReadings[i] >= 150 && rpmReadings[i] <= 250) min = rpmReadings[i];
        }

        if (min != NULL) {
            currentRpm = min;
        }

        lastStateWasHigh = false;
        lastHighStateTime = currentTime;
    } else if (analogRead(A5) > 0) {
        lastStateWasHigh = true;
    }

    int adjustment = -10 + (int) ((double) analogRead(A0) / 4096 * 20);
    if (currentTime >= lastFrameUpdateTime + MICROS_PER_TICK((currentRpm + adjustment))) {
        // currentRpm = 100 + analogRead(A0) * 100 / 1024;
        // Serial.println(MICROS_PER_TICK(currentRpm));
        // Serial.println(currentRpm);
        finishTick(currentTick);

        // Increment current tick by 1 and process it
        currentTick = (currentTick + 1) % TICK_RESOLUTION;
        processTick(currentTick);

        lastFrameUpdateTime = currentTime;

        // Serial.println(analogRead(A5));
        // Serial.println((long)(micros() - currentTime));
    }
}
