#include <vector>

#include "Pixel.h"
#include "Translations.h"

std::vector<Pixel> Translations::movedBy(const uint8_t dX, const uint8_t dY, const std::vector<Pixel> pixels) {
    std::vector<Pixel> out;

    for (auto const& it: pixels) {
        out.insert(out.end(), Pixel::initWithImageCoordinates(it.x + dX, it.y + dY, it.color));
    }

    return out;
}

std::vector<Pixel> Translations::widenedBy(const uint8_t horizontalFactor, const uint8_t verticalFactor, const std::vector<Pixel> pixels) {
    std::vector<Pixel> out;

    for (auto const& it: pixels) {
        out.insert(out.end(), Pixel::initWithImageCoordinates(it.x * horizontalFactor, it.y * verticalFactor, it.color));
    }

    return out;
}

std::vector<Pixel> Translations::rotatedBy(const int dTicks, const std::vector<Pixel> pixels) {
    std::vector<Pixel> out;

    for (auto const& it: pixels) {
        out.insert(out.end(), Pixel::initWithPolarCoordinates(it.radius, it.tick + dTicks, it.color));
    }

    return out;
}

std::vector<Pixel> Translations::movedOutBy(const uint8_t dRadius, const std::vector<Pixel> pixels) {
    std::vector<Pixel> out;

    for (auto const& it: pixels) {
        out.insert(out.end(), Pixel::initWithPolarCoordinates(it.radius + dRadius, it.tick, it.color));
    }

    return out;
}