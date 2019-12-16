#include <vector>

#include "../Pixel.h"
#include "SerialHeaders.h"

#ifndef IFANPROGRAM_H
#define IFANPROGRAM_H

class IFanProgram {
    protected:
        const unsigned int mRefreshRate;

    public:
        IFanProgram(const unsigned int refreshRate): mRefreshRate(refreshRate) { };

        const unsigned int getRefreshRate() {
            return mRefreshRate;
        }

        virtual std::vector<Pixel> getCurrentFrame(const unsigned long micros) = 0; // Abstract method

        int checkSerialHeadersForNewProgram(uint8_t serialHeader[]) {
            for (int i = 0; i < 4; i++) {
                if (serialHeader[i] != ALIGN_PROGRAM_START[i]) return -1;
            }

            return (serialHeader[4] >= 0 && serialHeader[4] < 10) ? serialHeader[4] : -1;
        }
};

#endif