#pragma once

#include "IAddressable.h"
#include "IState.h"

#include "DMA.h"

#include <fstream>

class MMU;
class GameWindow;
class Logger;

class GPU : public IAddressable, public IState {
public:
	const static u8 LCDWidth = 160;
	const static u8 LCDHeight = 144;

    GPU(MMU& mmu);
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

    Logger* logger = nullptr;

	LCDC_t& GetLCDCRef();
	u8* GetOAMPtr();

private:
	MMU& mmu;
    DMA dma;
	
	GPUMode mode = GPUMode::OAMAccess;
	u16 modeCycles = 0;
	
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
