#include "GPU.h"
#include "MMU.h"

uint8_t GPU::Read(uint16_t address) {
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

void GPU::Write(uint8_t value, uint16_t address) {
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

bool GPU::Step(uint8_t cycles) {
	if (!IsOn())
	{
		// TODO?
		return false;
	}

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
			uint8_t newLine = OnLineFinished();
			
			SetMode(newLine > 143 ? GPUMode::VBlank : GPUMode::OAMAccess);
			modeCycles -= 204;
			if (newLine == 144) {
				DrawFrame();
				frameDrawn = true;
			}
		}
		break;
	}
	case GPUMode::VBlank: {
		if (modeCycles >= 456) {
			modeCycles -= 456;
			uint8_t newLine = OnLineFinished();

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

uint8_t GPU::GetCurrentLine() const {
	//TODO assert newline < 154?
	return LY;
}

void GPU::SetCurrentLine(uint8_t newLine) {
	//TODO assert newline < 154?
	LY = newLine;

	if (LYC == newLine)
		LCDStat |= 4;
	else
		LCDStat &= ~4;

	if (LCDStat & 0x40 && LYC == newLine)
		mmu->SetInterruptFlag(1);
}

uint8_t GPU::OnLineFinished() {
	uint8_t line = GetCurrentLine();
	if (line == 153)
		line = 0;
	else
		line++;
	SetCurrentLine(line);
	return line;
}

void GPU::DrawFrame() {
	for (int line = 0; line < 144; line++) {
		//if (LCDC & LCDCMask::BGOn)
			DrawBackground(line);
		if (LCDC & LCDCMask::Win)
			DrawWindow(line);
		//if (LCDC & LCDCMask::OBJOn)
			DrawSprites(line);
	}
}

void GPU::DrawBackground(uint8_t line) {
	uint16_t bgDataAddress = (LCDC & LCDCMask::BGCodeArea) > 0 ? 0x9C00 : 0x9800;
	uint16_t tileDataAddress = (LCDC & LCDCMask::BGCharacterArea) > 0 ? 0x8000 : 0x8800;

	uint8_t bgPalette = BGP;

	const uint8_t paletteMask = 0b00000011;

	if (SCY + line > 144) {
		int a = 0;
	}

	uint8_t firstTileY = (SCY + line) / 8;
	uint8_t firstTileX = SCX / 8;
	uint8_t xTileOffset = SCX % 8;

	for (int tileX = 0; tileX < 20; tileX++) {
		uint8_t tileOffset = mmu->Read(bgDataAddress + firstTileY * 32 + firstTileX + tileX);

		uint16_t tileAddress = tileDataAddress == 0x8000 ? tileDataAddress + tileOffset * 16 : 0x9000 + ((int8_t)tileOffset) * 16;

		uint8_t tileDataLow = mmu->Read(tileAddress + ((SCY + line) % 8) * 2);
		uint8_t tileDataHigh = mmu->Read(tileAddress + ((SCY + line) % 8) * 2 + 1);

		uint16_t screenPosBase = line * 160 + tileX * 8 - xTileOffset;

		for (int8_t pixel = 7; pixel >= 0; pixel--) {
			int16_t screenPos = screenPosBase + (7 - pixel);
			if (screenPos >= 0 && screenPos <= LCDWidth * LCDHeight) {
				uint8_t lowBit = (tileDataLow >> pixel) & 0x01;
				uint8_t highBit = (pixel > 0 ? tileDataHigh >> (pixel - 1) : tileDataHigh << 1) & 0x02;
				uint8_t id = lowBit | highBit;

				screen[screenPos] = (bgPalette & (paletteMask << (id << 1))) >> (id << 1);
			}
		}
	}
}

void GPU::DrawWindow(uint8_t line) {
	if (line < WY)
		return;

	uint16_t winDataAddress = (LCDC & LCDCMask::WinCodeArea) > 0 ? 0x9C00 : 0x9800;
	uint16_t tileDataAddress = (LCDC & LCDCMask::BGCharacterArea) > 0 ? 0x8000 : 0x8800;

	uint8_t bgPalette = BGP;

	const uint8_t paletteMask = 0b00000011;

	uint8_t firstTileY = (line - WY) / 8;
	uint8_t firstTileX = 0;

	uint8_t tilesToDraw = (160 - WX) / 8 + 1;

	for (int tileX = 0; tileX < tilesToDraw; tileX++) {
		uint8_t tileOffset = mmu->Read(winDataAddress + firstTileY * 32 + firstTileX + tileX);

		uint16_t tileAddress = tileDataAddress == 0x8000 ? tileDataAddress + tileOffset * 16 : 0x9000 + ((int8_t)tileOffset) * 16;

		uint8_t tileDataLow = mmu->Read(tileAddress + (line % 8) * 2);
		uint8_t tileDataHigh = mmu->Read(tileAddress + (line % 8) * 2 + 1);

		uint16_t screenPosBase = line * 160 + tileX * 8 + (WX - 7);

		for (int8_t pixel = 7; pixel >= 0; pixel--) {
			int16_t screenPos = screenPosBase + (7 - pixel);
			if (screenPos >= 0 && screenPos <= LCDWidth * LCDHeight) {
				uint8_t lowBit = (tileDataLow >> pixel) & 0x01;
				uint8_t highBit = (pixel > 0 ? tileDataHigh >> (pixel - 1) : tileDataHigh << 1) & 0x02;
				uint8_t id = lowBit | highBit;

				screen[screenPos] = (bgPalette & (paletteMask << (id << 1))) >> (id << 1);
			}
		}
	}
}

void GPU::DrawSprites(uint8_t line) {
	const uint8_t paletteMask = 0b00000011;

	uint8_t spriteHeight = LCDC & LCDCMask::ObjSize ? 16 : 8;
	uint8_t spritesDrawn = 0;
	for (uint8_t i = 0; i < 40; i++) {
		uint8_t spriteIndex = i * 4;

		int16_t spriteY = (int16_t)mmu->Read(0xFE00 + spriteIndex) - 16;

		if ((spriteY <= line) && (spriteY + spriteHeight > line)) {
			uint8_t spriteX = mmu->Read(0xFE00 + spriteIndex + 1) - 8;
			uint8_t tileIndex = mmu->Read(0xFE00 + spriteIndex + 2);
			uint8_t attributes = mmu->Read(0xFE00 + spriteIndex + 3);

			bool hasPriority = (attributes & 0b10000000) > 0;
			bool flipY = (attributes & 0b01000000) > 0;
			bool flipX = (attributes & 0b00100000) > 0;
			uint16_t palette = (attributes & 0b00010000) > 0 ? OBP1 : OBP0;

			uint16_t tileAddress = 0x8000 + tileIndex * 16;

			uint8_t tileDataLow = mmu->Read(tileAddress + (line - spriteY) * 2);
			uint8_t tileDataHigh = mmu->Read(tileAddress + (line - spriteY) * 2 + 1);

			uint16_t screenPosBase = line * 160 + spriteX;

			for (int8_t pixel = 7; pixel >= 0; pixel--) {
				int16_t screenPos = screenPosBase + (7 - pixel);
				if (screenPos >= 0 && screenPos <= LCDWidth * LCDHeight) {
					uint8_t lowBit = (tileDataLow >> pixel) & 0x01;
					uint8_t highBit = (pixel > 0 ? tileDataHigh >> (pixel - 1) : tileDataHigh << 1) & 0x02;
					uint8_t id = lowBit | highBit;

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
