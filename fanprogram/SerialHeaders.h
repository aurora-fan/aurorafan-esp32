

#ifndef SerialHeaders_h
#define SerialHeaders_h

enum SerialPrograms {
    kAlign = 0,
    kText = 1,
    kMusicVisualizer = 2
};

const int ALIGN_PROGRAM_START[] = { 255, 254, 253, 252, kAlign };
const int TEXT_PROGRAM_START[] = { 255, 254, 253, 252, kText };
const int MVIZ_PROGRAM_START[] = { 255, 254, 253, 252, kMusicVisualizer };

const int TEXT[] = { 255, 254, 253, 252, 10 };
const int MVIZ_VALUES[] = { 255, 254, 253, 252, 20 };
const int IMAGE[] = { 255, 254, 253, 252, 30 };
const int ANALOG_CLOCK[] = { 255, 254, 253, 252, 90 };

#endif
