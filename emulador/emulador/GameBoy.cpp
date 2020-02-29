#include "GameBoy.h"
#include "RomParser.h"

GameBoy::GameBoy(const std::string& RomPath) : cartridge(RomPath), cpu(mmu), gpu(mmu), timer(mmu), joypad(mmu), dma(mmu) {
    cpu.interruptService = &interruptService;

    mmu.cartridge = &cartridge;
    mmu.interruptServiceRoutine = &interruptService;
    mmu.gpu = &gpu;
    mmu.timer = &timer;
    mmu.joypad = &joypad;
    mmu.dma = &dma;
    mmu.serial = &serial;
    mmu.audio = &audio;

    bool parseRom = false;
    if (parseRom) {
        RomParser parser;
        // TODO support more cartridge sizes
        // TODO move parse logic into Cartridge class
        parser.ParseCartridgeROM(cartridge.GetRomPtr(), 32 * 1024);
        parser.PrintCodeToFile();
    }

    if (skipBios)
        LoadState();
}

GameBoy::~GameBoy() {}

void GameBoy::MainLoop() {
    if (interruptService.IE & interruptService.IF) {
        if (cpu.isHalted) {
            cpu.isHalted = false;
            cpu.lastOpCycles = 1;
        }
        if (interruptService.IME) {
            for (int i = 0; i < 5; i++) {
                if (interruptService.IsInterruptSet(i) && interruptService.IsInterruptEnabled(i)) {
                    interruptService.IME = false;
                    mmu.ResetInterruptFlag(i);
                    cpu.Push16(cpu.pc); // cpu.lastOpCycles = 2
                    cpu.pc = 0x40 + i * 8; //0x40, 0x48, 0x50, 0x58, 0x60
                    cpu.lastOpCycles += 3; // +2 idle cycles, +1 updating PC
                    break;
                }
            }
        }
    }

    if (interruptService.eiDelay) {
        interruptService.IME = true;
        interruptService.eiDelay = false;
    }

    if (!skipBios && cpu.pc == 0x100)
        SaveState();

    // used only for debugging to break at specific instructions
    if (cpu.pc == 0x3155 /*0x3306*/) {
        int a = 0;
    }

    if (!cpu.isHalted) {
        u8 opCode = cpu.ReadOpCode();
        if (opCode == 0xCB) {
            opCode = cpu.ReadOpCode();
            cpu.CallCBOpCode(opCode);
        }
        else
            cpu.CallOpCode(opCode);
    }
    else
        cpu.lastOpCycles = 1;

    u8 lastOpCycles = cpu.lastOpCycles;
    frameFinished = gpu.Step(lastOpCycles * 4);

    timer.Step(lastOpCycles * 4);
    dma.Step(lastOpCycles);

    cpu.lastOpCycles = 0;

    sampleGenerated = audio.Step(lastOpCycles);
}

void GameBoy::LoadState() {
    std::ifstream stream("bootState.txt", std::ios::binary);

    cpu.Load(stream);
    mmu.Load(stream);
    gpu.Load(stream);
    interruptService.Load(stream);
    //dma.Load(stream);

    stream.close();
}

void GameBoy::SaveState() {
    std::ofstream stream("bootState.txt", std::ios::binary);

    cpu.Save(stream);
    mmu.Save(stream);
    gpu.Save(stream);
    interruptService.Save(stream);
    //dma.Save(stream);

    stream.close();
}
