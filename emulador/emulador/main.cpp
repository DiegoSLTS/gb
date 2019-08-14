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
			// turn gpuScreen value [0,3] into an 8 bit value [255,0]
			uint8_t gpuColor = 255 - gpuScreen[index] * 85; // 85 == 255/3
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
	//std::string romName = "Asteroids (USA, Europe).gb"; // broken window over background, freezes during gameplay
	//std::string romName = "BattleCity (Japan).gb"; // broken priority
	//std::string romName = "Bomb Jack (Europe).gb";
	//std::string romName = "Bouken! Puzzle Road (Japan).gb"; // "Daedalian Opus (USA).gb"
	//std::string romName = "Boxxle (USA, Europe) (Rev A).gb";
	//std::string romName = "Boxxle II (USA, Europe).gb"; // broken priority
	//std::string romName = "Brain Bender (Europe).gb"; // broken stretch effect and level layout
	//std::string romName = "Bubble Ghost (USA, Europe).gb";
	//std::string romName = "Castelian (Europe).gb"; // "Kyoro-chan Land (Japan).gb"
	//std::string romName = "Catrap (USA).gb";
	//std::string romName = "Centipede (USA, Europe).gb";
	//std::string romName = "Chiki Chiki Tengoku (Japan).gb";
	//std::string romName = "Cool Ball (USA).gb";
	//std::string romName = "Daedalian Opus (USA).gb"; // "Bouken! Puzzle Road (Japan).gb"
	//std::string romName = "Crystal Quest (USA).gb"; // no enemies
	//std::string romName = "Dr. Mario (World) (Rev A).gb";
	//std::string romName = "Dragon Slayer I (Japan).gb"; // broken map/offsets
	//std::string romName = "Dropzone (Europe).gb"; // broken splash, broken shots
	//std::string romName = "Flappy Special (Japan).gb";
	//std::string romName = "Flipull (USA).gb";
	//std::string romName = "Heiankyo Alien (USA).gb";
	//std::string romName = "Hong Kong (Japan).gb";
	//std::string romName = "Hyper Lode Runner (World) (Rev A).gb";
	//std::string romName = "Ishido - The Way of Stones (Japan).gb"; // level doesn't start
	//std::string romName = "Kakomunja (Japan).gb";
	//std::string romName = "Klax (Japan).gb";
	//std::string romName = "Koi wa Kakehiki (Japan).gb";
	//std::string romName = "Koro Dice (Japan).gb";
	//std::string romName = "Kwirk - He's A-maze-ing! (USA, Europe).gb";
	//std::string romName = "Kyoro-chan Land (Japan).gb"; // Castelian (Europe).gb
	//std::string romName = "Loopz (World).gb"; // no image? window over everything?
	//std::string romName = "Master Karateka (Japan).gb"; // broken background
	//std::string romName = "Migrain (Japan).gb"; // "Brain Bender (Europe).gb"
	//std::string romName = "Minesweeper - Soukaitei (Japan).gb";
	//std::string romName = "Missile Command (USA, Europe).gb";
	//std::string romName = "Mogura de Pon! (Japan).gb";
	//std::string romName = "Motocross Maniacs (Europe) (Rev A).gb"; // level broken offsets
	//std::string romName = "NFL Football (USA).gb"; // broken field
	//std::string romName = "Othello (Europe).gb"; // "Game Start" broken
	//std::string romName = "Palamedes (Europe).gb";
	//std::string romName = "Penguin Land (Japan).gb";
	//std::string romName = "Pipe Dream (USA).gb";
	//std::string romName = "Pitman (Japan).gb"; //"Catrap (USA).gb"
	//std::string romName = "Pop Up (Europe).gb"; //"Cool Ball (USA).gb"
	//std::string romName = "Puzzle Boy (Japan).gb"; //"Kwirk - He's A-maze-ing! (USA, Europe).gb"
	//std::string romName = "Q Billion (USA).gb"; // sprites offset
	//std::string romName = "Serpent (USA).gb";
	//std::string romName = "Shanghai (USA).gb"; // broken layout
	//std::string romName = "Soukoban (Japan).gb"; // "Boxxle (USA, Europe) (Rev A).gb"
	//std::string romName = "Soukoban 2 (Japan).gb"; // "Boxxle II (USA, Europe).gb"
	//std::string romName = "Space Invaders (Japan).gb";
	//std::string romName = "Spot (USA).gb"; // broken at logo, freeze after logo
	//std::string romName = "Tasmania Story (USA).gb";
	//std::string romName = "Tennis (World).gb";
	//std::string romName = "Tesserae (Europe) (En,Fr,De,Es,It).gb";
	std::string romName = "Tetris (World) (Rev A).gb";
	//std::string romName = "Trump Boy (Japan).gb";
	//std::string romName = "Volley Fire (Japan).gb"; // freeze at main menu
	//std::string romName = "World Bowling (USA).gb";
	//std::string romName = "Yakuman (Japan) (Rev A).gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\01-special.gb"; // DAA
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\02-interrupts.gb"; // missing
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\03-op sp,hl.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\04-op r,imm.gb"; // CE DE
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\05-op rp.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\06-ld r,r.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\07-jr,jp,call,ret,rst.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\08-misc instrs.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\09-op r,r.gb"; // ADC A,r, SBC A,r
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\10-bit ops.gb";
	//std::string romName = "gb-test-roms-master\\cpu_instrs\\individual\\11-op a,(hl).gb"; // ADC A,(HL), SBC A,(HL), DAA
	//std::string romName = "gb-test-roms-master\\instr_timing\\instr_timing.gb";
	//std::string romName = "gb-test-roms-master\\interrupt_time\\interrupt_time.gb";
	//std::string romName = "gb-test-roms-master\\mem_timing\\individual\\01-read_timing.gb";
	//std::string romName = "gb-test-roms-master\\mem_timing\\individual\\02-write_timing.gb";
	//std::string romName = "gb-test-roms-master\\mem_timing\\individual\\03-modify_timing.gb";
	//std::string romName = "gb-test-roms-master\\mem_timing-2\\rom_singles\\01-read_timing.gb";
	//std::string romName = "gb-test-roms-master\\mem_timing-2\\rom_singles\\02-write_timing.gb";
	//std::string romName = "gb-test-roms-master\\mem_timing-2\\rom_singles\\03-modify_timing.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\\rom_singles\\1-lcd_sync.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\\rom_singles\\2-causes.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\\rom_singles\\3-non_causes.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\\rom_singles\\4-scanline_timing.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\\rom_singles\\5-timing_bug.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\\rom_singles\\6-timing_no_bug.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\\rom_singles\\7-timing_effect.gb";
	//std::string romName = "gb-test-roms-master\\oam_bug\\rom_singles\\8-instr_effect.gb";
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