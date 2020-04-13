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
		u8 lowByte = gameBoy.gpu.ReadVRAM(tileDataAddress + line * 2, VRAMBank);
		u8 highByte = gameBoy.gpu.ReadVRAM(tileDataAddress + line * 2 + 1, VRAMBank);

        u16 screenPosBase = (y * 8 + line) * width + x * 8;
        for (s8 pixel = 7; pixel >= 0; pixel--) {
            u16 screenPos = screenPosBase + (7 - pixel);
			u8 lowBit = (lowByte >> pixel) & 0x01;
			u8 highBit = (highByte >> pixel) & 0x01;
			PixelInfo pixelInfo = { 0 };
			pixelInfo.colorIndex = lowBit | (highBit << 1);
            pixelInfo.paletteIndex = cgbPaletteIndex;
			pixelInfo.isBG = displayAsBG;

			((sf::Uint32*)screenArray)[screenPos] = gameBoy.gpu.GetABGR(pixelInfo).v;
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

void TileViewer::ToggleBgSprite() {
    displayAsBG = !displayAsBG;
    UpdateTitle();
}

void TileViewer::UpdateTitle() {
    if (isCGB)
        SetTitle(title + " (CGB) " + (displayAsBG ? "BG" : "OBJ") + " - Bank: " + std::to_string(VRAMBank) + " - Palette: " + std::to_string(cgbPaletteIndex));
    else
        SetTitle(title + " (GB) - " + (displayAsBG ? "BG" : "OBJ"));
}

void TileViewer::PrintTile(u8 x, u8 y) {
    printf("\nTile X: %d Y: %d\n", x, y);
    printf("Map address: %s\n", Logger::u16ToHex(GetTileAddress(x, y)).c_str());
}

void TileViewer::OnMouseClicked(u32 x, u32 y) {
    u8 newX = x / 16; // tile size (8) * window scale (2)
    u8 newY = y / 16;

    if (newX != loggedTileX || newY != loggedTileY) {
        loggedTileX = newX;
        loggedTileY = newY;
        PrintTile(loggedTileX, loggedTileY);
    }
}
