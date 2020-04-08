#include "PalettesViewer.h"

#include "GameBoy.h"

#include <iomanip>
#include <sstream>

PalettesViewer::PalettesViewer(GameBoy& gameBoy) : Window(250, 200, "Palettes", { 700,50 }, false, false), gpu(gameBoy.gpu) {
	bgpMemory = gpu.GetBGPPtr();
	obpMemory = gpu.GetOBPPtr();

	if (!font.loadFromFile("Pokemon_GB.ttf"))
	{
		// error...
	}

	SetupLabel(labelBGP, "BGP", 2, 2);
	SetupLabel(labelOBP0, "OBP0", 87, 2);
	SetupLabel(labelOBP1, "OBP1", 172, 2);
	SetupLabel(labelBG0, "BG0", 2, 22);
	SetupLabel(labelBG1, "BG1", 2, 32);
	SetupLabel(labelBG2, "BG2", 2, 42);
	SetupLabel(labelBG3, "BG3", 2, 52);
	SetupLabel(labelBG4, "BG4", 2, 62);
	SetupLabel(labelBG5, "BG5", 2, 72);
	SetupLabel(labelBG6, "BG6", 2, 82);
	SetupLabel(labelBG7, "BG7", 2, 92);
	SetupLabel(labelOB0, "OB0", 2, 112);
	SetupLabel(labelOB1, "OB1", 2, 122);
	SetupLabel(labelOB2, "OB2", 2, 132);
	SetupLabel(labelOB3, "OB3", 2, 142);
	SetupLabel(labelOB4, "OB4", 2, 152);
	SetupLabel(labelOB5, "OB5", 2, 162);
	SetupLabel(labelOB6, "OB6", 2, 172);
	SetupLabel(labelOB7, "OB7", 2, 182);

	SetupValue(valueBGP, 52, 2);
	SetupValue(valueOBP0, 137, 2);
	SetupValue(valueOBP1, 222, 2);
	SetupValue(valueBG0, 52, 22);
	SetupValue(valueBG1, 52, 32);
	SetupValue(valueBG2, 52, 42);
	SetupValue(valueBG3, 52, 52);
	SetupValue(valueBG4, 52, 62);
	SetupValue(valueBG5, 52, 72);
	SetupValue(valueBG6, 52, 82);
	SetupValue(valueBG7, 52, 92);
	SetupValue(valueOB0, 52, 112);
	SetupValue(valueOB1, 52, 122);
	SetupValue(valueOB2, 52, 132);
	SetupValue(valueOB3, 52, 142);
	SetupValue(valueOB4, 52, 152);
	SetupValue(valueOB5, 52, 162);
	SetupValue(valueOB6, 52, 172);
	SetupValue(valueOB7, 52, 182);
}

PalettesViewer::~PalettesViewer() {
	bgpMemory = nullptr;
	obpMemory = nullptr;
}

void PalettesViewer::Update() {
    if (!IsOpen())
        return;

	renderWindow->clear();

	valueBGP.setString(ToHex(gpu.Read(0xFF47)));
	valueOBP0.setString(ToHex(gpu.Read(0xFF48)));
	valueOBP1.setString(ToHex(gpu.Read(0xFF49)));
	renderWindow->draw(labelBGP);
	renderWindow->draw(labelOBP0);
	renderWindow->draw(labelOBP1);
	renderWindow->draw(valueBGP);
	renderWindow->draw(valueOBP0);
	renderWindow->draw(valueOBP1);

	if (gpu.isCGB) {
		valueBG0.setString(PaletteToString(0, true));
		valueBG1.setString(PaletteToString(1, true));
		valueBG2.setString(PaletteToString(2, true));
		valueBG3.setString(PaletteToString(3, true));
		valueBG4.setString(PaletteToString(4, true));
		valueBG5.setString(PaletteToString(5, true));
		valueBG6.setString(PaletteToString(6, true));
		valueBG7.setString(PaletteToString(7, true));

		valueOB0.setString(PaletteToString(0, false));
		valueOB1.setString(PaletteToString(1, false));
		valueOB2.setString(PaletteToString(2, false));
		valueOB3.setString(PaletteToString(3, false));
		valueOB4.setString(PaletteToString(4, false));
		valueOB5.setString(PaletteToString(5, false));
		valueOB6.setString(PaletteToString(6, false));
		valueOB7.setString(PaletteToString(7, false));
	
		renderWindow->draw(labelBG0);
		renderWindow->draw(labelBG1);
		renderWindow->draw(labelBG2);
		renderWindow->draw(labelBG3);
		renderWindow->draw(labelBG4);
		renderWindow->draw(labelBG5);
		renderWindow->draw(labelBG6);
		renderWindow->draw(labelBG7);
	
		renderWindow->draw(labelOB0);
		renderWindow->draw(labelOB1);
		renderWindow->draw(labelOB2);
		renderWindow->draw(labelOB3);
		renderWindow->draw(labelOB4);
		renderWindow->draw(labelOB5);
		renderWindow->draw(labelOB6);
		renderWindow->draw(labelOB7);
	
		renderWindow->draw(valueBG0);
		renderWindow->draw(valueBG1);
		renderWindow->draw(valueBG2);
		renderWindow->draw(valueBG3);
		renderWindow->draw(valueBG4);
		renderWindow->draw(valueBG5);
		renderWindow->draw(valueBG6);
		renderWindow->draw(valueBG7);
	
		renderWindow->draw(valueOB0);
		renderWindow->draw(valueOB1);
		renderWindow->draw(valueOB2);
		renderWindow->draw(valueOB3);
		renderWindow->draw(valueOB4);
		renderWindow->draw(valueOB5);
		renderWindow->draw(valueOB6);
		renderWindow->draw(valueOB7);
	}
	renderWindow->display();
}

std::string PalettesViewer::PaletteToString(u8 index, bool bg) {
	u8* mem = bg ? bgpMemory : obpMemory;
	std::stringstream palette;
	palette << ToHex(mem[index * 8 + 1]) << ToHex(mem[index * 8]) << " ";
	palette << ToHex(mem[index * 8 + 3]) << ToHex(mem[index * 8 + 2]) << " ";
	palette << ToHex(mem[index * 8 + 5]) << ToHex(mem[index * 8 + 4]) << " ";
	palette << ToHex(mem[index * 8 + 7]) << ToHex(mem[index * 8 + 6]);
	return palette.str();
}

void PalettesViewer::SetupLabel(sf::Text& label, const std::string& text, float x, float y) {
	label.setFont(font);
	label.setString(text);
	label.setCharacterSize(10);
	label.setFillColor(sf::Color::White);
	label.setStyle(sf::Text::Style::Regular);
	label.setPosition(x, y);
}

void PalettesViewer::SetupValue(sf::Text& value, float x, float y) {
	value.setFont(font);
	value.setCharacterSize(10);
	value.setFillColor(sf::Color::White);
	value.setStyle(sf::Text::Style::Regular);
	value.setPosition(x, y);
}

std::string PalettesViewer::ToHex(u8 value) {
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(2) << std::hex << (u16)value;
	return stream.str();
}

std::string PalettesViewer::ToHex(u16 value) {
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(4) << std::hex << value;
	return stream.str();
}
