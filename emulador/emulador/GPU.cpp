#include "GPU.h"
#include "MMU.h"
#include "InterruptServiceRoutine.h"
#include "GameWindow.h"
#include "Logger.h"

#include <iostream>

GPU::GPU(MMU& mmu, InterruptServiceRoutine& interruptService) : mmu(mmu), dma(mmu, oam), interruptService(interruptService) {}
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
		return LY;
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
        if (VRAMBank == 0) {
            videoRAM0[address - 0x8000] = value;
            if (log) Logger::instance->log("VRAM0[" + Logger::u16ToHex(address) + "] = " + Logger::u8ToHex(value) + "\n");
        } else {
            videoRAM1[address - 0x8000] = value;
            if (log) Logger::instance->log("VRAM1[" + Logger::u16ToHex(address) + "] = " + Logger::u8ToHex(value) + "\n");
        }
		return;
	}

	if (address >= 0xFE00 && address < 0xFEA0) {
		oam[address - 0xFE00] = value;
        if (log) Logger::instance->log("oam[" + Logger::u16ToHex(address) + "] = " + Logger::u8ToHex(value) + "\n");
		return;
	}

	switch (address) {
    case 0xFF40: {
        bool wasOn = LCDC.displayOn;
        LCDC.v = value;
        if (LCDC.displayOn != wasOn) {
            if (LCDC.displayOn) {
                SetCurrentLine(0);
                mode = GPUMode::OAMAccess;
                modeCycles = 0;
                if (log) Logger::instance->log("LCD on\n");
            } else {
                memset(screen, 0x80, LCDWidth * LCDHeight); // 0x80 = PixelInfo bytes with "blank" bit set
                if (log) Logger::instance->log("LCD off\n");
            }
        }
        if (log) Logger::instance->log("LCDC = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(LCDC.v) + "\n");
		break;
	}
	case 0xFF41:
		LCDStat.v = value;
		LCDStat.mode = mode;
        if (log) Logger::instance->log("LCDStat = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(LCDStat.v) + "\n");
		break;
	case 0xFF42:
		SCY = value;
        if (log) Logger::instance->log("SCY = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(SCY) + "\n"); 
        break;
	case 0xFF43:
		SCX = value;
        if (log) Logger::instance->log("SCX = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(SCX) + "\n"); 
        break;
	case 0xFF44:
		LY = 0;
        if (log) Logger::instance->log("LY = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(LY) + "\n"); 
        break;
	case 0xFF45:
		LYC = value;
        if (log) Logger::instance->log("LYC = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(LYC) + "\n"); 
        break;
	case 0xFF47:
		BGP = value;
        if (log) Logger::instance->log("BGP = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(BGP) + "\n");
		break;
	case 0xFF48:
		OBP0 = value;
        if (log) Logger::instance->log("OBP0 = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(OBP0) + "\n"); 
        break;
	case 0xFF49:
		OBP1 = value;
        if (log) Logger::instance->log("OBP1 = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(OBP1) + "\n"); 
        break;
	case 0xFF4A:
		WY = value;
        if (log) Logger::instance->log("WY = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(WY) + "\n");
        break;
	case 0xFF4B:
		WX = value;
        if (log) Logger::instance->log("WX = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(WX) + "\n"); 
        break;
	case 0xFF68:
		BGPI = value & 0xBF;
        if (log) Logger::instance->log("BGPI = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(BGPI) + "\n");
        break;
	case 0xFF69:
		if (log) Logger::instance->log("BG palette " + Logger::u8ToHex(BGPI & 0x3F) + ": " + Logger::u8ToHex(value) + "\n");
		BGPMemory[BGPI & 0x3F] = value;
		if (BGPI & 0x80) {
			BGPI++;
			BGPI &= 0xBF;
		}
		break;
	case 0xFF6A:
		OBPI = value & 0xBF;
        if (log) Logger::instance->log("OBPI = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(OBPI) + "\n");
        break;
	case 0xFF6B:
        if (log) Logger::instance->log("OBJ palette " + Logger::u8ToHex(OBPI & 0x3F) + ": " + Logger::u8ToHex(value) + "\n");
		OBPMemory[OBPI & 0x3F] = value;
		if (OBPI & 0x80) {
			OBPI++;
			OBPI &= 0xBF;
		}
		break;
	case 0xFF4F:
		VRAMBank = (value & 0x01);
        if (log) Logger::instance->log("VRAMBank = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(VRAMBank) + "\n");
        break;
	case 0xFF46: // dma
	case 0xFF51: // hdma - CGB
	case 0xFF52:
	case 0xFF53:
	case 0xFF54:
	case 0xFF55:
		dma.Write(value, address);
        if (mode == GPUMode::HBlank)
            dma.StepHDMA();
        break;
	case 0xFF4C:
		FF4C = value;
        if (log) Logger::instance->log("FF4C = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(FF4C) + "\n");
        break;
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
	stream.write((const char*)&BGPI, 1);
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
            u8 spriteHeight = LCDC.spritesSize == 0 ? 8 : 16;
            u8 spritesDrawn = 0;
            spritesInLineCount = 0;

            for (u8 i = 0; i < 40; i++) {
                u8 spriteIndex = i * 4;
                s16 spriteY = (s16)oam[spriteIndex] - 16;

                if ((LY >= spriteY) && (LY < spriteY + spriteHeight)) {
                    SortSprite(spriteIndex);

                    if (spritesInLineCount == 10)
                        break;
                }
            }
			
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
            if (newLine == 144)
                frameDrawn = true;
		}
		break;
	}
	case GPUMode::VBlank: {
        if (LY == 153)
            SetCurrentLine(0);

		if (modeCycles >= 456) {
			modeCycles -= 456;

            // Line 153 is considered line 0 by the LCD driver, but then there's also a line 0
            // once VBlank finishes and the new frame begins.
            // So, VBlank runs from line 144 through 152, then a line 0, then that line and the frame
            // finish and it's line 0 again and the new frame starts.
            // https://forums.nesdev.com/viewtopic.php?t=13727#p162336
            // This fixes Aladdin and lots of games using HiColor mode
            
            // For every line that finishes during VBlank, I just increment it's value. OnLineFinished
            // takes care of wrapping after line 152, so it'll be 0 again.
            // If the current line is 0, since this is VBlank, it means the fake line 0 is finishing,
            // and that means the frame finished. The line is set to 0 again to trigger the LYC interrupt
            // if necessary
            if (LY == 0)
                SetMode(GPUMode::OAMAccess);
            else
                OnLineFinished();
		}
		break;
	}
	}

	return frameDrawn;
}

void GPU::SortSprite(u8 spriteIndex) {
    spritesInLine[spritesInLineCount] = spriteIndex;
    if (!isCGB && spritesInLineCount > 0) {
        s16 spriteX = oam[spriteIndex + 1];
        for (s8 i = spritesInLineCount - 1; i >= 0; i--) {
            u8 otherSpriteIndex = spritesInLine[i];
            s16 otherSpriteX = oam[otherSpriteIndex + 1];
            if (spriteX < otherSpriteX) {
                spritesInLine[i] = spriteIndex;
                spritesInLine[i + 1] = otherSpriteIndex;
            } else
                break;
        }
    }

    if (spritesInLineCount < 10)
        spritesInLineCount++;
}

void GPU::SetMode(GPUMode newMode) {
	mode = newMode;

    if (mode == GPUMode::VBlank && LCDC.displayOn) {
        interruptService.SetInterruptFlag(InterruptFlag::VBlank);

        // Apparently, when VBlank mode starts the LCD driver still fires the OAM-STAT interrupt if enabled
        // as if OAM mode has started
        // https://forums.nesdev.com/viewtopic.php?t=13727
        if (LCDStat.oamInterruptEnabled)
            interruptService.SetInterruptFlag(InterruptFlag::LCDStat);
    }

	if (LCDC.displayOn && ((mode == GPUMode::OAMAccess && LCDStat.oamInterruptEnabled) ||
		(mode == GPUMode::VBlank && LCDStat.vBlankInterruptEnabled) ||
		(mode == GPUMode::HBlank && LCDStat.hBlankInterruptEnabled)))
		interruptService.SetInterruptFlag(InterruptFlag::LCDStat);

	LCDStat.mode = mode;

    if (log) Logger::instance->log("mode: " + std::to_string((unsigned int)mode) + "\n");
}

void GPU::SetCurrentLine(u8 newLine) {
	LY = newLine;
	LCDStat.lyc = (LYC == newLine);

	if (LCDStat.lycInterruptEnable && LCDStat.lyc && LCDC.displayOn)
		interruptService.SetInterruptFlag(InterruptFlag::LCDStat);

	if (log) Logger::instance->log("line: " + std::to_string((unsigned int)LY) + "\n");
}

u8 GPU::OnLineFinished() {
	u8 line = LY + 1;
	SetCurrentLine(line);
	return line;
}

void GPU::DrawLine(u8 line) {
    bool drawBG = isCGB || LCDC.bgOn;
    bool drawWIN = LCDC.winOn && (isCGB || LCDC.bgOn) && line >= WY && WX >= 0 && WX <= 166;
    bool drawSprites = LCDC.spritesOn;

    if (drawBG)
        DrawBackground(line, drawWIN ? WX : LCDWidth + 8);
    else
        memset(screen + line*LCDWidth, 0x80, LCDWidth); // 0x80 = PixelInfo bytes with "blank" bit set

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
    u8 tileLine = (SCY + line) % 8;
	u8 firstTileX = SCX / 8;
	u8 xOffset = SCX % 8;

	u8 tilesToDraw = pixelCount / 8;

	u16 addressBase = tileMapAddress + tileY * 32 - 0x8000;
	for (u8 tileX = 0; tileX < tilesToDraw; tileX++) {
		u16 address = addressBase + (firstTileX + tileX) % 32;
		u8 tileOffset = videoRAM0[address];
		BGAttributes cgbAttributes = { videoRAM1[address] };

        u16 tileAddress = LCDC.bgWinData == 0 ? 0x9000 + ((s8)tileOffset) * 16 : 0x8000 + tileOffset * 16;
		tileAddress += (cgbAttributes.flipY ? 7 - tileLine : tileLine) * 2;

		u8 lowByte = ReadVRAM(tileAddress, cgbAttributes.bank);
		u8 highByte = ReadVRAM(tileAddress + 1, cgbAttributes.bank);

		u16 screenPosBase = line * LCDWidth + tileX * 8 - xOffset;

		for (s8 bit = 7; bit >= 0; bit--) {
			u8 pixel = cgbAttributes.flipX ? 7 - bit : bit;

			s16 screenPos = screenPosBase + (7 - bit);
			if ((screenPos >= line * LCDWidth) && (screenPos < (line + 1) * LCDWidth)) {
				u8 lowBit = (lowByte >> pixel) & 0x01;
				u8 highBit = (highByte >> pixel) & 0x01;
				PixelInfo pixelInfo = { 0 };
				pixelInfo.colorIndex = lowBit | (highBit << 1);
				pixelInfo.paletteIndex = cgbAttributes.paletteIndex;
				pixelInfo.isBG = true;

				if (cgbAttributes.hasPriority && LCDC.bgOn) // LCDC.bgOn on GBC is bgPriority
					pixelInfo.bgPriority = true;

				screen[screenPos] = pixelInfo;
			}
		}
	}
}

void GPU::DrawWindow(u8 line) {
	u16 tileMapAddress = LCDC.winMap == 0 ? 0x9800 : 0x9C00;

	u8 tileY = ((line - WY) / 8) % 32;
    u8 tileLine = (line - WY) % 8;
	u8 firstTileX = 0;

	u8 tilesToDraw = (LCDWidth - (WX - 7)) / 8 + 1;

	u16 addressBase = tileMapAddress + tileY * 32 - 0x8000;
	for (int tileX = 0; tileX < tilesToDraw; tileX++) {
		u16 address = addressBase + (firstTileX + tileX) % 32;
		u8 tileOffset = videoRAM0[address];
		BGAttributes cgbAttributes = { videoRAM1[address] };

		u16 tileAddress = LCDC.bgWinData == 0 ? 0x9000 + ((s8)tileOffset) * 16 : 0x8000 + tileOffset * 16;
        tileAddress += (cgbAttributes.flipY ? 7 - tileLine : tileLine) * 2;

		u8 lowByte = ReadVRAM(tileAddress, cgbAttributes.bank);
		u8 highByte = ReadVRAM(tileAddress + 1, cgbAttributes.bank);

		u16 screenPosBase = line * LCDWidth + tileX * 8 + (WX - 7);

		for (s8 bit = 7; bit >= 0; bit--) {
			u8 pixel = cgbAttributes.flipX ? 7 - bit : bit;

			s16 screenPos = screenPosBase + (7 - bit);
			if ((screenPos >= line * LCDWidth) && (screenPos < (line + 1) * LCDWidth)) {
				u8 lowBit = (lowByte >> pixel) & 0x01;
				u8 highBit = (highByte >> pixel) & 0x01;
				PixelInfo pixelInfo = { 0 };
				pixelInfo.colorIndex = lowBit | (highBit << 1);
				pixelInfo.paletteIndex = cgbAttributes.paletteIndex;
				pixelInfo.isBG = true;

				// TODO confirm if this applies to the window too
				if (cgbAttributes.hasPriority && LCDC.bgOn)
					pixelInfo.bgPriority = true;

				screen[screenPos] = pixelInfo;
			}
		}
	}
}

void GPU::DrawSprites(u8 line) {
	u8 spriteHeight = LCDC.spritesSize == 0 ? 8 : 16;
	u8 spritesDrawn = 0;

	for (u8 i = 0; i < spritesInLineCount; i++) {
		u8 spriteIndex = spritesInLine[i];
		s16 spriteY = (s16)oam[spriteIndex] - 16;

		if ((line >= spriteY) && (line < spriteY + spriteHeight)) {
			s16 spriteX = oam[spriteIndex + 1] - 8;
			u8 tileIndex = oam[spriteIndex + 2];
            if (spriteHeight == 16)
                tileIndex &= 0xFE;
            u16 tileAddress = 0x8000 + tileIndex * 16;

			OBJAttributes attributes = { oam[spriteIndex + 3] };
            u8 spriteLine = attributes.flipY ? spriteHeight - 1 - (line - spriteY) : line - spriteY;
			u8 lowByte = ReadVRAM(tileAddress + spriteLine * 2, attributes.bank);
			u8 highByte = ReadVRAM(tileAddress + spriteLine * 2 + 1, attributes.bank);

			s16 screenPosBase = line * LCDWidth + spriteX;

			for (s8 bit = 7; bit >= 0; bit--) {
				u8 pixel = attributes.flipX ? 7 - bit : bit;
				s16 screenPos = line * LCDWidth + spriteX + (7 - bit);

				// if LCDC is set and the BG has priority, ignore the pixel color
				if (LCDC.bgOn && screen[screenPos].bgPriority && screen[screenPos].colorIndex > 0)
					continue;

				if (screenPos >= line * LCDWidth && screenPos < (line + 1) * LCDWidth) {
					u8 lowBit = (lowByte >> pixel) & 0x01;
					u8 highBit = (highByte >> pixel) & 0x01;
					PixelInfo pixelInfo = { 0 };
					pixelInfo.colorIndex = lowBit | (highBit << 1);

					if (pixelInfo.colorIndex > 0 && screen[screenPos].isBG && (!LCDC.bgOn || !attributes.behindBG || (screen[screenPos].colorIndex == 0))) {
						pixelInfo.paletteIndex = isCGB && FF4C != 0x04 ? attributes.paletteIndex : attributes.palette;
						// pixelInfo.isBG already false

						screen[screenPos] = pixelInfo;
					}
				}
			}
		}
	}
}

ABGR GPU::GetABGRAt(u32 pixelIndex) {
	return GetABGR(screen[pixelIndex]);
}

ABGR GPU::GetABGR(PixelInfo pixelInfo) {
    if (pixelInfo.blank)
        return { 0xFFFFFFFF };

	// turn gb color [0,3] into an 8 bit color [255,0], 85 == 255/3
	const static u8 greyColors[] = { 0xFF, 0xAA , 0x55, 0x00 }; // 255 - index * 85 

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
	} else
		abgr.r = abgr.g = abgr.b = greyColors[(gbPalette >> (pixelInfo.colorIndex << 1)) & 0x03];

	return abgr;
}

u8 GPU::ReadVRAM(u16 address, u8 bank) {
	if (bank == 0 || !isCGB || FF4C == 0x04)
		return videoRAM0[address - 0x8000];

	return videoRAM1[address - 0x8000];
}

LCDC_t& GPU::GetLCDCRef() {
	return LCDC;
}

u8* GPU::GetOAMPtr() {
	return oam;
}

u8* GPU::GetBGPPtr() {
	return BGPMemory;
}

u8* GPU::GetOBPPtr() {
	return OBPMemory;
}
