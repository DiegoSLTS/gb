#pragma once

#include "Cartridge.h"
#include "MMU.h"
#include "CPU.h"
#include "InterruptServiceRoutine.h"
#include "GPU.h"
#include "Timer.h"
#include "Joypad.h"
#include "SerialDataTransfer.h"
#include "Audio.h"
#include "Logger.h"

enum class EmulationModeSetting {
    Detect,
    GameBoy,
    GameBoyColor
};

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
    SerialDataTransfer serial;
    Audio audio;

    void MainLoop();

    bool frameFinished = false;
    bool sampleGenerated = false;

    bool syncWithAudio = true;

	bool IsCGB = false;

	bool isPaused = false;
	u8 stepsToEmulate = 0;
    void Reset();

    void ToggleLogging();

private:
    void LoadState();
    void SaveState();    

    bool isLogging = false;
	bool skipBios = false;

    Logger logger;
};
