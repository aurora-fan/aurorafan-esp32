#include <vector>

#include "../Pixel.h"

#ifndef IFANPROGRAM_H
#define IFANPROGRAM_H

class IFanProgram {
    protected:
        const unsigned int mRefreshRate;

    public:
        IFanProgram(const unsigned int refreshRate);

        const unsigned int getRefreshRate() {
            return mRefreshRate;
        }

        virtual std::vector<Pixel> getCurrentFrame(const unsigned long micros) = 0; // Abstract method
};

#endif