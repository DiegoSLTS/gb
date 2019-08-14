#include "GPU.h"
#include "MMU.h"

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
	case 0xFF40:
		LCDC = value; break;
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
            u8 currentLine = GetCurrentLine();
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
	//TODO update STATE register (LY == LYC, mode, etc)
	//TODO remaining cycles?
	return frameDrawn;
}

void GPU::SetMode(GPUMode newMode) {
	mode = newMode;

	if (mode == GPUMode::VBlank)
		mmu->SetInterruptFlag(0);

	if ((mode == GPUMode::OAMAccess && (LCDStat & 0x20)) || (mode == GPUMode::VBlank && (LCDStat & 0x10)) || (mode == GPUMode::HBlank && (LCDStat & 0x08)))
		mmu->SetInterruptFlag(1);

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
	bool isOn = (LCDC & LCDCMask::LCDOn) > 0;

	// TODO move to Write
	if (isOn && !wasOn)
		SetCurrentLine(0);

	if (!isOn && wasOn && mode != GPUMode::VBlank) {
		//TODO assert? forbiden according to http://gbdev.gg8.se/wiki/articles/LCDC
	}
	wasOn = isOn;
	return isOn;
}

u8 GPU::GetCurrentLine() const {
	//TODO assert newline < 154?
	return LY;
}

void GPU::SetCurrentLine(u8 newLine) {
	//TODO assert newline < 154?
	LY = newLine;

	if (LYC == newLine)
		LCDStat |= 4;
	else
		LCDStat &= ~4;

	if (LCDStat & 0x40 && LYC == newLine)
		mmu->SetInterruptFlag(1);
}

u8 GPU::OnLineFinished() {
	u8 line = GetCurrentLine();
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
	u16 bgDataAddress = (LCDC & LCDCMask::BGCodeArea) > 0 ? 0x9C00 : 0x9800;
	u16 tileDataAddress = (LCDC & LCDCMask::BGCharacterArea) > 0 ? 0x8000 : 0x8800;

	u8 bgPalette = BGP;

	const u8 paletteMask = 0b00000011;

	u8 firstTileY = (SCY + line) / 8;
	u8 firstTileX = SCX / 8;
	u8 xTileOffset = SCX % 8;

	for (int tileX = 0; tileX < 20; tileX++) {
		u8 tileOffset = mmu->Read(bgDataAddress + firstTileY * 32 + firstTileX + tileX);

		u16 tileAddress = tileDataAddress == 0x8000 ? tileDataAddress + tileOffset * 16 : 0x9000 + ((s8)tileOffset) * 16;

		u8 tileDataLow = mmu->Read(tileAddress + ((SCY + line) % 8) * 2);
		u8 tileDataHigh = mmu->Read(tileAddress + ((SCY + line) % 8) * 2 + 1);

		u16 screenPosBase = line * 160 + tileX * 8 - xTileOffset;

		for (s8 pixel = 7; pixel >= 0; pixel--) {
			s16 screenPos = screenPosBase + (7 - pixel);
			if (screenPos >= 0 && screenPos <= LCDWidth * LCDHeight) {
				u8 lowBit = (tileDataLow >> pixel) & 0x01;
				u8 highBit = (pixel > 0 ? tileDataHigh >> (pixel - 1) : tileDataHigh << 1) & 0x02;
				u8 id = lowBit | highBit;

				screen[screenPos] = (bgPalette & (paletteMask << (id << 1))) >> (id << 1);
			}
		}
	}
}

void GPU::DrawWindow(u8 line) {
	if (line < WY)
		return;

	u16 winDataAddress = (LCDC & LCDCMask::WinCodeArea) > 0 ? 0x9C00 : 0x9800;
	u16 tileDataAddress = (LCDC & LCDCMask::BGCharacterArea) > 0 ? 0x8000 : 0x8800;

	u8 bgPalette = BGP;

	const u8 paletteMask = 0b00000011;

	u8 firstTileY = (line - WY) / 8;
	u8 firstTileX = 0;

	u8 tilesToDraw = (160 - WX) / 8 + 1;

	for (int tileX = 0; tileX < tilesToDraw; tileX++) {
		u8 tileOffset = mmu->Read(winDataAddress + firstTileY * 32 + firstTileX + tileX);

		u16 tileAddress = tileDataAddress == 0x8000 ? tileDataAddress + tileOffset * 16 : 0x9000 + ((s8)tileOffset) * 16;

		u8 tileDataLow = mmu->Read(tileAddress + (line % 8) * 2);
		u8 tileDataHigh = mmu->Read(tileAddress + (line % 8) * 2 + 1);

		u16 screenPosBase = line * 160 + tileX * 8 + (WX - 7);

		for (s8 pixel = 7; pixel >= 0; pixel--) {
            s16 screenPos = screenPosBase + (7 - pixel);
			if (screenPos >= 0 && screenPos <= LCDWidth * LCDHeight) {
				u8 lowBit = (tileDataLow >> pixel) & 0x01;
				u8 highBit = (pixel > 0 ? tileDataHigh >> (pixel - 1) : tileDataHigh << 1) & 0x02;
				u8 id = lowBit | highBit;

				screen[screenPos] = (bgPalette & (paletteMask << (id << 1))) >> (id << 1);
			}
		}
	}
}

void GPU::DrawSprites(u8 line) {
	const u8 paletteMask = 0b00000011;

	u8 spriteHeight = LCDC & LCDCMask::ObjSize ? 16 : 8;
	u8 spritesDrawn = 0;
	for (u8 i = 0; i < 40; i++) {
		u8 spriteIndex = i * 4;

        s16 spriteY = (s16)mmu->Read(0xFE00 + spriteIndex) - 16;

		if ((spriteY <= line) && (spriteY + spriteHeight > line)) {
			u8 spriteX = mmu->Read(0xFE00 + spriteIndex + 1) - 8;
			u8 tileIndex = mmu->Read(0xFE00 + spriteIndex + 2);
			u8 attributes = mmu->Read(0xFE00 + spriteIndex + 3);

			bool hasPriority = (attributes & 0b10000000) > 0;
			bool flipY = (attributes & 0b01000000) > 0;
			bool flipX = (attributes & 0b00100000) > 0;
			u16 palette = (attributes & 0b00010000) > 0 ? OBP1 : OBP0;

			u16 tileAddress = 0x8000 + tileIndex * 16;

			u8 tileDataLow = mmu->Read(tileAddress + (line - spriteY) * 2);
			u8 tileDataHigh = mmu->Read(tileAddress + (line - spriteY) * 2 + 1);

			u16 screenPosBase = line * 160 + spriteX;

			for (s8 pixel = 7; pixel >= 0; pixel--) {
                s16 screenPos = screenPosBase + (7 - pixel);
				if (screenPos >= 0 && screenPos <= LCDWidth * LCDHeight) {
					u8 lowBit = (tileDataLow >> pixel) & 0x01;
					u8 highBit = (pixel > 0 ? tileDataHigh >> (pixel - 1) : tileDataHigh << 1) & 0x02;
					u8 id = lowBit | highBit;

					if (id > 0)
						screen[screenPos] = (palette & (paletteMask << (id << 1))) >> (id << 1);
				}
			}
			
			spritesDrawn++;
			if (spritesDrawn == 10)
				return;
		}
	}
}
