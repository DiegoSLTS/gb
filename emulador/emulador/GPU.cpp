#include "GPU.h"
#include "MMU.h"
#include "GameWindow.h"
#include "Logger.h"

#include <iostream>

GPU::GPU(MMU& mmu) : mmu(mmu), dma(mmu, oam) {}
GPU::~GPU() {}

u8 GPU::Read(u16 address) {
	if (address >= 0x8000 && address < 0xA000)
		return ReadVRAM(address, VRAMBank);

	if (address >= 0xFE00 && address < 0xFEA0)
		return oam[address - 0xFE00];

	switch (address) {
	case 0xFF40:
		return LCDC.v;
	case 0xFF41:
		return LCDStat.v | 0x80;
	case 0xFF42:
		return SCY;
	case 0xFF43:
		return SCX;
	case 0xFF44:
		return LCDC.displayOn ? LY : 0x00;
	case 0xFF45:
		return LYC;
	case 0xFF47:
		return BGP;
	case 0xFF48:
		return OBP0;
	case 0xFF49:
		return OBP1;
	case 0xFF4A:
		return WY;
	case 0xFF4B:
		return WX;
	case 0xFF68:
		return BGPI;
	case 0xFF69:
		return BGPMemory[BGPI & 0x3F];
	case 0xFF6A:
		return OBPI;
	case 0xFF6B:
		return OBPMemory[OBPI & 0x3F];
	case 0xFF4F:
		return VRAMBank | 0xFE;
	case 0xFF46: // dma
	case 0xFF51: // hdma - CGB
	case 0xFF52:
	case 0xFF53:
	case 0xFF54:
	case 0xFF55:
		return dma.Read(address);
	case 0xFF4C:
		return FF4C;
	}
	return 0xFF;
}

void GPU::Write(u8 value, u16 address) {
	if (address >= 0x8000 && address < 0xA000) {
		if (VRAMBank == 0)
			videoRAM0[address - 0x8000] = value;
		else
			videoRAM1[address - 0x8000] = value;
		return;
	}

	if (address >= 0xFE00 && address < 0xFEA0) {
		oam[address - 0xFE00] = value;
		return;
	}

	switch (address) {
	case 0xFF40: {
		bool wasOn = LCDC.displayOn;
		LCDC.v = value;
		bool isOn = LCDC.displayOn;
		if (isOn && !wasOn) {
			SetCurrentLine(0);
			mode = GPUMode::OAMAccess;
			modeCycles = 0;
		}

		break;
	}
	case 0xFF41:
		LCDStat.v = value;
		LCDStat.mode = mode;
		break;
	case 0xFF42:
		SCY = value; break;
	case 0xFF43:
		SCX = value; break;
	case 0xFF44:
		LY = 0; break;
	case 0xFF45:
		LYC = value; break;
	case 0xFF47:
		BGP = value;
		break;
	case 0xFF48:
		OBP0 = value; break;
	case 0xFF49:
		OBP1 = value; break;
	case 0xFF4A:
		WY = value; break;
	case 0xFF4B:
		WX = value; break;
	case 0xFF68:
		BGPI = value & 0xBF; break;
	case 0xFF69:
		if (logger != nullptr) logger->log("-- palette " + std::to_string((unsigned int)(BGPI & 0x3F)) + ": " + std::to_string((unsigned int)value) + "\n");
		BGPMemory[BGPI & 0x3F] = value;
		if (BGPI & 0x80) {
			BGPI++;
			BGPI &= 0xBF;
		}
		break;
	case 0xFF6A:
		OBPI = value & 0xBF; break;
	case 0xFF6B:
		OBPMemory[OBPI & 0x3F] = value;
		if (OBPI & 0x80) {
			OBPI++;
			OBPI &= 0xBF;
		}
		break;
	case 0xFF4F:
		VRAMBank = (value & 0x01); break;
	case 0xFF46: // dma
	case 0xFF51: // hdma - CGB
	case 0xFF52:
	case 0xFF53:
	case 0xFF54:
	case 0xFF55:
		dma.Write(value, address); break;
	case 0xFF4C:
		FF4C = value; break;
	}
}

void GPU::Load(std::ifstream& stream) const {
	stream.read((char*)&mode, 1);
	stream.read((char*)&modeCycles, 2);
	stream.read((char*)&LCDC, 1);
	stream.read((char*)&LCDStat, 1);
	stream.read((char*)&SCY, 1);
	stream.read((char*)&SCX, 1);
	stream.read((char*)&LY, 1);
	stream.read((char*)&LYC, 1);
	stream.read((char*)&BGP, 1);
	stream.read((char*)&OBP0, 1);
	stream.read((char*)&OBP1, 1);
	stream.read((char*)&WY, 1);
	stream.read((char*)&WX, 1);
	stream.read((char*)videoRAM0, 0x2000);
	stream.read((char*)videoRAM1, 0x2000);
	stream.read((char*)oam, 160);
	stream.read((char*)&VRAMBank, 1);
	stream.read((char*)&BGPI, 1);
	stream.read((char*)&OBPI, 1);
	stream.read((char*)BGPMemory, 64);
	stream.read((char*)OBPMemory, 64);
	stream.read((char*)&FF4C, 1);
	//dma.Load(stream);
}

void GPU::Save(std::ofstream& stream) const {
	stream.write((const char*)&mode, 1);
	stream.write((const char*)&modeCycles, 2);
	stream.write((const char*)&LCDC, 1);
	stream.write((const char*)&LCDStat, 1);
	stream.write((const char*)&SCY, 1);
	stream.write((const char*)&SCX, 1);
	stream.write((const char*)&LY, 1);
	stream.write((const char*)&LYC, 1);
	stream.write((const char*)&BGP, 1);
	stream.write((const char*)&OBP0, 1);
	stream.write((const char*)&OBP1, 1);
	stream.write((const char*)&WY, 1);
	stream.write((const char*)&WX, 1);
	stream.write((const char*)videoRAM0, 0x2000);
	stream.write((const char*)videoRAM1, 0x2000);
	stream.write((const char*)oam, 160);
	stream.write((const char*)&VRAMBank, 1);
	stream.write((const char*)&BGPI, 1); // TODO save BGPD too?
	stream.write((const char*)&OBPI, 1);
	stream.write((const char*)BGPMemory, 64);
	stream.write((const char*)OBPMemory, 64);
	stream.write((const char*)&FF4C, 1);
	//dma.Save(stream);
}

bool GPU::Step(u8 cycles, bool isDoubleSpeedEnabled) {
	dma.Step(cycles);

	modeCycles += (isDoubleSpeedEnabled ? cycles / 2 : cycles);
	bool frameDrawn = false;
	switch (mode) {
	case GPUMode::OAMAccess: {
		if (modeCycles >= 80) {
			SetMode(GPUMode::VRAMAccess);
			modeCycles -= 80;
		}
		break;
	}
	case GPUMode::VRAMAccess: {
		if (modeCycles >= 172) {
			u8 currentLine = LY;
			if (currentLine < 144 && LCDC.displayOn)
				DrawLine(currentLine);

			SetMode(GPUMode::HBlank);
			modeCycles -= 172;
			dma.StepHDMA();
		}
		break;
	}
	case GPUMode::HBlank: {
		if (modeCycles >= 204) {
			u8 newLine = OnLineFinished();

			SetMode(newLine > 143 ? GPUMode::VBlank : GPUMode::OAMAccess);
			modeCycles -= 204;
			if (newLine == 144 && LCDC.displayOn)
				frameDrawn = true;
		}
		break;
	}
	case GPUMode::VBlank: {
		if (modeCycles >= 456) {
			modeCycles -= 456;
			u8 newLine = OnLineFinished();

			if (newLine == 0) // frame finished
				SetMode(GPUMode::OAMAccess);
		}
		break;
	}
	}

	return frameDrawn;
}

void GPU::SetMode(GPUMode newMode) {
	mode = newMode;

	if (mode == GPUMode::VBlank)
		mmu.SetInterruptFlag(0);

	if ((mode == GPUMode::OAMAccess && LCDStat.oamInterruptEnabled) ||
		(mode == GPUMode::VBlank && LCDStat.vBlankInterruptEnabled) ||
		(mode == GPUMode::HBlank && LCDStat.hBlankInterruptEnabled))
		mmu.SetInterruptFlag(1);

	LCDStat.mode = mode;

	if (logger != nullptr) logger->log("-- mode: " + std::to_string((unsigned int)mode) + "\n");
}

void GPU::SetCurrentLine(u8 newLine) {
	LY = newLine;
	LCDStat.lyc = LYC == newLine;

	if (LCDStat.lycInterruptEnable && LCDStat.lyc)
		mmu.SetInterruptFlag(1);

	if (logger != nullptr) logger->log("-- line: " + std::to_string((unsigned int)LY) + "\n");
}

u8 GPU::OnLineFinished() {
	u8 line = LY == 153 ? 0 : LY + 1;
	SetCurrentLine(line);
	return line;
}

void GPU::DrawLine(u8 line) {
	/*// This fixes CGB's Aladdin HiColor images in some way, so the problem is that the palettes for lines 0 and 1 are being copied to soon? or skipped?
	if (line <= 1)
		return;
	line -= 2;*/

	if (logger != nullptr) logger->log("-- drawing line: " + std::to_string((unsigned int)line) + "\n");

	bool drawBG = isCGB || LCDC.bgOn;
	bool drawWIN = LCDC.winOn && (isCGB || LCDC.bgOn) && line >= WY;
	bool drawSprites = LCDC.spritesOn;

	if (drawBG)
		DrawBackground(line, drawWIN ? WX - 7 : LCDWidth + 8);
	if (drawWIN)
		DrawWindow(line);
	if (drawSprites)
		DrawSprites(line);

	if (gameWindow != nullptr)
		gameWindow->DrawLine(line);
}

void GPU::DrawBackground(u8 line, u8 pixelCount) {
	u16 tileMapAddress = LCDC.bgMap == 0 ? 0x9800 : 0x9C00;

	u8 tileY = ((SCY + line) / 8) % 32;
	u8 firstTileX = SCX / 8;
	u8 xOffset = SCX % 8;

	u8 tilesToDraw = pixelCount / 8;

	u16 addressBase = tileMapAddress + tileY * 32 - 0x8000;
	for (u8 tileX = 0; tileX < tilesToDraw; tileX++) {
		u16 address = addressBase + (firstTileX + tileX) % 32;
		u8 tileOffset = videoRAM0[address];
		BGAttributes cgbAttributes = { videoRAM1[address] };

		u8 tileLine = cgbAttributes.flipY ? 7 - line : line;

		u16 tileAddress = LCDC.bgWinData == 0 ? 0x9000 + ((s8)tileOffset) * 16 : 0x8000 + tileOffset * 16;
		tileAddress += ((SCY + tileLine) % 8) * 2;
		u8 lowByte = ReadVRAM(tileAddress, cgbAttributes.bank);
		u8 highByte = ReadVRAM(tileAddress + 1, cgbAttributes.bank);

		u16 screenPosBase = line * LCDWidth + tileX * 8 - xOffset;

		for (s8 bit = 7; bit >= 0; bit--) {
			u8 pixel = cgbAttributes.flipX ? 7 - bit : bit;

			s16 screenPos = screenPosBase + (7 - bit);
			if (screenPos >= line * LCDWidth && screenPos < (line + 1) * LCDWidth) {
				u8 lowBit = (lowByte >> pixel) & 0x01;
				u8 highBit = (highByte >> pixel) & 0x01;
				PixelInfo pixelInfo = { 0 };
				pixelInfo.colorIndex = lowBit | (highBit << 1);
				pixelInfo.paletteIndex = cgbAttributes.paletteIndex;
				pixelInfo.isBG = true;

				if (cgbAttributes.hasPriority && LCDC.bgOn == 1) // LCDC.bgOn on GBC is bgPriority
					pixelInfo.bgPriority = true;

				screen[screenPos] = pixelInfo;
			}
		}
	}
}

void GPU::DrawWindow(u8 line) {
	u16 tileMapAddress = LCDC.winMap == 0 ? 0x9800 : 0x9C00;

	u8 tileY = ((line - WY) / 8) % 32;
	u8 firstTileX = 0;

	u8 tilesToDraw = (LCDWidth - (WX - 7)) / 8 + 1;

	u16 addressBase = tileMapAddress + tileY * 32 - 0x8000;
	for (int tileX = 0; tileX < tilesToDraw; tileX++) {
		u16 address = addressBase + (firstTileX + tileX) % 32;
		u8 tileOffset = videoRAM0[address];
		BGAttributes cgbAttributes = { videoRAM1[address] };

		u8 tileLine = cgbAttributes.flipY ? 7 - line : line;

		u16 tileAddress = LCDC.bgWinData == 0 ? 0x9000 + ((s8)tileOffset) * 16 : 0x8000 + tileOffset * 16;
		tileAddress += ((tileLine - WY) % 8) * 2;
		u8 lowByte = ReadVRAM(tileAddress, cgbAttributes.bank);
		u8 highByte = ReadVRAM(tileAddress + 1, cgbAttributes.bank);

		u16 screenPosBase = line * LCDWidth + tileX * 8 + (WX - 7);

		for (s8 bit = 7; bit >= 0; bit--) {
			u8 pixel = cgbAttributes.flipX ? 7 - bit : bit;

			s16 screenPos = screenPosBase + (7 - bit);
			if (screenPos >= line * LCDWidth && screenPos < (line + 1) * LCDWidth) {
				u8 lowBit = (lowByte >> pixel) & 0x01;
				u8 highBit = (highByte >> pixel) & 0x01;
				PixelInfo pixelInfo = { 0 };
				pixelInfo.colorIndex = lowBit | (highBit << 1);
				pixelInfo.paletteIndex = cgbAttributes.paletteIndex;
				pixelInfo.isBG = true;

				// TODO confirm if this applies to the window too
				if (cgbAttributes.hasPriority && LCDC.bgOn == 1)
					pixelInfo.bgPriority = true;

				screen[screenPos] = pixelInfo;
			}
		}
	}
}

void GPU::DrawSprites(u8 line) {
	u8 spriteHeight = LCDC.spritesSize == 0 ? 8 : 16;
	u8 spritesDrawn = 0;

	// TODO if GB, sort by x

	for (u8 i = 0; i < 40; i++) {
		u8 spriteIndex = i * 4;

		s16 spriteY = (s16)oam[spriteIndex] - 16;

		if ((line >= spriteY) && (line < spriteY + spriteHeight)) {
			s16 spriteX = oam[spriteIndex + 1] - 8;
			u8 tileIndex = oam[spriteIndex + 2];
			OBJAttributes attributes = { oam[spriteIndex + 3] };

			if (attributes.flipX)
				int a = 0;
			u16 tileAddress = 0x8000 + tileIndex * 16;

			u8 spriteLine = attributes.flipY ? spriteHeight - 1 - (line - spriteY) : line - spriteY;

			u8 lowByte = ReadVRAM(tileAddress + spriteLine * 2, attributes.bank);
			u8 highByte = ReadVRAM(tileAddress + spriteLine * 2 + 1, attributes.bank);

			s16 screenPosBase = line * LCDWidth + spriteX;

			for (s8 bit = 7; bit >= 0; bit--) {
				u8 pixel = attributes.flipX ? 7 - bit : bit;
				s16 screenPos = line * LCDWidth + spriteX + (7 - bit);

				// if LCDC is set and the BG has priority, ignore the pixel color
				if (LCDC.bgOn && screen[screenPos].bgPriority)
					continue;

				if (screenPos >= line * LCDWidth && screenPos < (line + 1) * LCDWidth) {
					u8 lowBit = (lowByte >> pixel) & 0x01;
					u8 highBit = (highByte >> pixel) & 0x01;
					PixelInfo pixelInfo = { 0 };
					pixelInfo.colorIndex = lowBit | (highBit << 1);

					if (pixelInfo.colorIndex > 0 && screen[screenPos].isBG && (!LCDC.bgOn || !attributes.behindBG || (screen[screenPos].colorIndex) == 0)) {
						pixelInfo.paletteIndex = isCGB ? attributes.paletteIndex : attributes.palette;
						// pixelInfo.isBG already false

						screen[screenPos] = pixelInfo;
					}
				}
			}

			spritesDrawn++;
			if (spritesDrawn == 10)
				return;
		}
	}
}

ABGR GPU::GetABGRAt(u32 pixelIndex) {
	return GetABGR(screen[pixelIndex]);
}

ABGR GPU::GetABGR(PixelInfo pixelInfo) {
	u16 gpuColor = 0;

	u8 gbPalette = 0;
	if (pixelInfo.isBG)
		gbPalette = BGP;
	else
		gbPalette = pixelInfo.paletteIndex == 0 ? OBP0 : OBP1;

	ABGR abgr = { 0xFF000000 };

	if (isCGB) {
		if (FF4C == 0x04) // GBC Non-CGB mode
			pixelInfo.colorIndex = ((gbPalette >> (pixelInfo.colorIndex << 1)) & 0x03);

		u8 paletteOffset = pixelInfo.paletteIndex * 8 + pixelInfo.colorIndex * 2;
		if (pixelInfo.isBG)
			gpuColor = (BGPMemory[paletteOffset + 1] << 8) | BGPMemory[paletteOffset];
		else
			gpuColor = (OBPMemory[paletteOffset + 1] << 8) | OBPMemory[paletteOffset];

		// TODO check for a better conversion, first to consider this: https://gbdev.gg8.se/wiki/articles/Video_Display#RGB_Translation_by_CGBs
		// but also because 0x1F * 8 is not 255 (the actual max value for each component)... 255/31 should be used, but it's not round
		// When that's changed, change the TileViewers and SpritesViewers code
		// https://byuu.net/video/color-emulation <- GameBoy Color section
		abgr.r = (gpuColor & 0x1F) * 8;
		abgr.g = ((gpuColor >> 5) & 0x1F) * 8;
		abgr.b = ((gpuColor >> 10) & 0x1F) * 8;
	}
	else
		// turn gpuScreen value [0,3] into an 8 bit value [255,0], 85 == 255/3
		// const static u8 sfmlColors[] = { 0xFF, 0xAA , 0x55, 0x00 }; // 255 - index * 85 
		abgr.r = abgr.g = abgr.b = 255 - ((gbPalette >> (pixelInfo.colorIndex << 1)) & 0x03) * 85;

	return abgr;
}

u8 GPU::ReadVRAM(u16 address, u8 bank) {
	if (bank == 0)
		return videoRAM0[address - 0x8000];

	return videoRAM1[address - 0x8000];
}

LCDC_t& GPU::GetLCDCRef() {
	return LCDC;
}

u8* GPU::GetOAMPtr() {
	return oam;
}
