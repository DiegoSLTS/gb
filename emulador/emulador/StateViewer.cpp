#include "StateViewer.h"

#include "CPU.h"

StateViewer::StateViewer(CPU& Cpu, GPU& Gpu, MMU& Mmu) : Window(150,144,"State"), cpu(Cpu), gpu(Gpu), mmu(Mmu) {
	if (!font.loadFromFile("Pokemon_GB[1].ttf"))
	{
		// error...
	}

	SetupLabel(labelAF, "AF", 2, 2);
	SetupLabel(labelBC, "BC", 2, 12);
	SetupLabel(labelDE, "DE", 2, 22);
	SetupLabel(labelHL, "HL", 2, 32);
	SetupLabel(labelPC, "PC", 2, 42);
	SetupLabel(labelSP, "SP", 2, 52);

	SetupValue(valueAF, 10, 2);
	SetupValue(valueBC, 10, 12);
	SetupValue(valueDE, 10, 22);
	SetupValue(valueHL, 10, 32);
	SetupValue(valuePC, 10, 42);
	SetupValue(valueSP, 10, 52);

	Update();
}

StateViewer::~StateViewer() {}

void StateViewer::Update() {
	/*valueAF.setString(std::to_string(cpu.Read16BitReg(CPU16BitReg::af)));

	renderWindow->clear();
	renderWindow->draw(labelAF);
	renderWindow->draw(labelBC);
	renderWindow->draw(labelDE);
	renderWindow->draw(labelHL);
	renderWindow->draw(labelPC);
	renderWindow->draw(labelSP);
	renderWindow->draw(valueAF);
	renderWindow->draw(valueBC);
	renderWindow->draw(valueDE);
	renderWindow->draw(valueHL);
	renderWindow->draw(valuePC);
	renderWindow->draw(valueSP);
	renderWindow->display();*/
}

void StateViewer::SetupLabel(sf::Text& label, const std::string& text, float x, float y) {
	label.setFont(font);
	label.setString(text);
	label.setCharacterSize(10);
	label.setFillColor(sf::Color::Black);
	label.setStyle(sf::Text::Style::Bold);
	label.setPosition(x, y);
}

void StateViewer::SetupValue(sf::Text& value, float x, float y) {
	value.setFont(font);
	value.setCharacterSize(10);
	value.setFillColor(sf::Color::Black);
	value.setStyle(sf::Text::Style::Regular);
	value.setPosition(x, y);
}
