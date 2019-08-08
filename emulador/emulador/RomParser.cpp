#include "RomParser.h"

#include "CPUOpCodes.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <fstream>

void RomParser::ParseSection(uint8_t* bytes, uint16_t from, uint16_t to) {
	// 0xC0, 0xD0, 0xC8, 0xD8, 0xC9 = RET
	// 0xD9 = RETI
	// 0x20, 0x30, 0x18, 0x28, 0x38 = JR
	// 0xC2, 0xD2, 0xC3, 0xE9, 0xCA, 0xDA = JP
	// 0xC7, 0xD7, 0xE7, 0xF7, 0xCF, 0xDF, 0xEF, 0xFF = RST
	// 0xC4, 0xD4, 0xCC, 0xDC, 0xCD = CALL
	const uint8_t allJumpCodes[] = { 0xC0, 0xD0, 0xC8, 0xD8, 0xC9, 0xD9, 0x20, 0x30, 0x18, 0x28, 0x38, 0xC2, 0xD2, 0xC3, 0xE9, 0xCA, 0xDA, 0xC7, 0xD7, 0xE7, 0xF7, 0xCF, 0xDF, 0xEF, 0xFF, 0xC4, 0xD4, 0xCC, 0xDC, 0xCD };
	// this opCodes move the PC to another address and don't return (like CALL), so it's not safe to keep parsing after them
	const uint8_t endOfBlockJumpCodes[] = { 0xC9, 0xD9, 0x18, 0xC3, 0xE9, 0xC7, 0xD7, 0xE7, 0xF7, 0xCF, 0xDF, 0xEF, 0xFF };
	// this opCodes move the PC to an absolute or relative address known at compile time (exluding RST)
	const uint8_t relativeJumpCodes[] = { 0x20, 0x30, 0x18, 0x28, 0x38 };
	const uint8_t absoluteJumpCodes[] = { 0xC2, 0xD2, 0xC3, 0xCA, 0xDA, 0xC4, 0xD4, 0xCC, 0xDC, 0xCD };
	uint8_t* position = nullptr;

	CodeSection section;
	section.addressStart = from;

	pc = from;
	while (pc < to) {
		Instruction instruction = ParseInstruction(bytes, to);
		section.instructions.push_back(instruction);
		if (std::find(allJumpCodes, allJumpCodes + 30, instruction.opCode) != allJumpCodes + 30) {
			if (std::find(relativeJumpCodes, relativeJumpCodes + 5, instruction.opCode) != relativeJumpCodes + 5) {
				uint16_t jumpAddress = pc + (int8_t)instruction.byte1;
				jumpTargets.push_back(jumpAddress);
			} else if (std::find(absoluteJumpCodes, absoluteJumpCodes + 10, instruction.opCode) != absoluteJumpCodes + 10) {
				uint16_t jumpAddress = (instruction.byte2 << 8) + instruction.byte1;
				jumpTargets.push_back(jumpAddress);
			}

			if (std::find(endOfBlockJumpCodes, endOfBlockJumpCodes + 13, instruction.opCode) != endOfBlockJumpCodes + 13) {
				break;
			}
		}
	}
	section.addressEnd = pc - 1;

	// insert new section in address order or replace an existing one if the new one contains it
	bool containsAnother = ContainsAnotherSection(section);
	size_t insertIndex = GetInsertIndex(section);
	if (insertIndex != -1) {
		if (containsAnother)
			sections[insertIndex] = section;
		else
			sections.insert(sections.begin() + insertIndex, section);
	}
}

void RomParser::ParseBiosROM(uint8_t* bytes, uint16_t size) {
	ParseSection(bytes, 0, size);
	
	while (!jumpTargets.empty()) {
		uint16_t address = jumpTargets[jumpTargets.size() - 1];
		jumpTargets.pop_back();
		if (!IsAlreadyParsed(address)) {
			ParseSection(bytes, address, size);
		}
	}
}

void RomParser::ParseCartridgeROM(uint8_t* bytes, uint16_t size) {
	// parse all RST addresses
	if (!AreAllNOP(bytes, 0x00, 0x08))
		ParseSection(bytes, 0x00, 0x08);
	if (!AreAllNOP(bytes, 0x08, 0x10))
		ParseSection(bytes, 0x08, 0x10);
	if (!AreAllNOP(bytes, 0x10, 0x18))
		ParseSection(bytes, 0x10, 0x18);
	if (!AreAllNOP(bytes, 0x18, 0x20))
		ParseSection(bytes, 0x18, 0x20);
	if (!AreAllNOP(bytes, 0x20, 0x28))
		ParseSection(bytes, 0x20, 0x28);
	if (!AreAllNOP(bytes, 0x28, 0x30))
		ParseSection(bytes, 0x28, 0x30);
	if (!AreAllNOP(bytes, 0x30, 0x38))
		ParseSection(bytes, 0x30, 0x38);
	if (!AreAllNOP(bytes, 0x38, 0x40))
		ParseSection(bytes, 0x38, 0x40);

	// parse all interrupts addresses
	if (!AreAllNOP(bytes, 0x40, 0x48)) // V-Blank
		ParseSection(bytes, 0x40, 0x48);
	if (!AreAllNOP(bytes, 0x48, 0x50)) // LCD Stat
		ParseSection(bytes, 0x48, 0x50);
	if (!AreAllNOP(bytes, 0x50, 0x58)) // Timer
		ParseSection(bytes, 0x50, 0x58);
	if (!AreAllNOP(bytes, 0x58, 0x60)) // Serial
		ParseSection(bytes, 0x58, 0x60);
	if (!AreAllNOP(bytes, 0x60, 0x68)) // Joypad
		ParseSection(bytes, 0x60, 0x68);
	
	// parse rom
	ParseSection(bytes, 0x100, size);

	while (!jumpTargets.empty()) {
		uint16_t address = jumpTargets[jumpTargets.size() - 1];
		jumpTargets.pop_back();
		if (!IsAlreadyParsed(address)) {
			ParseSection(bytes, address, size);
		}
	}
}

Instruction RomParser::ParseInstruction(uint8_t* bytes, uint16_t size) {
	Instruction newInstruction;
	newInstruction.address = pc;

	uint8_t opCode = bytes[pc++];
	newInstruction.opCode = opCode;

	uint8_t opCodeSize = GetSize(opCode);
	if (opCodeSize > 1)
		newInstruction.byte1 = bytes[pc++];

	if (opCodeSize > 2)
		newInstruction.byte2 = bytes[pc++];

	std::string format = GetFormat(opCode, newInstruction.byte1);

	if (opCode == 0xCB || opCodeSize == 1)
		newInstruction.displayText = format;
	else if (opCodeSize == 2) {
		size_t pos = format.find("n");
		newInstruction.displayText = format.replace(pos, 1, "0x" + ByteToHex(newInstruction.byte1));
	} else {
		size_t pos = format.find("nn");
		newInstruction.displayText = format.replace(pos, 2, "0x" + ByteToHex(newInstruction.byte2) + ByteToHex(newInstruction.byte1));
	}

	return newInstruction;
}

uint8_t RomParser::GetSize(uint8_t opCode) const {
	return opCodeSizes[opCode];
}

std::string RomParser::GetFormat(uint8_t opCode, uint8_t byte1) const {
	if (opCode != 0xCB)
		return opCodeFormats[opCode];
	else
		return cbOpCodeFormats[byte1];
}

void RomParser::PrintCode() {
	for (auto& section : sections) {
		for (auto&& instruction : section.instructions) {
			PrintInstruction(instruction);
		}
	}
}

void RomParser::PrintCodeToFile() {
	std::ofstream stream;
	stream.open("out.txt");

	for (auto& section : sections) {
		for (auto&& instruction : section.instructions) {
			PrintInstructionToFile(stream, instruction);
		}
	}

	stream.close();
}

void RomParser::PrintInstruction(const Instruction& instruction) {
	printf("0x%04x - 0x%02x - %s\n", instruction.address, instruction.opCode, instruction.displayText.c_str());
}

void RomParser::PrintInstructionToFile(std::ostream& stream, const Instruction& instruction) {
	stream << std::hex << "0x" << instruction.address << ": 0x" << (unsigned int)instruction.opCode << " -- " << instruction.displayText << std::endl;
}

bool RomParser::IsAlreadyParsed(uint16_t address) {
	for (CodeSection& section : sections) {
		if (address >= section.addressStart && address < section.addressEnd) {
			return true;
		}
	}
	return false;
}

size_t RomParser::GetInsertIndex(const CodeSection& newSection) const {
	if (sections.empty())
		return 0;
		
	for (size_t i = 0; i < sections.size(); i++) {
		const CodeSection& section = sections[i];
		if (newSection.addressStart < section.addressStart)
			return i;
	}

	const CodeSection& lastSection = sections[sections.size() - 1];
	if (newSection.addressStart > lastSection.addressEnd)
		return sections.size();

	return -1;
}

bool RomParser::ContainsAnotherSection(const CodeSection& newSection) const {
	for (auto& section : sections)
		if (newSection.addressStart <= section.addressStart && newSection.addressEnd >= section.addressEnd)
			return true;
	return false;
}

bool RomParser::AreAllNOP(uint8_t* bytes, uint16_t from, uint16_t to) const {
	for (int i = from; i < to; i++)
		if (bytes[i] != 0)
			return false;
	return true;
}

std::string RomParser::ByteToHex(uint8_t byte) {
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(2)	<< std::hex << (unsigned int)byte;
	return stream.str();
}