#include "GPU.h"
#include "MMU.h"

GPU::GPU(MMU& mmu) : mmu(mmu) {}
GPU::~GPU() {}

u8 GPU::Read(u16 address) {
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
	}
	return 0xFF;
}

void GPU::Write(u8 value, u16 address) {
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
		BGP = value; break;
	case 0xFF48:
		OBP0 = value; break;
	case 0xFF49:
		OBP1 = value; break;
	case 0xFF4A:
		WY = value; break;
	case 0xFF4B:
		WX = value; break;
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
}

bool GPU::Step(u8 cycles) {
	modeCycles += cycles;
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
    if (LCDC & LCDCMask::BGOn)
        DrawBackground(line);
    if (LCDC & LCDCMask::Win)
        DrawWindow(line);
    if (LCDC & LCDCMask::OBJOn)
        DrawSprites(line);
}

void GPU::DrawBackground(u8 line) {
	u16 tileMapAddress = (LCDC & LCDCMask::BGCodeArea) > 0 ? 0x9C00 : 0x9800;
	u16 tileSetAddress = (LCDC & LCDCMask::BGCharacterArea) > 0 ? 0x8000 : 0x8800;
	
	u8 tileY = ((SCY + line) / 8) % 32;
	u8 firstTileX = SCX / 8;
	u8 xOffset = SCX % 8;

	for (u8 tileX = 0; tileX < 21; tileX++) {
		u8 tileOffset = mmu.Read(tileMapAddress + tileY * 32 + (firstTileX + tileX) % 32);
		u16 tileAddress = tileSetAddress == 0x8000 ? tileSetAddress + tileOffset * 16 : 0x9000 + ((s8)tileOffset) * 16;

		u8 lowByte = mmu.Read(tileAddress + ((SCY + line) % 8) * 2);
		u8 highByte = mmu.Read(tileAddress + ((SCY + line) % 8) * 2 + 1);

		u16 screenPosBase = line * LCDWidth + tileX * 8 - xOffset;

		for (s8 pixel = 7; pixel >= 0; pixel--) {
			s16 screenPos = screenPosBase + (7 - pixel);
			if (screenPos >= line * LCDWidth && screenPos < (line + 1) * LCDWidth) {
				u8 lowBit = (lowByte >> pixel) & 0x01;
				u8 highBit = (highByte >> pixel) & 0x01;
				u8 index = lowBit | (highBit << 1);

				screen[screenPos] = (BGP >> (index << 1)) & 0x03;
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
		u8 tileOffset = mmu.Read(tileMapAddress + tileY * 32 + (firstTileX + tileX) % 32);
		u16 tileAddress = tileSetAddress == 0x8000 ? tileSetAddress + tileOffset * 16 : 0x9000 + ((s8)tileOffset) * 16;

		u8 lowByte = mmu.Read(tileAddress + ((line - WY) % 8) * 2);
		u8 highByte = mmu.Read(tileAddress + ((line - WY) % 8) * 2 + 1);

		u16 screenPosBase = line * LCDWidth + tileX * 8 + (WX - 7);

		for (s8 pixel = 7; pixel >= 0; pixel--) {
            s16 screenPos = screenPosBase + (7 - pixel);
			if (screenPos >= line * LCDWidth && screenPos < (line + 1) * LCDWidth) {
				u8 lowBit = (lowByte >> pixel) & 0x01;
				u8 highBit = (highByte >> pixel) & 0x01;
				u8 index = lowBit | (highBit << 1);

				screen[screenPos] = (BGP >> (index << 1)) & 0x03;
			}
		}
	}
}

void GPU::DrawSprites(u8 line) {
	u8 spriteHeight = (LCDC & LCDCMask::ObjSize) > 0 ? 16 : 8;
	u8 spritesDrawn = 0;

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
			u16 palette = (attributes & 0b00010000) > 0 ? OBP1 : OBP0;

			u16 tileAddress = 0x8000 + tileIndex * 16;

            u8 spriteLine = flipY ? spriteHeight - 1 - (line - spriteY) : line - spriteY;

			u8 lowByte = mmu.Read(tileAddress + spriteLine * 2);
			u8 highByte = mmu.Read(tileAddress + spriteLine * 2 + 1);

			s16 screenPosBase = line * LCDWidth + spriteX;

			for (s8 bit = 7; bit >= 0; bit--) {
                u8 pixel = flipX ? 7 - bit : bit;
                s16 screenPos = line * LCDWidth + spriteX + (7 - bit);
				if (screenPos >= line * LCDWidth && screenPos < (line + 1) * LCDWidth) {
					u8 lowBit = (lowByte >> pixel) & 0x01;
					u8 highBit = (highByte >> pixel) & 0x01;
					u8 index = lowBit | (highBit << 1);

					if (index > 0 && (hasPriority || screen[screenPos] == 0))
						screen[screenPos] = (palette >> (index << 1)) & 0x03;
				}
			} 
			
			spritesDrawn++;
			if (spritesDrawn == 10)
				return;
		}
	}
}
