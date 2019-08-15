#pragma once

#include "Types.h"
#include <queue>

class GPU;

enum PixelSource {
    Background,
    Window,
    Pixel
};

struct PixelInfo {
    u8 color = 0;
    PixelSource source = PixelSource::Background;
    u8 palette = 0;
};

class PixelFifo {
public:
    PixelFifo();
    virtual ~PixelFifo();

    std::queue<PixelInfo> queue;
    GPU* gpu = nullptr;

    // returns true if already pushed a whole line (160 pixels)
    bool Step(u8 cycles);

    bool enoughPixels = false;

    u16 fetchAddress = 0;
    u8 tileID = 0;
    u8 lowData = 0;
    u8 highData = 0;
    bool fetchFinished = false;
    bool fetchingSprite = false;
    u8 fetchCycles = 0;

    u16 lastPixelIndex = 0;
};