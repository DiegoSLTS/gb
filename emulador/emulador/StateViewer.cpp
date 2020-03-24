#include "StateViewer.h"

#include "GameBoy.h"

#include <iomanip>

StateViewer::StateViewer(GameBoy& gameBoy) : Window(250, 144, "State", {400,50}, false, false), cpu(gameBoy.cpu), gpu(gameBoy.gpu) {
	if (!font.loadFromFile("Pokemon_GB.ttf"))
	{
		// error...
	}

	SetupLabel(labelPC, "PC", 2, 42);
	SetupLabel(labelSP, "SP", 2, 52);

	SetupValue(valueAF, 2, 2);
	SetupValue(valueBC, 2, 12);
	SetupValue(valueDE, 2, 22);
	SetupValue(valueHL, 2, 32);
	SetupValue(valuePC, 52, 42);
	SetupValue(valueSP, 52, 52);

	SetupLabel(labelLCDC, "LCDC", 110, 2);
	SetupLabel(labelLCDStat, "LCDS", 110, 12);
	SetupLabel(labelLY, "LY", 110, 22);
	SetupLabel(labelLYC, "LYC", 110, 32);
	SetupLabel(labelSCX, "SCX", 110, 42);
	SetupLabel(labelSCY, "SCY", 110, 52);
	SetupLabel(labelWX, "WX", 110, 62);
	SetupLabel(labelWY, "WY", 110, 72);
	SetupLabel(labelBGPI, "BGPI", 110, 82);
	SetupLabel(labelOBPI, "OBPI", 110, 92);
	SetupLabel(labelBGP, "BGP", 110, 102);
	SetupLabel(labelOBP0, "OBP0", 110, 112);
	SetupLabel(labelOBP1, "OBP1", 110, 122);

	SetupValue(valueLCDC, 183, 2);
	SetupValue(valueLCDStat, 183, 12);
	SetupValue(valueLY, 183, 22);
	SetupValue(valueLYC, 183, 32);
	SetupValue(valueSCX, 183, 42);
	SetupValue(valueSCY, 183, 52);
	SetupValue(valueWX, 183, 62);
	SetupValue(valueWY, 183, 72);
	SetupValue(valueBGPI, 183, 82);
	SetupValue(valueOBPI, 183, 92);
	SetupValue(valueBGP, 183, 102);
	SetupValue(valueOBP0, 183, 112);
	SetupValue(valueOBP1, 183, 122);

	Update();
}

StateViewer::~StateViewer() {}

void StateViewer::Update() {
	valueAF.setString(cpu.reg16ToString(CPU16BitReg::af));
	valueBC.setString(cpu.reg16ToString(CPU16BitReg::bc));
	valueDE.setString(cpu.reg16ToString(CPU16BitReg::de));
	valueHL.setString(cpu.reg16ToString(CPU16BitReg::hl));
	valuePC.setString(ToHex(cpu.pc));
	valueSP.setString(ToHex(cpu.sp));

	valueLCDC.setString(ToHex(gpu.Read(0xFF40)));
	valueLCDStat.setString(ToHex(gpu.Read(0xFF41)));
	valueLY.setString(ToHex(gpu.Read(0xFF44)));
	valueLYC.setString(ToHex(gpu.Read(0xFF45)));
	valueSCX.setString(ToHex(gpu.Read(0xFF43)));
	valueSCY.setString(ToHex(gpu.Read(0xFF42)));
	valueWX.setString(ToHex(gpu.Read(0xFF4B)));
	valueWY.setString(ToHex(gpu.Read(0xFF4A)));
	valueBGPI.setString(ToHex(gpu.Read(0xFF68)));
	valueOBPI.setString(ToHex(gpu.Read(0xFF6A)));
	valueBGP.setString(ToHex(gpu.Read(0xFF47)));
	valueOBP0.setString(ToHex(gpu.Read(0xFF48)));
	valueOBP1.setString(ToHex(gpu.Read(0xFF49)));

	renderWindow->clear();
	renderWindow->draw(labelPC);
	renderWindow->draw(labelSP);
	renderWindow->draw(valueAF);
	renderWindow->draw(valueBC);
	renderWindow->draw(valueDE);
	renderWindow->draw(valueHL);
	renderWindow->draw(valuePC);
	renderWindow->draw(valueSP);

	renderWindow->draw(labelLCDC);
	renderWindow->draw(labelLCDStat);
	renderWindow->draw(labelLY);
	renderWindow->draw(labelLYC);
	renderWindow->draw(labelSCX);
	renderWindow->draw(labelSCY);
	renderWindow->draw(labelWX);
	renderWindow->draw(labelWY);
	renderWindow->draw(labelBGPI);
	renderWindow->draw(labelOBPI);
	renderWindow->draw(labelBGP);
	renderWindow->draw(labelOBP0);
	renderWindow->draw(labelOBP1);
	renderWindow->draw(valueLCDC);
	renderWindow->draw(valueLCDStat);
	renderWindow->draw(valueLY);
	renderWindow->draw(valueLYC);
	renderWindow->draw(valueSCX);
	renderWindow->draw(valueSCY);
	renderWindow->draw(valueWX);
	renderWindow->draw(valueWY);
	renderWindow->draw(valueBGPI);
	renderWindow->draw(valueOBPI);
	renderWindow->draw(valueBGP);
	renderWindow->draw(valueOBP0);
	renderWindow->draw(valueOBP1);
	renderWindow->display();
}

void StateViewer::SetupLabel(sf::Text& label, const std::string& text, float x, float y) {
	label.setFont(font);
	label.setString(text);
	label.setCharacterSize(10);
	label.setFillColor(sf::Color::White);
	label.setStyle(sf::Text::Style::Regular);
	label.setPosition(x, y);
}

void StateViewer::SetupValue(sf::Text& value, float x, float y) {
	value.setFont(font);
	value.setCharacterSize(10);
	value.setFillColor(sf::Color::White);
	value.setStyle(sf::Text::Style::Regular);
	value.setPosition(x, y);
}

std::string StateViewer::ToHex(u8 value) {
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(2) << std::hex << (u16)value;
	return stream.str();
}

std::string StateViewer::ToHex(u16 value) {
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(4) << std::hex << value;
	return stream.str();
}
