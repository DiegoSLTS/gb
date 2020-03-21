#pragma once

#include <stdint.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef int8_t s8;
typedef int16_t s16;

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

enum GPUMode {
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