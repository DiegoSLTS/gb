#pragma once

#include "Cartridge.h"
#include "MMU.h"
#include "CPU.h"
#include "InterruptServiceRoutine.h"
#include "GPU.h"
#include "Timer.h"
#include "Joypad.h"
#include "DMA.h"
#include "SerialDataTransfer.h"
#include "Audio.h"

class GameBoy {
public:
    GameBoy(const std::string& romPath);
    virtual ~GameBoy();

    Cartridge cartridge;
    MMU mmu;
    CPU cpu;
    InterruptServiceRoutine interruptService;
    GPU gpu;
    Timer timer;
    Joypad joypad;
    DMA dma;
    SerialDataTransfer serial;
    Audio audio;

    void MainLoop();

    bool frameFinished = false;
    bool sampleGenerated = false;

    bool skipBios = false;
    bool syncWithAudio = true;

private:
    void LoadState();
    void SaveState();    
};
