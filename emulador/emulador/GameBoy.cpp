#include "GameBoy.h"
#include "RomParser.h"

GameBoy::GameBoy(const std::string& RomPath)
	: cartridge(RomPath), cpu(mmu, interruptService), gpu(mmu, interruptService), timer(interruptService), joypad(interruptService), serial(interruptService) {

	// TODO load settings from a settings file
	EmulationModeSetting emulationModeSetting = EmulationModeSetting::Detect;
	bool skipBios = true;
	syncWithAudio = false;
    bool startLogging = false;
    isPaused = false;

	switch (emulationModeSetting) {
	case EmulationModeSetting::Detect:
		IsCGB = cartridge.IsGBCCartridge();
		break;
	case EmulationModeSetting::GameBoy:
		IsCGB = false;
		break;
	case EmulationModeSetting::GameBoyColor:
		IsCGB = true;
		break;
	}

	gpu.isCGB = IsCGB;

    mmu.cpu = &cpu;
    mmu.cartridge = &cartridge;
    mmu.interruptServiceRoutine = &interruptService;
    mmu.gpu = &gpu;
    mmu.timer = &timer;
    mmu.joypad = &joypad;
    mmu.serial = &serial;
    mmu.audio = &audio;
    
    /*RomParser parser;
    parser.ParseCartridgeROM(cartridge.GetRomPtr(), 0x200);
    parser.PrintCodeToFile();*/

    if (skipBios)
        LoadState();
	else
		mmu.LoadBootRom(IsCGB);

    if (startLogging)
        ToggleLogging();
}

GameBoy::~GameBoy() {}

void GameBoy::ToggleLogging() {
    cpu.log = !cpu.log;
    gpu.log = !gpu.log;
    gpu.dma.log = !gpu.dma.log;
    cartridge.mbc->log = !cartridge.mbc->log;
    joypad.log = !joypad.log;
    interruptService.log = !interruptService.log;
}

void GameBoy::Reset() {
    cpu.pc = 0;
    mmu.Write(0xFF50, 0);
    // TODO reset all components
}

void GameBoy::MainLoop() {
    if (isPaused)
        return;

	// boot rom finished, entry point of game code
	//if (cpu.pc == 0x100 && !skipBios)
	//	SaveState();

    u8 lastOpCycles = cpu.Step();
    frameFinished = gpu.Step(lastOpCycles * 4, cpu.IsDoubleSpeedEnabled());
    timer.Step(lastOpCycles * 4);
    sampleGenerated = audio.Step(lastOpCycles, cpu.IsDoubleSpeedEnabled());
    serial.Step(lastOpCycles * 4, cpu.IsDoubleSpeedEnabled());

	if (stepsToEmulate > 0) {
		stepsToEmulate--;
		if (stepsToEmulate == 0)
			isPaused = true;
	}
}

void GameBoy::LoadState() {
    std::string bootStateFileName = IsCGB ? "CGBBootState.bin" : "DMGBootState.bin";
    std::ifstream stream(bootStateFileName, std::ios::binary);

    cpu.Load(stream);
    mmu.Load(stream);
    gpu.Load(stream);
    interruptService.Load(stream);
	// TODO Load audio, timer, joystick, serial

    stream.close();
}

void GameBoy::SaveState() {
    std::string bootStateFileName = IsCGB ? "CGBBootState.bin" : "DMGBootState.bin";
    std::ofstream stream(bootStateFileName, std::ios::binary);

    cpu.Save(stream);
    mmu.Save(stream);
    gpu.Save(stream);
    interruptService.Save(stream);
	// TODO Save audio, timer, joystick, serial

    stream.close();
}
