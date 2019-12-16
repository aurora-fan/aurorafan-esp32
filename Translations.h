#include <vector>

#include "Pixel.h"

#ifndef Translations_h
#define Translations_h

class Translations {
    public:
        static std::vector<Pixel> movedBy(const uint8_t dX, const uint8_t dY, const std::vector<Pixel> pixels);

        static std::vector<Pixel> widenedBy(const uint8_t horizontalFactor, const uint8_t verticalFactor, const std::vector<Pixel> pixels);

        static std::vector<Pixel> rotatedBy(const int dTicks, const std::vector<Pixel> pixels);

        static std::vector<Pixel> movedOutBy(const uint8_t dRadius, const std::vector<Pixel> pixels);
};

#endif