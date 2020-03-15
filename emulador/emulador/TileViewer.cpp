#include "TileViewer.h"
#include "GameBoy.h"
#include "GPU.h"
#include "MMU.h"

TileViewer::TileViewer(unsigned int Width, unsigned int Height, const std::string& Title, const sf::Vector2i& Position, GameBoy& GameBoy) : Window(Width, Height, Title, Position, false), gameBoy(GameBoy) {
	tilesPerRow = Width / 8;
	rows = Height / 8;
	Update();

    isCGB = gameBoy.IsCGB;
    UpdateTitle();
}

TileViewer::~TileViewer() {}

void TileViewer::Update() {
    if (!IsOpen())
        return;

	for (u8 y = 0; y < rows; y++)
		for (u8 x = 0; x < tilesPerRow; x++)
			UpdateTile(x, y);

	screenTexture.update(screenArray);
	screenSprite.setTexture(screenTexture, true);

	renderWindow->clear();
	renderWindow->draw(screenSprite);
	renderWindow->display();
}

u16 TileViewer::GetTileAddress(u8 x, u8 y) const {
	return 0x8000 + (y * tilesPerRow + x) * 16;
}

void TileViewer::UpdateTile(u8 x, u8 y) {
	u16 tileDataAddress = GetTileAddress(x, y);

	for (int line = 0; line < 8; line++) {
        u8 lowByte = 0;
        u8 highByte = 0;

        if (VRAMBank == 0) {
            lowByte = gameBoy.gpu.ReadVRAM0(tileDataAddress + line * 2);
            highByte = gameBoy.gpu.ReadVRAM0(tileDataAddress + line * 2 + 1);
        } else {
            lowByte = gameBoy.gpu.ReadVRAM1(tileDataAddress + line * 2);
            highByte = gameBoy.gpu.ReadVRAM1(tileDataAddress + line * 2 + 1);
        }
		
        u16 screenPosBase = (y * 8 + line) * screenTexture.getSize().x + x * 8;

        for (s8 pixel = 7; pixel >= 0; pixel--) {
            u16 screenPos = screenPosBase + (7 - pixel);
			u8 lowBit = (lowByte >> pixel) & 0x01;
			u8 highBit = (highByte >> pixel) & 0x01;
			u8 index = lowBit | (highBit << 1);

			if (isCGB)
				index |= (cgbPaletteIndex << 2);

			index |= 0x20;
			((sf::Uint32*)screenArray)[screenPos] = gameBoy.gpu.GetABGR(index);
        }
	}
}

void TileViewer::NextPalette() {
    if (cgbPaletteIndex == 7)
        cgbPaletteIndex = 0;
    else
        cgbPaletteIndex++;
    UpdateTitle();
}

void TileViewer::PreviousPalette() {
    if (cgbPaletteIndex == 0)
        cgbPaletteIndex = 7;
    else
        cgbPaletteIndex--;
    UpdateTitle();
}

void TileViewer::ToggleBank() {
    VRAMBank = (VRAMBank + 1) % 2;
    UpdateTitle();
}

void TileViewer::UpdateTitle() {
    if (isCGB)
        SetTitle(title + " (CGB) - Bank: " + std::to_string(VRAMBank) + " - Palette: " + std::to_string(cgbPaletteIndex));
}
