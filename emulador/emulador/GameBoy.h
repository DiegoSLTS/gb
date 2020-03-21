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

    bool skipBios = false;
    bool syncWithAudio = true;

    EmulationModeSetting emulationModeSetting = EmulationModeSetting::Detect;
	bool IsCGB = false;
    bool IsNonCGBMode = false;

    void Pause();
    void Resume();
    bool IsPaused();
    void Reset();

    void ToggleLogging();

private:
    void LoadState();
    void SaveState();    

    bool isPaused = false;
    bool isLogging = false;

    Logger logger;
};
