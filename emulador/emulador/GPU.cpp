#include "GPU.h"
#include "MMU.h"
#include "GameWindow.h"

GPU::GPU(MMU& mmu) : mmu(mmu), dma(mmu) {}
GPU::~GPU() {}

u8 GPU::Read(u16 address) {
	if (address >= 0x8000 && address < 0xA000) {
		if (VRAMBank == 0)
			return videoRAM0[address - 0x8000];
		else
			return videoRAM1[address - 0x8000];
	}

	switch (address) {
	case 0xFF40:
		return LCDC;
	case 0xFF41:
		return LCDStat | 0b10000000;
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
		if (VRAMBank == 0)
			videoRAM0[address - 0x8000] = value;
		else
			videoRAM1[address - 0x8000] = value;
	}

	switch (address) {
    case 0xFF40: {
        bool wasOn = IsOn();
        LCDC = value;
        bool isOn = IsOn();
        if (isOn && !wasOn)
            SetCurrentLine(0);
        break;
    }
	case 0xFF41:
		LCDStat = (value & 0xF8) | (LCDStat & 0x07); break;
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
    stream.read((char*)&videoRAM0, 0x2000);
    stream.read((char*)&videoRAM1, 0x2000);
    stream.read((char*)&VRAMBank, 1);
    stream.read((char*)&BGPI, 1);
    stream.read((char*)&OBPI, 1);
    stream.read((char*)&BGPMemory, 64);
    stream.read((char*)&OBPMemory, 64);
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
    stream.write((const char*)&videoRAM0, 0x2000);
    stream.write((const char*)&videoRAM1, 0x2000);
    stream.write((const char*)&VRAMBank, 1);
    stream.write((const char*)&BGPI, 1); // TODO save BGPD too?
    stream.write((const char*)&OBPI, 1);
    stream.write((const char*)&BGPMemory, 64);
    stream.write((const char*)&OBPMemory, 64);
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
			SetMode(GPUMode::HBlank);
			modeCycles -= 172;
            dma.StepHDMA();
		}
		break;
	}
	case GPUMode::HBlank: {
        if (modeCycles >= 204) {
            u8 currentLine = LY;
            if (currentLine < 144 && IsOn())
                DrawLine(currentLine);

			u8 newLine = OnLineFinished();
			
			SetMode(newLine > 143 ? GPUMode::VBlank : GPUMode::OAMAccess);
			modeCycles -= 204;
			if (newLine == 144 && IsOn())
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

	if ((mode == GPUMode::OAMAccess && (LCDStat & 0x20)) || (mode == GPUMode::VBlank && (LCDStat & 0x10)) || (mode == GPUMode::HBlank && (LCDStat & 0x08)))
		mmu.SetInterruptFlag(1);

	switch (mode) {
	case GPUMode::HBlank:
		LCDStat &= ~1;
		LCDStat &= ~2;
		break;
	case GPUMode::OAMAccess:
		LCDStat &= ~1;
		LCDStat |= 2;
		break;
	case GPUMode::VBlank:
		LCDStat |= 1;
		LCDStat &= ~2;
		break;
	case GPUMode::VRAMAccess:
		LCDStat |= 1;
		LCDStat |= 2;
		break;
	}
}

bool GPU::IsOn() {
	return (LCDC & LCDCMask::LCDOn) > 0;
}

void GPU::SetCurrentLine(u8 newLine) {
	LY = newLine;

	if (LYC == newLine)
		LCDStat |= 4;
	else
		LCDStat &= ~4;

	if (LCDStat & 0x40 && LYC == newLine)
		mmu.SetInterruptFlag(1);
}

u8 GPU::OnLineFinished() {
	u8 line = LY;
	if (line == 153)
		line = 0;
	else
		line++;
	SetCurrentLine(line);
	return line;
}

void GPU::DrawLine(u8 line) {
    if (isCGB || (LCDC & LCDCMask::BGOn))
        DrawBackground(line);
    if ((LCDC & LCDCMask::Win) && (isCGB || (LCDC & LCDCMask::BGOn)))
        DrawWindow(line);
    if (LCDC & LCDCMask::OBJOn)
        DrawSprites(line);

    if (gameWindow != nullptr)
        gameWindow->DrawLine(line);
}

void GPU::DrawBackground(u8 line) {
    u16 tileMapAddress = (LCDC & LCDCMask::BGCodeArea) > 0 ? 0x9C00 : 0x9800;
    u16 tileSetAddress = (LCDC & LCDCMask::BGCharacterArea) > 0 ? 0x8000 : 0x8800;

    u8 tileY = ((SCY + line) / 8) % 32;
    u8 firstTileX = SCX / 8;
    u8 xOffset = SCX % 8;

    for (u8 tileX = 0; tileX < 21; tileX++) {
        u8 tileOffset = videoRAM0[tileMapAddress + tileY * 32 + (firstTileX + tileX) % 32 - 0x8000];
        u8 cgbTileAttributes = videoRAM1[tileMapAddress + tileY * 32 + (firstTileX + tileX) % 32 - 0x8000];

        u8 bgPaletteIndex = cgbTileAttributes & 0x07;
        u8 tileVRAMBank = ((cgbTileAttributes >> 3) & 0x01);
        bool flipX = ((cgbTileAttributes & 0x20) != 0);
        bool flipY = ((cgbTileAttributes & 0x40) != 0);
        bool hasPriority = ((cgbTileAttributes & 0x80) != 0);

        u8 tileLine = flipY ? 7 - line : line;

        u16 tileAddress = tileSetAddress == 0x8000 ? tileSetAddress + tileOffset * 16 : 0x9000 + ((s8)tileOffset) * 16;

        u8 lowByte = 0;
        u8 highByte = 0;

        if (tileVRAMBank == 0) {
            lowByte = videoRAM0[tileAddress + ((SCY + tileLine) % 8) * 2 - 0x8000];
            highByte = videoRAM0[tileAddress + ((SCY + tileLine) % 8) * 2 + 1 - 0x8000];
        } else {
            lowByte = videoRAM1[tileAddress + ((SCY + tileLine) % 8) * 2 - 0x8000];
            highByte = videoRAM1[tileAddress + ((SCY + tileLine) % 8) * 2 + 1 - 0x8000];
        }

        u16 screenPosBase = line * LCDWidth + tileX * 8 - xOffset;

        for (s8 bit = 7; bit >= 0; bit--) {
            u8 pixel = flipX ? 7 - bit : bit;

            s16 screenPos = screenPosBase + (7 - bit);
            if (screenPos >= line * LCDWidth && screenPos < (line + 1) * LCDWidth) {
                u8 lowBit = (lowByte >> pixel) & 0x01;
                u8 highBit = (highByte >> pixel) & 0x01;
                u8 pixelInfo = lowBit | (highBit << 1);
                pixelInfo |= (bgPaletteIndex << 2);
                pixelInfo |= 0x20;
                
                if (hasPriority && ((LCDC & 0x80) > 0))
                    pixelInfo |= 0x80;

                screen[screenPos] = pixelInfo;
            }
        }
    }
}

void GPU::DrawWindow(u8 line) {
    if (line < WY)
        return;

    u16 tileMapAddress = (LCDC & LCDCMask::WinCodeArea) > 0 ? 0x9C00 : 0x9800;
    u16 tileSetAddress = (LCDC & LCDCMask::BGCharacterArea) > 0 ? 0x8000 : 0x8800;

    u8 tileY = ((line - WY) / 8) % 32;
    u8 firstTileX = 0;

    u8 tilesToDraw = (LCDWidth - WX) / 8 + 1;

    for (int tileX = 0; tileX < tilesToDraw; tileX++) {
        u8 tileOffset = videoRAM0[tileMapAddress + tileY * 32 + (firstTileX + tileX) % 32 - 0x8000];
        u8 cgbTileAttributes = videoRAM1[tileMapAddress + tileY * 32 + (firstTileX + tileX) % 32 - 0x8000];

        u8 bgPaletteIndex = cgbTileAttributes & 0x07;
        u8 tileVRAMBank = ((cgbTileAttributes >> 3) & 0x01);
        bool flipX = ((cgbTileAttributes & 0x20) != 0);
        bool flipY = ((cgbTileAttributes & 0x40) != 0);
        bool hasPriority = ((cgbTileAttributes & 0x80) != 0);

        u8 tileLine = flipY ? 7 - line : line;

        u16 tileAddress = tileSetAddress == 0x8000 ? tileSetAddress + tileOffset * 16 : 0x9000 + ((s8)tileOffset) * 16;

        u8 lowByte = 0;
        u8 highByte = 0;

        if (tileVRAMBank == 0) {
            lowByte = videoRAM0[tileAddress + ((tileLine - WY) % 8) * 2 - 0x8000];
            highByte = videoRAM0[tileAddress + ((tileLine - WY) % 8) * 2 + 1 - 0x8000];
        }
        else {
            lowByte = videoRAM1[tileAddress + ((tileLine - WY) % 8) * 2 - 0x8000];
            highByte = videoRAM1[tileAddress + ((tileLine - WY) % 8) * 2 + 1 - 0x8000];
        }

        u16 screenPosBase = line * LCDWidth + tileX * 8 + (WX - 7);

        for (s8 bit = 7; bit >= 0; bit--) {
            u8 pixel = flipX ? 7 - bit : bit;

            s16 screenPos = screenPosBase + (7 - bit);
            if (screenPos >= line * LCDWidth && screenPos < (line + 1) * LCDWidth) {
                u8 lowBit = (lowByte >> pixel) & 0x01;
                u8 highBit = (highByte >> pixel) & 0x01;
                u8 pixelInfo = lowBit | (highBit << 1);
                pixelInfo |= (bgPaletteIndex << 2);
                pixelInfo |= 0x20;

                if (hasPriority && ((LCDC & 0x80) > 0))
                    pixelInfo |= 0x80;

                screen[screenPos] = pixelInfo;
            }
        }
    }
}

void GPU::DrawSprites(u8 line) {
    u8 spriteHeight = (LCDC & LCDCMask::ObjSize) > 0 ? 16 : 8;
    u8 spritesDrawn = 0;
    bool LCDC0Set = ((LCDC & LCDCMask::BGOn) > 0);

    // TODO if GB, sort by x

    for (u8 i = 0; i < 40; i++) {
        u8 spriteIndex = i * 4;

        s16 spriteY = (s16)mmu.Read(0xFE00 + spriteIndex) - 16;

        if ((line >= spriteY) && (line < spriteY + spriteHeight)) {
            s16 spriteX = mmu.Read(0xFE00 + spriteIndex + 1) - 8;
            u8 tileIndex = mmu.Read(0xFE00 + spriteIndex + 2);
            u8 attributes = mmu.Read(0xFE00 + spriteIndex + 3);

            bool hasPriority = (attributes & 0b10000000) == 0;

            bool flipY = (attributes & 0b01000000) > 0;
            bool flipX = (attributes & 0b00100000) > 0;
            u8 gbPalette = (attributes & 0b00010000) > 0 ? OBP1 : OBP0;
            u8 tileVRAMBank = (attributes & 0b00001000) > 0 ? 1 : 0;
            u8 cgbPalette = attributes & 0x7;

            u16 tileAddress = 0x8000 + tileIndex * 16;

            u8 spriteLine = flipY ? spriteHeight - 1 - (line - spriteY) : line - spriteY;

            u8 lowByte = 0;
            u8 highByte = 0;

            if (tileVRAMBank == 0) {
                lowByte = videoRAM0[tileAddress + spriteLine * 2 - 0x8000];
                highByte = videoRAM0[tileAddress + spriteLine * 2 + 1 - 0x8000];
            }
            else {
                lowByte = videoRAM1[tileAddress + spriteLine * 2 - 0x8000];
                highByte = videoRAM1[tileAddress + spriteLine * 2 + 1 - 0x8000];
            }

            s16 screenPosBase = line * LCDWidth + spriteX;

            for (s8 bit = 7; bit >= 0; bit--) {
                u8 pixel = flipX ? 7 - bit : bit;
                s16 screenPos = line * LCDWidth + spriteX + (7 - bit);

                // if LCDC is set and the BG had priority here, ignore the pixel color
                if (LCDC0Set && ((screen[screenPos] & 0x80) > 0))
                    continue;

                if (screenPos >= line * LCDWidth && screenPos < (line + 1) * LCDWidth) {
                    u8 lowBit = (lowByte >> pixel) & 0x01;
                    u8 highBit = (highByte >> pixel) & 0x01;
                    u8 index = lowBit | (highBit << 1);

                    // TODO also ignore is the color is already from a sprite
                    if (index > 0 && (!LCDC0Set || hasPriority || (screen[screenPos] & 0x03) == 0)) {
                        u8 pixelInfo = index;
                        pixelInfo |= (cgbPalette << 2);
                        // pixelInfo bit 5 already 0

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

u32 GPU::GetABGR(u8 pixelInfo) {
    u8 colorIndex = pixelInfo & 0x03;
    u8 paletteIndex = (pixelInfo >> 2) & 0x07;
    bool isBG = (pixelInfo & 0x20) > 0;

    u16 gpuColor = 0;

    u8 gbPalette = 0;
    if (isBG)
        gbPalette = BGP;
    else
        gbPalette = paletteIndex == 0 ? OBP0 : OBP1;

    u8 r = 0;
    u8 g = 0;
    u8 b = 0;
    u8 a = 0xFF;

    if (isCGB) {
        if (FF4C == 0x04) // GBC Non-CGB mode
            colorIndex = ((gbPalette >> (colorIndex << 1)) & 0x03);

        if (isBG)
            gpuColor = (BGPMemory[paletteIndex * 8 + colorIndex * 2 + 1] << 8) | BGPMemory[paletteIndex * 8 + colorIndex * 2];
        else
            gpuColor = (OBPMemory[paletteIndex * 8 + colorIndex * 2 + 1] << 8) | OBPMemory[paletteIndex * 8 + colorIndex * 2];

        // TODO check for a better conversion, first to consider this: https://gbdev.gg8.se/wiki/articles/Video_Display#RGB_Translation_by_CGBs
        // but also because 0x1F * 8 is not 255 (the actual max value for each component)... 255/31 should be used, but it's not round
        // When that's changed, change the TileViewers and SpritesViewers code
        // https://byuu.net/video/color-emulation <- GameBoy Color section
        r = (gpuColor & 0x1F) * 8;
        g = ((gpuColor >> 5) & 0x1F) * 8;
        b = ((gpuColor >> 10) & 0x1F) * 8;
    } else
        // turn gpuScreen value [0,3] into an 8 bit value [255,0], 85 == 255/3
        // const static u8 sfmlColors[] = { 0xFF, 0xAA , 0x55, 0x00 }; // 255 - index * 85 
        r = g = b = 255 - ((gbPalette >> (colorIndex << 1)) & 0x03) * 85;

    return (a << 24) | (b << 16) | (g << 8) | r;
}

u8 GPU::ReadVRAM0(u16 address) {
	return videoRAM0[address - 0x8000];
}

u8 GPU::ReadVRAM1(u16 address) {
	return videoRAM1[address - 0x8000];
}
