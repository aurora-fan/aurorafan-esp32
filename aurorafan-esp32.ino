#include <string>
#include <vector>

#include <FastLED.h>
#include <Wire.h>

#include "fanprogram/IFanProgram.h"
#include "FastLED_RGBW.h"
#include "Pixel.h"
#include "fanprogram/SerialHeaders.h"
#include "Text.h"
#include "Translations.h"

#define SIDE_LENGTH 52
#define LEDS_PER_BLADE 24
#define BLADE_1_PORT 15
#define BLADE_2_PORT 27

#define TICK_RESOLUTION 50 // Ticks per rotation
#define MICROS_PER_MINUTE 60000000
#define MICROS_PER_SECOND 1000000

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

void switchProgramIfNeeded(const int programId);

//////
class AlignProgram : virtual public IFanProgram {
    public:
        AlignProgram(): IFanProgram(MICROS_PER_SECOND / 2) { }

        virtual std::vector<Pixel> getCurrentFrame(const unsigned long micros) {
            uint8_t frameHeader[5];

            std::vector<Pixel> pixels;

            if (Serial.available()) {
                Serial.readBytes(frameHeader, 5);

                const int newProgram = checkSerialHeadersForNewProgram(frameHeader);
                switchProgramIfNeeded(newProgram);
            }

            return {
                Pixel::initWithImageCoordinates(26, 10, CRGB::Green),
                Pixel::initWithImageCoordinates(10, 26, CRGB::Blue),
                Pixel::initWithImageCoordinates(36, 26, CRGB::Red)
            };
        }
};
//////

//////
class TextProgram : virtual public IFanProgram {
    private:
        unsigned int mMarqueePosition;
        const int WINDOW_WIDTH = 30;

    protected:
        std::vector<Pixel> mLetters;
        std::vector<Pixel> mPixelsInMarquee;

    public:
        TextProgram(const unsigned int refreshRate): IFanProgram(refreshRate) { }

        void displayText(const char letters[], const int textLength, const bool shouldBlink, const CRGB color, const uint8_t radius) {
            uint8_t pos = 0;
            std::vector<Pixel> pixels;
            for (int i = 0; i < textLength; i++) {
                if (letters[i] == ' ') {
                    pos += 2;
                    continue;
                }

                std::vector<Pixel> dots = Translations::movedOutBy(radius, Translations::rotatedBy(pos, Text::pixelsForChar(letters[i])));

                for (auto& pixel: dots) {
                    pixel.color = color;
                    pixels.push_back(pixel);
                    Serial.println(letters[i]);
                    Serial.println(pixel.tick);
                }
                
                pos += 6;
            }

            mLetters = pixels;
            mMarqueePosition = 0;
        }

        void rotateMarqueeIfNeeded() {
            int min = mMarqueePosition;
            int max = min + WINDOW_WIDTH;
            int maxPixelTick = -1;

            std::vector<Pixel> visiblePixels;
            for (auto const& pixel : mLetters) {
                // Sliding window on the pixels
                if (pixel.tick >= min && pixel.tick <= max) {
                    visiblePixels.push_back(pixel);
                }

                if (pixel.tick > maxPixelTick) {
                    maxPixelTick = pixel.tick;
                }
            }

            if (maxPixelTick <= max) { // Either all text fits in window or the end has been reached
                mMarqueePosition = 0;
            } else { // Still remaining text
                mMarqueePosition++;
            }

            mPixelsInMarquee = Translations::rotatedBy(-min, visiblePixels);
        }
};
//////

//////
class MusicVisualizer : virtual public TextProgram {
    public:
        MusicVisualizer(): TextProgram(MICROS_PER_SECOND / 10), IFanProgram(MICROS_PER_SECOND / 10) {
            Serial.println("Music visualizer");
        }
        
        virtual std::vector<Pixel> getCurrentFrame(const unsigned long micros) {
            uint8_t frameHeader[5];

            std::vector<Pixel> pixels;

            if (Serial.available()) {
                Serial.readBytes(frameHeader, 5);

                const int newProgram = checkSerialHeadersForNewProgram(frameHeader);
                switchProgramIfNeeded(newProgram);

                for (int i = 0; i < 5; i++) {
                    if (frameHeader[i] != MVIZ_VALUES[i] && frameHeader[i] != TEXT[i]) {
                        Serial.flush();
                        return std::vector<Pixel>();
                    }
                }

                if (frameHeader[4] == MVIZ_VALUES[4]) {
                    uint8_t tick = 0;
                    uint8_t levels[50];

                    Serial.readBytes(levels, 50);
                    
                    for (int i = 0; i < 50; ++i) {
                        if (levels[i] > 8 || levels[i] < 0) continue;

                        CRGB color;
                        if (levels[i] >= 8) color = CRGB::Red;
                        else if (levels[i] >= 7) color = CRGB::Yellow;
                        else color = CRGB::Green;

                        for (int j = 0; j < levels[i] * 2; ++j) {
                            pixels.push_back(Pixel::initWithPolarCoordinates(j + 12, tick, color));
                        }

                        ++tick;
                    }
                } else if (frameHeader[4] == TEXT[4]) {
                    Serial.readBytes(frameHeader, 5);
                    const CRGB textColor = CRGB(frameHeader[0], frameHeader[1], frameHeader[2]);
                    const bool shouldBlink = frameHeader[3];
                    const uint8_t textLength = frameHeader[4];

                    char letters[textLength];
                    Serial.readBytes(letters, textLength);

                    displayText(letters, textLength, shouldBlink, textColor, 5);
                    rotateMarqueeIfNeeded();
                }

                for (auto const& pixel : mPixelsInMarquee) {
                    pixels.push_back(pixel);
                }
                rotateMarqueeIfNeeded();
            } else {
                pixels.push_back(Pixel::initWithPolarCoordinates(3, 0, CRGB::Red));
            }

            return pixels;
        }
};
//////

//////
class SerialTextProgram : virtual public TextProgram {
    private:
        std::vector<Pixel> image;
        std::vector<Pixel> clock;

        void buildClockHand(const uint8_t length, const uint8_t tick, const CRGB color) {
            for (int i = 2; i < length; ++i) {
                clock.push_back(Pixel::initWithPolarCoordinates(i, tick, color));
            }
        }

    public:
        SerialTextProgram(): TextProgram(MICROS_PER_SECOND / 4), IFanProgram(MICROS_PER_SECOND / 4) { // Dear god why is c++ like this
            Serial.println("Serial Text");
        }
        
        virtual std::vector<Pixel> getCurrentFrame(const unsigned long micros) {
            uint8_t frameHeader[5];

            std::vector<Pixel> pixels;

            if (Serial.available()) {
                Serial.readBytes(frameHeader, 5);

                const int newProgram = checkSerialHeadersForNewProgram(frameHeader);
                switchProgramIfNeeded(newProgram);

                for (int i = 0; i < 5; i++) {
                    if (frameHeader[i] != TEXT[i] && frameHeader[i] != IMAGE[i] && frameHeader[i] != ANALOG_CLOCK[i]) return pixels;
                }

                if (frameHeader[4] == TEXT[4]) {
                    Serial.readBytes(frameHeader, 5);
                    const CRGB textColor = CRGB(frameHeader[0], frameHeader[1], frameHeader[2]);
                    const bool shouldBlink = frameHeader[3];
                    const uint8_t textLength = frameHeader[4];

                    char letters[textLength];
                    Serial.readBytes(letters, textLength);

                    Serial.println(textColor.g);
                    Serial.println(textLength);

                    displayText(letters, textLength, shouldBlink, textColor, 10);
                    rotateMarqueeIfNeeded();
                    pixels = mPixelsInMarquee;
                } else if (frameHeader[4] == IMAGE[4]) {
                    // We cannot support image transfer because the serial bus loses too many packets so the image comes out garbled
                    // std::vector<Pixel> tempImage;
                    // uint8_t buffer[4];
                    // Serial.readBytes(buffer, 4);
                    // const uint8_t startingX = buffer[0];
                    // const uint8_t startingY = buffer[1];
                    // const int amount = buffer[2] * 16 + buffer[3];

                    // uint8_t data[5 * amount];
                    // Serial.readBytes(data, 5 * amount);
                    // for (int i = 0; i < amount * 5; i += 5) {
                    //     const Pixel pixel = Pixel::initWithImageCoordinates(data[i + 0] + startingX,
                    //             data[i + 1] + startingY,
                    //             CRGB(data[i + 2], data[i + 3], data[i + 4]));
                    //     tempImage.push_back(pixel);
                    // }

                    uint8_t buffer[3];
                    Serial.readBytes(buffer, 3);
                    const uint8_t imageId = buffer[0];
                    const uint8_t startingX = buffer[1];
                    const uint8_t startingY = buffer[2];

                    image = Translations::rotatedBy(25, Translations::movedBy(startingX, startingY, Text::cloud));
                } else if (frameHeader[4] == ANALOG_CLOCK[4]) {
                    uint8_t buffer[6];
                    Serial.readBytes(buffer, 6);
                    const double hours = buffer[0];
                    const double minutes = buffer[1];
                    const double seconds = buffer[2];
                    const CRGB tickColor = CRGB(buffer[3], buffer[4], buffer[5]);

                    image = std::vector<Pixel>();
                    mLetters = std::vector<Pixel>();
                    clock = std::vector<Pixel>();

                    for (int i = 0; i < 60; i += 5) {
                        const uint8_t tick = (double) i / 60.0 * 50.0;

                        for (int j = ((i == 0) ? 19 : 22); j < 26; ++j) {
                            clock.push_back(Pixel::initWithPolarCoordinates(j, tick, tickColor));
                        }
                    }

                    const uint8_t hourTick = 50 - (hours * 5.0 + minutes / 60.0 * 5.0) * 50.0 / 60.0;
                    const uint8_t minuteTick = 50 - minutes * 50.0 / 60.0;
                    const uint8_t secondsTick = 50 - seconds * 50.0 / 60.0;

                    buildClockHand(10, hourTick, CRGB::Blue);
                    buildClockHand(14, minuteTick, CRGB::Green);
                    buildClockHand(18, secondsTick, CRGB::Red);

                    pixels = clock;
                }
            } else if (!mLetters.empty() || !image.empty()) {
                pixels = mPixelsInMarquee;
                rotateMarqueeIfNeeded();

                for (auto const& pixel : image) {
                    pixels.push_back(pixel);
                }
            } else if (!clock.empty()) {
                pixels = clock;
            } else {
                pixels.push_back(Pixel::initWithPolarCoordinates(3, 0, CRGB::Red));
            }

            return pixels;
        }
};
//////

// Current program: program that controls the fan; inherits from IFanProgram
IFanProgram* currentProgram = nullptr;

void switchProgramIfNeeded(const int programId) {
    Serial.println(programId);
    
    if (programId == kAlign) {
        currentProgram = new AlignProgram();
    } else if (programId == kText) {
        currentProgram = new SerialTextProgram();
    } else if (programId == kMusicVisualizer) {
        currentProgram = new MusicVisualizer();
    }
}

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
    
    currentProgram = new AlignProgram();

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

            const int ledIndex = 23 - currentPixels[i].radius + 2;
            if (ledIndex >= 0 && ledIndex < LEDS_PER_BLADE) {
                // Only light pixel within valid radius range
                blade[ledIndex] = currentPixels[i].color;
            }

            if (!currentPixels[i].hold) {
                currentPixels.erase(currentPixels.begin() + i);
            }
        }
    }

    FastLED.show();

    clear();
}

void finishTick(int tick) {
    
}

unsigned long long lastPixelChangeTime = 0;

void loop() {
    unsigned long long currentTime = micros();

    if (currentTime >= lastPixelChangeTime + currentProgram->getRefreshRate()) {
        currentPixels = currentProgram->getCurrentFrame(currentTime);

        lastPixelChangeTime = currentTime;
    }

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
