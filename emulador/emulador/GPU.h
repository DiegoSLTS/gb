#pragma once

#include "IAddressable.h"
#include "IState.h"

class MMU;

enum GPUMode {
	HBlank = 0,
	VBlank,
	OAMAccess,
	VRAMAccess
};

enum LCDCMask {
	BGOn = 0x1, // 0 = Off, 1 = On (CGB, always on)
	OBJOn = 0x2, // 0 = Off, 1 = On
	ObjSize = 0x4, // 0 = 8x8, 1 = 8x16
	BGCodeArea = 0x8, // 0 = 0x9800-0x9BFF, 1 = 0x9C00-0x9FFF
	BGCharacterArea = 0x10, // 0 = 0x8800-0x97FF, 1 = 0x8000-0x8FFF
	Win = 0x20, // 0 = Off, 1 = On
	WinCodeArea = 0x40, // 0 = 0x9800-0x9BFF, 1 = 0x9C00-0x9FFF
	LCDOn = 0x80 // 0 = Off, 1 = On
};

class GPU : public IAddressable, public IState {
public:
	const static uint8_t LCDWidth = 160;
	const static uint8_t LCDHeight = 144;

	MMU* mmu = nullptr;
	
	GPUMode mode = GPUMode::OAMAccess;
	uint16_t modeCycles = 0;

	bool wasOn = false;

	uint8_t screen[LCDWidth*LCDHeight] = { 0 };
	
	// returns true if frame was drawn
	bool Step(uint8_t cycles);

	void SetMode(GPUMode newMode);

	bool IsOn();

	uint8_t GetCurrentLine() const;

	void SetCurrentLine(uint8_t newLine);

	// returns new line number
	uint8_t OnLineFinished();

	void DrawFrame();
	void DrawBackground(uint8_t line);
	void DrawWindow(uint8_t line);
	void DrawSprites(uint8_t line);

	uint8_t LCDC = 0; //0xFF40
	uint8_t LCDStat = 0x80; //0xFF41
	uint8_t SCY = 0; //0xFF42
	uint8_t SCX = 0; //0xFF43
	uint8_t LY = 0; //0xFF44
	uint8_t LYC = 0; //0xFF45
	uint8_t BGP = 0; //0xFF47
	uint8_t OBP0 = 0; //0xFF48
	uint8_t OBP1 = 0; //0xFF49
	uint8_t WY = 0; //0xFF4A
	uint8_t WX = 0; //0xFF4B

	virtual uint8_t Read(uint16_t address) override;
	virtual void Write(uint8_t value, uint16_t address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
};