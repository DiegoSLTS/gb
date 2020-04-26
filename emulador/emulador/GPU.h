#pragma once

#include "IAddressable.h"
#include "IState.h"

#include "DMA.h"

#include <fstream>

class MMU;
class InterruptServiceRoutine;
class GameWindow;
class Logger;

typedef union {
	u8 v;
	struct {
		bool bgOn : 1;			// Bit 0 - BG / Window Display / Priority(0 = Off, 1 = On)
		bool spritesOn : 1;		// Bit 1 - OBJ(Sprite) Display Enable(0 = Off, 1 = On)
		u8 spritesSize : 1;		// Bit 2 - OBJ(Sprite) Size(0 = 8x8, 1 = 8x16)
		u8 bgMap : 1;			// Bit 3 - BG Tile Map Display Select(0 = 9800 - 9BFF, 1 = 9C00 - 9FFF)
		u8 bgWinData : 1;		// Bit 4 - BG & Window Tile Data Select(0 = 8800 - 97FF, 1 = 8000 - 8FFF)
		bool winOn : 1;			// Bit 5 - Window Display Enable(0 = Off, 1 = On)
		u8 winMap : 1;			// Bit 6 - Window Tile Map Display Select(0 = 9800 - 9BFF, 1 = 9C00 - 9FFF)
		bool displayOn : 1;		// Bit 7 - LCD Display Enable(0 = Off, 1 = On)
	};
} LCDC_t;

enum class GPUMode : u8 {
	HBlank = 0,
	VBlank = 1,
	OAMAccess = 2,
	VRAMAccess = 3
};

typedef union {
	u8 v;
	struct {
		GPUMode mode : 2;					// Bit 1 - 0 - Mode Flag(Mode 0 - 3, see below) (Read Only)
		bool lyc : 1;						// Bit 2 - Coincidence Flag(0:LYC<>LY, 1 : LYC = LY) (Read Only)
		bool hBlankInterruptEnabled : 1;	// Bit 3 - Mode 0 H - Blank Interrupt(1 = Enable) (Read / Write)
		bool vBlankInterruptEnabled : 1;	// Bit 4 - Mode 1 V - Blank Interrupt(1 = Enable) (Read / Write)
		bool oamInterruptEnabled : 1;		// Bit 5 - Mode 2 OAM Interrupt(1 = Enable) (Read / Write)
		bool lycInterruptEnable : 1;		// Bit 6 - LYC = LY Coincidence Interrupt(1 = Enable) (Read / Write)
		u8 unused : 1;
	};
} LCDStat_t;

typedef union {
	u8 v;
	struct {
		u8 paletteIndex : 3;	// Bit 0 - 2 Background Palette number(BGP0 - 7)
		u8 bank : 1;			// Bit 3     Tile VRAM Bank number(0 = Bank 0, 1 = Bank 1)
		u8 unused : 1;			// Bit 4     Not used
		bool flipX : 1;			// Bit 5     Horizontal Flip(0 = Normal, 1 = Mirror horizontally)
		bool flipY : 1;			// Bit 6     Vertical Flip(0 = Normal, 1 = Mirror vertically)
		bool hasPriority : 1;	// Bit 7     BG - to - OAM Priority(0 = Use OAM priority bit, 1 = BG Priority)
	};
} BGAttributes;

typedef union {
	u8 v;
	struct {
		u8 paletteIndex : 3;	// Bit 0 - 2 Palette number  **CGB Mode Only**     (OBP0 - 7)
		u8 bank : 1;			// Bit 3     Tile VRAM - Bank * *CGB Mode Only**     (0 = Bank 0, 1 = Bank 1)
		u8 palette : 1;			// Bit 4     Palette number  **Non CGB Mode Only** (0 = OBP0, 1 = OBP1)
		bool flipX : 1;			// Bit 5     Horizontal Flip(0 = Normal, 1 = Mirror horizontally)
		bool flipY : 1;			// Bit 6     Vertical Flip(0 = Normal, 1 = Mirror vertically)
		bool behindBG : 1;		// Bit 7     OBJ - to - BG Priority(0 = OBJ Above BG, 1 = OBJ Behind BG color 1 - 3) (Used for both BG and Window.BG color 0 is always behind OBJ)
	};
} OBJAttributes;

typedef union {
	u8 v;
	struct {
		u8 colorIndex : 2;		// Bits 0 - 1 colorIndex [0,3]
		u8 paletteIndex : 3;	// Bits 2 - 4 paletteIndex [0,7] for CGB, [0,1] for sprites in GB, 0 for BG in GB
		bool isBG : 1;			// Bit 5      bg/win or sprite, 1 for BG, 0 for sprites
		u8 unused : 1;
		bool bgPriority : 1;	// Bit 7      BG priority (CGB only, used when rendering sprites)
	};
} PixelInfo;

typedef union {
	u32 v;
	struct {
		u8 r;
		u8 g;
		u8 b;
		u8 a;
	};
} ABGR;

class GPU : public IAddressable, public IState {
public:
	const static u8 LCDWidth = 160;
	const static u8 LCDHeight = 144;

    GPU(MMU& mmu, InterruptServiceRoutine& interruptService);
    virtual ~GPU();

	PixelInfo screen[LCDWidth*LCDHeight] = { 0 };

	// returns true if a frame was drawn
	bool Step(u8 cycles, bool isDoubleSpeedEnabled);

	virtual u8 Read(u16 address) override;
	virtual void Write(u8 value, u16 address) override;

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;

	bool isCGB = false;

    // Gets the 32 bit color in ABGR format from the pixel information
	ABGR GetABGRAt(u32 pixelIndex);
	ABGR GetABGR(PixelInfo pixelInfo);

    GameWindow* gameWindow = nullptr;

	u8 ReadVRAM(u16 address, u8 bank);

    bool log = false;

	LCDC_t& GetLCDCRef();
	u8* GetOAMPtr();
	u8* GetBGPPtr();
	u8* GetOBPPtr();

    DMA dma;

private:
	MMU& mmu;
	InterruptServiceRoutine& interruptService;
	
	GPUMode mode = GPUMode::OAMAccess;
	u16 modeCycles = 0;

    u8 spritesInLineCount = 0;
    u8 spritesInLine[11] = { 0xFF }; // limit is 10, but adding one more to simplify the sorting algorithm

    void SortSprite(u8 spriteIndex);
	
	void SetMode(GPUMode newMode);
	void SetCurrentLine(u8 newLine);
	
	// returns new line number
	u8 OnLineFinished();

    void DrawLine(u8 line);
	void DrawBackground(u8 line, u8 pixelCount);
	void DrawWindow(u8 line);
	void DrawSprites(u8 line);

	u8 videoRAM0[0x2000] = { 0 };
	u8 oam[0xA0] = { 0 };

	LCDC_t LCDC = { 0 };			// 0xFF40
	LCDStat_t LCDStat = { 0x80 };	// 0xFF41
	u8 SCY = 0;						// 0xFF42
	u8 SCX = 0;						// 0xFF43
	u8 LY = 0;						// 0xFF44
	u8 LYC = 0;						// 0xFF45
	u8 BGP = 0;						// 0xFF47
	u8 OBP0 = 0;					// 0xFF48
	u8 OBP1 = 0;					// 0xFF49
	u8 WY = 0;						// 0xFF4A
	u8 WX = 0;						// 0xFF4B
    
	// CGB

	u8 BGPI = 0;					// 0xFF68
	u8 OBPI = 0;					// 0xFF6A

	u8 videoRAM1[0x2000] = { 0 };	// Used for CGB only
	u8 VRAMBank = 0;				// Used for CGB only

	u8 BGPMemory[64] = { 0 };		// through 0xFF69 based on BGPI
	u8 OBPMemory[64] = { 0 };		// through 0xFF6B based on OBPI

    u8 FF4C = 0;					// CGB non-CGB mode (value = 0x04)
};
