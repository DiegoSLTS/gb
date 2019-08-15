#include "PixelFifo.h"
#include "GPU.h"

PixelFifo::PixelFifo() {}
PixelFifo::~PixelFifo() {}

bool PixelFifo::Step(u8 cycles) {
    if (queue.size() > 8) {
        PixelInfo info = queue.front();
        queue.pop();

        u8 palette = info.palette;
        const u8 paletteMask = 0b00000011;

        // get final color
        u8 gbColor = (palette & (paletteMask << (info.color << 1))) >> (info.color << 1);

        // send to GPU
        // screen[lastPixelIndex] = gbColor;
        lastPixelIndex++;

        // whole line processed
        if (lastPixelIndex == 160)
            return true;
    } else if (fetchFinished) {
        // push all PixelInfo
        // TODO if flip x true, reverse fetched data
        if (fetchingSprite) {
            // TODO merge with current queue  (consider horizontal flip)
        } else {
            // TODO push into queue (consider horizontal flip)
        }
        fetchFinished = false;
    }
    
    if (!fetchFinished) { // start/continue fetch
        if (fetchCycles + cycles >= 2) {
            cycles -= 2 - fetchCycles;
            fetchCycles = 2;
            // TODO fetch tileID
        }

        if (fetchCycles + cycles >= 4) {
            cycles -= 4 - fetchCycles;
            fetchCycles = 4;
            // TODO fetch low byte
        }

        if (fetchCycles + cycles >= 6) {
            cycles -= 6 - fetchCycles;
            fetchCycles = 6;
            // TODO fetch high byte

            /*PixelInfo info;
            
            switch (info.source) {
            case PixelSource::Background:
            case PixelSource::Window:
                palette = gpu->BGP;
                break;
            case PixelSource::Pixel:
                palette = 0; //TODO GPU::OBP0 or GPU::OBP1
                break;
            }*/
            fetchFinished = true;
        }
    }

    return false;
}