#include <SFML/Graphics.hpp>
#include <fstream>

#include "Cartridge.h"
#include "MMU.h"
#include "CPU.h"
#include "GPU.h"
#include "Timer.h"
#include "Joypad.h"
#include "DMA.h"
#include "SerialDataTransfer.h"
#include "InterruptServiceRoutine.h"
#include "RomParser.h"
#include "Window.h"
#include "TileViewer.h"
#include "TileMapViewer.h"
#include "SpritesViewer.h"
#include "StateViewer.h"

void printCPUFlags(const CPU& cpu) {
	printf("%u%u%u%u0000\n", cpu.HasFlag(FlagBit::Zero), cpu.HasFlag(FlagBit::Negative), cpu.HasFlag(FlagBit::HalfCarry), cpu.HasFlag(FlagBit::Carry));
}

bool isLittleEndian() {
	uint8_t test[2] = { 0x1, 0x0 };
	uint16_t* t16 = (uint16_t*)test;
	printf("temp: %u", *t16);
	return *t16 == test[0];
}

void UpdateSFMLScreenArray(sf::Uint8 sfmlScreen[], uint8_t gpuScreen[]) {
	for (int y = 0; y < 160; y++) {
		for (int x = 0; x < 144; x++) {
			uint16_t index = y * 144 + x;
			uint8_t gpuColor = 255 - (gpuScreen[index] / 3.0f) * 255;
			sfmlScreen[index * 4] = gpuColor;
			sfmlScreen[index * 4 + 1] = gpuColor;
			sfmlScreen[index * 4 + 2] = gpuColor;
			sfmlScreen[index * 4 + 3] = 0xFF;
		}
	}
}

void SaveState(const CPU& cpu, const GPU& gpu, const MMU& mmu, const InterruptServiceRoutine& interruptService) {
	std::ofstream stream;
	stream.open("bootState.txt", std::ios::binary);

	cpu.Save(stream);
	mmu.Save(stream);
	gpu.Save(stream);
	interruptService.Save(stream);
	
	stream.close();
}

void LoadState(const CPU& cpu, const GPU& gpu, const MMU& mmu, const InterruptServiceRoutine& interruptService) {
	std::ifstream stream;
	stream.open("bootState.txt", std::ios::binary);

	cpu.Load(stream);
	mmu.Load(stream);
	gpu.Load(stream);
	interruptService.Load(stream);

	stream.close();
}

int main() {
	std::string romsPath = "D:\\Programacion\\gb\\roms\\";
	//std::string romName = "Alleyway (World).gb";
	//std::string romName = "Amida (Japan).gb";
	//std::string romName = "Asteroids (USA, Europe).gb";
	//std::string romName = "BattleCity (Japan).gb";
	//std::string romName = "Bomb Jack (Europe).gb";
	//std::string romName = "Bouken! Puzzle Road (Japan).gb";
	//std::string romName = "Boxxle (USA, Europe) (Rev A).gb";
	//std::string romName = "Boxxle II (USA, Europe).gb";
	//std::string romName = "Brain Bender (Europe).gb";
	//std::string romName = "Bubble Ghost (USA, Europe).gb";
	//std::string romName = "Castelian (Europe).gb"; /!!!!!!!!!!!!!!/
	//std::string romName = "Catrap (USA).gb";
	//std::string romName = "Centipede (USA, Europe).gb";
	//std::string romName = "Chiki Chiki Tengoku (Japan).gb";
	//std::string romName = "Cool Ball (USA).gb";
	//std::string romName = "Daedalian Opus (USA).gb";
	//std::string romName = "Crystal Quest (USA).gb";
	//std::string romName = "Dr. Mario (World) (Rev A).gb";
	//std::string romName = "Dragon Slayer I (Japan).gb";
	//std::string romName = "Dropzone (Europe).gb";
	//std::string romName = "Elevator Action (Japan).gb";
	//std::string romName = "Flappy Special (Japan).gb";
	//std::string romName = "Flipull (USA).gb";
	//std::string romName = "Klax (USA).gb";
	//std::string romName = "Pipe Dream (USA).gb";
	//std::string romName = "Spot (USA).gb";
	std::string romName = "Tetris (World) (Rev A).gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\01-special.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\02-interrupts.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\03-op sp,hl.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\04-op r,imm.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\05-op rp.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\06-ld r,r.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\07-jr,jp,call,ret,rst.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\08-misc instrs.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\09-op r,r.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\10-bit ops.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\11-op a,(hl).gb";
	//std::string romName = "gb-test-roms-master\\instr_timing\\instr_timing.gb";
	//std::string romName = "gb-test-roms-master\\interrupt_time\\interrupt_time.gb";
	//std::string romName = "gb-test-roms-master\\mem_timing\\individual\\01-read_timing.gb";
	//std::string romName = "gb-test-roms-master\\mem_timing\\individual\\02-write_timing.gb";
	//std::string romName = "gb-test-roms-master\\mem_timing\\individual\\03-modify_timing.gb";
	//std::string romName = "gb-test-roms-master\\mem_timing-2\\individual\\01-read_timing.gb";
	//std::string romName = "gb-test-roms-master\\mem_timing-2\\individual\\02-write_timing.gb";
	//std::string romName = "gb-test-roms-master\\mem_timing-2\\individual\\03-modify_timing.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\rom_singles\\1-lcd_sync.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\rom_singles\\2-causes.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\rom_singles\\3-non_causes.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\rom_singles\\4-scanline_timing.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\rom_singles\\5-timing_bug.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\rom_singles\\6-timing_no_bug.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\rom_singles\\7-timing_effect.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\rom_singles\\8-instr_effect.gb";
	//std::string romName = "gb-test-roms-master\\halt_bug.gb";

	Cartridge cartridge;
	cartridge.LoadFile(romsPath.append(romName));
	
	/*RomParser parser;
	parser.ParseCartridgeROM(cartridge.rom, 32 * 1024);
	parser.PrintCodeToFile();*/
	
	MMU mmu;
	mmu.cartridge = &cartridge;

	CPU cpu;
	cpu.mmu = &mmu;

	InterruptServiceRoutine interruptService;
	interruptService.mmu = &mmu;
	mmu.interruptServiceRoutine = &interruptService;
	cpu.interruptService = &interruptService;

	GPU gpu;
	gpu.mmu = &mmu;
	mmu.gpu = &gpu;

	Timer timer;
	timer.mmu = &mmu;
	mmu.timer = &timer;

	Joypad joypad;
	mmu.joypad = &joypad;

	DMA dma;
	dma.mmu = &mmu;
	mmu.dma = &dma;

	SerialDataTransfer serial;
	mmu.serial = &serial;

	bool skipBios = true;
	if (skipBios)
		LoadState(cpu, gpu, mmu, interruptService);

	Window gameWindow(160, 144, "Game");
	sf::Vector2i p(50, 50);
	gameWindow.renderWindow->setPosition(p);
	
	TileViewer tilesWindow(168, 144, "Tiles", &mmu, 0x8000);
	p.x = 385;
	p.y = 50;
	tilesWindow.renderWindow->setPosition(p);

	SpritesViewer spritesWindow(256 + 8, 256 + 16, "Sprites", &mmu);
	p.x = 735;
	p.y = 50;
	spritesWindow.renderWindow->setPosition(p);

	/*StateViewer state(&cpu, &gpu, &cpu.mmu);
	p.x = 735;
	p.y = 50;
	state.renderWindow->setPosition(p);*/

	TileMapViewer tileMap0Window(256, 256, "Tile map 0", &mmu, 0x9800);
	p.x = 50;
	p.y = 380;
	tileMap0Window.renderWindow->setPosition(p);

	TileMapViewer tileMap1Window(256, 256, "Tile map 1", &mmu, 0x9C00);
	p.x = 580;
	p.y = 380;
	tileMap1Window.renderWindow->setPosition(p);

	while (gameWindow.renderWindow->isOpen()) {
		if (interruptService.eiDelay) {
			interruptService.IME = true;
			interruptService.eiDelay = false;
		} else if (interruptService.IME) {
			for (int i = 0; i < 5; i++) {
				if (interruptService.IsInterruptSet(i) && interruptService.IsInterruptEnabled(i)) {
					cpu.isHalted = false;
					interruptService.IME = false;
					mmu.ResetInterruptFlag(i);
					cpu.Push16(cpu.pc); // cpu.lastOpCycles = 2
					cpu.pc = 0x40 + i * 8; //0x40, 0x48, 0x50, 0x58, 0x60
					cpu.lastOpCycles += 3; // +2 idle cycles, +1 updating PC
					break;
				}
			}
		}
		
		if (!skipBios && cpu.pc == 0x100)
			SaveState(cpu, gpu, mmu, interruptService);

		if (cpu.pc == 0x799) {
			int a = 0;
		}

		if (!cpu.isHalted) {
			uint8_t opCode = cpu.ReadOpCode();
			bool isCB = opCode == 0xCB;
			if (isCB)
				opCode = cpu.ReadOpCode();

			if (isCB)
				cpu.CallCBOpCode(opCode);
			else
				cpu.CallOpCode(opCode);
		} else
			cpu.lastOpCycles = 1;

		uint8_t lastOpCycles = cpu.lastOpCycles;
		if (gpu.Step(lastOpCycles * 4)) {
			UpdateSFMLScreenArray(gameWindow.screenArray, gpu.screen);
			gameWindow.screenTexture.update(gameWindow.screenArray);
			gameWindow.screenSprite.setTexture(gameWindow.screenTexture, true);

			gameWindow.renderWindow->clear();
			gameWindow.renderWindow->draw(gameWindow.screenSprite);
			gameWindow.renderWindow->display();

			tilesWindow.Update();
			tileMap0Window.Update();
			tileMap1Window.Update();
			spritesWindow.Update();
		}

		//state.Update();

		timer.Step(lastOpCycles * 4);
		dma.Step(lastOpCycles);
		
		cpu.lastOpCycles = 0;

		sf::Event event;
		while (gameWindow.renderWindow->pollEvent(event))
			if (event.type == sf::Event::Closed)
				gameWindow.renderWindow->close();

		/*while (tilesWindow.renderWindow->pollEvent(event))
			if (event.type == sf::Event::Closed)
				tilesWindow.renderWindow->close();*/
	}
	
	return 0;
}