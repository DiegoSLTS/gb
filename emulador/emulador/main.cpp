#include <SFML/Graphics.hpp>
#include <fstream>
#include <ctime>
#include <iostream>
#include <bitset>

#include "Types.h"
#include "BootRom.h"

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

#include "Roms.h"

void printCPUFlags(const CPU& cpu) {
	std::bitset<8> flags(cpu.Read8BitReg(CPU8BitReg::f));
	std::cout << "flags: 0b" << flags << std::endl;
}

bool isLittleEndian() {
	u8 t8[2] = { 0x1, 0x0 };
	u16* t16 = (u16*)t8;
	bool isLittleEndian = t16[0] == t8[0];
	std::cout << "t8[0] = " << (unsigned int)t8[0] << " t16[0] = " << (unsigned int)t16[0] << " - is " << (isLittleEndian ? "little endian" : "big endian") << std::endl;
	return isLittleEndian;
}

void UpdateSFMLScreenArray(std::unique_ptr<sf::Uint8[]>& sfmlScreen, u8 gpuScreen[]) {
    int index = 0;
	for (; index < 160 * 144; index++) {
		// turn gpuScreen value [0,3] into an 8 bit value [255,0], 85 == 255/3
		u8 gpuColor = 255 - gpuScreen[index] * 85;
		sfmlScreen[index * 4] = gpuColor;
		sfmlScreen[index * 4 + 1] = gpuColor;
		sfmlScreen[index * 4 + 2] = gpuColor;
		sfmlScreen[index * 4 + 3] = 0xFF;
	}
}

void SaveState(const CPU& cpu, const GPU& gpu, const MMU& mmu, const InterruptServiceRoutine& interruptService, const DMA& dma) {
	std::ofstream stream("bootState.txt", std::ios::binary);

	cpu.Save(stream);
	mmu.Save(stream);
	gpu.Save(stream);
	interruptService.Save(stream);
    //dma.Save(stream);
	
	stream.close();
}

void LoadState(const CPU& cpu, const GPU& gpu, const MMU& mmu, const InterruptServiceRoutine& interruptService, const DMA& dma) {
	std::ifstream stream("bootState.txt", std::ios::binary);

	cpu.Load(stream);
	mmu.Load(stream);
	gpu.Load(stream);
	interruptService.Load(stream);
    //dma.Load(stream);

	stream.close();
}

int main(int argc, char *argv[]) {
	std::string romPath;
	std::string romDir;
	std::string romName;

	if (argc > 1) {
		//TODO validate path
		romPath = argv[1];
		size_t slashPosition = romPath.find_last_of("/");
		romDir = romPath.substr(0, slashPosition + 1);
		romName = romPath.substr(slashPosition + 1);
		std::cout << "Loading rom from argv[1] = " << romPath << std::endl;
	} else {
		romDir = "D:/Programacion/gb/roms/";
		romName = Games::METROID_II_RETURN_OF_SAMUS;
		romPath = romDir.append(romName);
		std::cout << "Loading hardcoded rom from = " << romPath << std::endl;
	}

	Cartridge cartridge(romPath);
	
	// TODO move parse logic into Cartridge class
	bool parseRom = false;
	if (parseRom) {
		RomParser parser;
		// TODO support more cartridge sizes
		// TODO parser.ParseCartridgeROM(cartridge.mbc->rom.get(), 32 * 1024);
		parser.PrintCodeToFile();
	}
	
	MMU mmu;
	mmu.cartridge = &cartridge;

	CPU cpu(mmu);

	InterruptServiceRoutine interruptService;
	mmu.interruptServiceRoutine = &interruptService;
	cpu.interruptService = &interruptService;

	GPU gpu(mmu);
	mmu.gpu = &gpu;

	Timer timer(mmu);
	mmu.timer = &timer;

	Joypad joypad(mmu);
	mmu.joypad = &joypad;

	DMA dma(mmu);
	mmu.dma = &dma;

	SerialDataTransfer serial;
	mmu.serial = &serial;

	bool skipBios = true;
	if (skipBios)
		LoadState(cpu, gpu, mmu, interruptService, dma);

	Window gameWindow(160, 144, "Game");
	sf::Vector2i p(50, 50);
	gameWindow.renderWindow->setPosition(p);
	
	TileViewer tilesWindow(168, 144, "Tiles", mmu, 0x8000);
	p.x = 385;
	p.y = 50;
	tilesWindow.renderWindow->setPosition(p);

	SpritesViewer spritesWindow(256 + 8, 256 + 16, "Sprites", mmu);
	p.x = 1110;
	p.y = 380;
	spritesWindow.renderWindow->setPosition(p);

	TileMapViewer tileMap0Window(256, 256, "Tile map 0", mmu, 0x9800);
	p.x = 50;
	p.y = 380;
	tileMap0Window.renderWindow->setPosition(p);

	TileMapViewer tileMap1Window(256, 256, "Tile map 1", mmu, 0x9C00);
	p.x = 580;
	p.y = 380;
	tileMap1Window.renderWindow->setPosition(p);

    std::time_t previousFPSTimer = time(NULL);
    std::time_t currentTimer;
    u16 framesCount = 0;
	bool logFPS = false;
	bool updateViewers = false;

	float elapsedTime = 0;
	std::time_t previousFrameTimer = time(NULL);

	while (gameWindow.renderWindow->isOpen()) {
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
			SaveState(cpu, gpu, mmu, interruptService, dma);

		// used only for debugging to break at specific instructions
		if (cpu.pc == 0x3155 /*0x3306*/) {
			int a = 0;
		}

		if (!cpu.isHalted) {
			u8 opCode = cpu.ReadOpCode();
			if (opCode == 0xCB) {
				opCode = cpu.ReadOpCode();
				cpu.CallCBOpCode(opCode);
			} else
				cpu.CallOpCode(opCode);
		} else
			cpu.lastOpCycles = 1;

		u8 lastOpCycles = cpu.lastOpCycles;
		if (gpu.Step(lastOpCycles * 4)) {
			UpdateSFMLScreenArray(gameWindow.screenArray, gpu.screen);
			gameWindow.screenTexture.update(gameWindow.screenArray.get());
			gameWindow.screenSprite.setTexture(gameWindow.screenTexture, true);

			gameWindow.renderWindow->clear();
			gameWindow.renderWindow->draw(gameWindow.screenSprite);
			gameWindow.renderWindow->display();

			if (updateViewers) {
				tilesWindow.Update();
				tileMap0Window.Update();
				tileMap1Window.Update();
				spritesWindow.Update();
			}

			if (logFPS)
				framesCount++;
		}
		
		timer.Step(lastOpCycles * 4);
		dma.Step(lastOpCycles);
		
		cpu.lastOpCycles = 0;

        time(&currentTimer);
        if (logFPS && currentTimer - previousFPSTimer >= 1) {
            std::cout << framesCount << " - ";
            previousFPSTimer = currentTimer;
            framesCount = 0;
        }

		if (currentTimer - previousFrameTimer >= 0.13) {
			previousFrameTimer = currentTimer;
			sf::Event event;
			while (gameWindow.renderWindow->pollEvent(event))
				if (event.type == sf::Event::Closed)
					gameWindow.renderWindow->close();
				else if (event.type == sf::Event::KeyPressed) {
					if (event.key.code == sf::Keyboard::F) {
						logFPS = !logFPS;
						time(&previousFPSTimer);
						framesCount = 0;
					}
					else if (event.key.code == sf::Keyboard::Q) {
						updateViewers = !updateViewers;
					}
				}
		}
	}

	return 0;
}