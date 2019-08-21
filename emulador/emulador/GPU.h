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
	const static u8 LCDWidth = 160;
	const static u8 LCDHeight = 144;
	const static u8 paletteMask = 0b00000011;

    GPU(MMU& mmu);
    virtual ~GPU();

	MMU& mmu;
	
	GPUMode mode = GPUMode::OAMAccess;
	u16 modeCycles = 0;

	bool wasOn = false;

	u8 screen[LCDWidth*LCDHeight] = { 0 };
	
	// returns true if frame was drawn
	bool Step(u8 cycles);

	void SetMode(GPUMode newMode);

	bool IsOn();

	u8 GetCurrentLine() const;

	void SetCurrentLine(u8 newLine);

	// returns new line number
	u8 OnLineFinished();

    void DrawLine(u8 line);
	void DrawBackground(u8 line);
	void DrawWindow(u8 line);
	void DrawSprites(u8 line);

	u8 LCDC = 0; //0xFF40
	u8 LCDStat = 0x80; //0xFF41
	u8 SCY = 0; //0xFF42
	u8 SCX = 0; //0xFF43
	u8 LY = 0; //0xFF44
	u8 LYC = 0; //0xFF45
	u8 BGP = 0; //0xFF47
	u8 OBP0 = 0; //0xFF48
	u8 OBP1 = 0; //0xFF49
	u8 WY = 0; //0xFF4A
	u8 WX = 0; //0xFF4B

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
};