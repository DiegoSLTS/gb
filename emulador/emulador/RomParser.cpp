#include "RomParser.h"

#include "Types.h"
#include "CPUOpCodes.h"
#include <algorithm>
#include <iomanip>
#include <sstream>
#include <fstream>
#include <iostream>

RomParser::RomParser() {}
RomParser::~RomParser() {}

void RomParser::ParseSection(const u8* bytes, u16 from, u16 to) {
	// 0xC0, 0xD0, 0xC8, 0xD8, 0xC9 = RET
	// 0xD9 = RETI
	// 0x20, 0x30, 0x18, 0x28, 0x38 = JR
	// 0xC2, 0xD2, 0xC3, 0xE9, 0xCA, 0xDA = JP
	// 0xC7, 0xD7, 0xE7, 0xF7, 0xCF, 0xDF, 0xEF, 0xFF = RST
	// 0xC4, 0xD4, 0xCC, 0xDC, 0xCD = CALL
	const u8 allJumpCodes[] = { 0xC0, 0xD0, 0xC8, 0xD8, 0xC9, 0xD9, 0x20, 0x30, 0x18, 0x28, 0x38, 0xC2, 0xD2, 0xC3, 0xE9, 0xCA, 0xDA, 0xC7, 0xD7, 0xE7, 0xF7, 0xCF, 0xDF, 0xEF, 0xFF, 0xC4, 0xD4, 0xCC, 0xDC, 0xCD };
	// this opCodes move the PC to another address and don't return (like CALL), so it's not safe to keep parsing after them
	const u8 endOfBlockJumpCodes[] = { 0xC9, 0xD9, 0x18, 0xC3, 0xE9, 0xC7, 0xD7, 0xE7, 0xF7, 0xCF, 0xDF, 0xEF, 0xFF };
	// this opCodes move the PC to an absolute or relative address known at compile time (exluding RST)
	const u8 relativeJumpCodes[] = { 0x20, 0x30, 0x18, 0x28, 0x38 };
	const u8 absoluteJumpCodes[] = { 0xC2, 0xD2, 0xC3, 0xCA, 0xDA, 0xC4, 0xD4, 0xCC, 0xDC, 0xCD };
	u8* position = nullptr;

	CodeSection section;
	section.addressStart = from;

	pc = from;
	while (pc < to) {
		Instruction instruction = ParseInstruction(bytes, to);
		section.instructions.push_back(instruction);
		if (std::find(allJumpCodes, allJumpCodes + 30, instruction.opCode) != allJumpCodes + 30) {
			if (std::find(relativeJumpCodes, relativeJumpCodes + 5, instruction.opCode) != relativeJumpCodes + 5) {
				u16 jumpAddress = pc + (s8)instruction.byte1;
				jumpTargets.push_back(jumpAddress);
			} else if (std::find(absoluteJumpCodes, absoluteJumpCodes + 10, instruction.opCode) != absoluteJumpCodes + 10) {
				u16 jumpAddress = (instruction.byte2 << 8) + instruction.byte1;
				jumpTargets.push_back(jumpAddress);
			}

			if (std::find(endOfBlockJumpCodes, endOfBlockJumpCodes + 13, instruction.opCode) != endOfBlockJumpCodes + 13)
				break;
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

void RomParser::ParseBiosROM(const u8* bytes, u16 size) {
	jumpTargets.push_back(0);
	
	Parse(bytes, size);
}

void RomParser::ParseCartridgeROM(const u8* bytes, u16 size) {
	// all RST addresses
	if (!AreAllNOP(bytes, 0x00, 0x08))
		jumpTargets.push_back(0x00);
	if (!AreAllNOP(bytes, 0x08, 0x10))
		jumpTargets.push_back(0x08);
	if (!AreAllNOP(bytes, 0x10, 0x18))
		jumpTargets.push_back(0x10);
	if (!AreAllNOP(bytes, 0x18, 0x20))
		jumpTargets.push_back(0x18);
	if (!AreAllNOP(bytes, 0x20, 0x28))
		jumpTargets.push_back(0x20);
	if (!AreAllNOP(bytes, 0x28, 0x30))
		jumpTargets.push_back(0x28);
	if (!AreAllNOP(bytes, 0x30, 0x38))
		jumpTargets.push_back(0x30);
	if (!AreAllNOP(bytes, 0x38, 0x40))
		jumpTargets.push_back(0x38);

	// parse all interrupts addresses
	if (!AreAllNOP(bytes, 0x40, 0x48)) // V-Blank
		jumpTargets.push_back(0x40);
	if (!AreAllNOP(bytes, 0x48, 0x50)) // LCD Stat
		jumpTargets.push_back(0x48);
	if (!AreAllNOP(bytes, 0x50, 0x58)) // Timer
		jumpTargets.push_back(0x50);
	if (!AreAllNOP(bytes, 0x58, 0x60)) // Serial
		jumpTargets.push_back(0x58);
	if (!AreAllNOP(bytes, 0x60, 0x68)) // Joypad
		jumpTargets.push_back(0x60);
	
	// rom start
	jumpTargets.push_back(0x100);

	Parse(bytes, size);
}

void RomParser::Parse(const u8* bytes, u16 size) {
	while (!jumpTargets.empty()) {
		u16 address = jumpTargets[jumpTargets.size() - 1];
		jumpTargets.pop_back();
		if (!IsAlreadyParsed(address))
			ParseSection(bytes, address, size);
	}
}

Instruction RomParser::ParseInstruction(const u8* bytes, u16 size) {
	Instruction newInstruction;
	newInstruction.address = pc;

	u8 opCode = bytes[pc++];
	newInstruction.opCode = opCode;

	u8 opCodeSize = GetSize(opCode);
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

u8 RomParser::GetSize(u8 opCode) const {
	return opCodeSizes[opCode];
}

std::string RomParser::GetFormat(u8 opCode, u8 byte1) const {
	return opCode != 0xCB ? opCodeFormats[opCode] : cbOpCodeFormats[byte1];
}

void RomParser::PrintCode() {
	for (auto& section : sections)
		for (auto&& instruction : section.instructions)
			PrintInstruction(instruction);
}

void RomParser::PrintCodeToFile() {
	std::ofstream stream;
	stream.open("out.txt");

	for (auto& section : sections)
		for (auto&& instruction : section.instructions)
			PrintInstructionToFile(stream, instruction);

	stream.close();
}

void RomParser::PrintInstruction(const Instruction& instruction) {
	std::cout << std::hex << "0x" << instruction.address << ": 0x" << (unsigned int)instruction.opCode << " -- " << instruction.displayText << std::endl;
}

void RomParser::PrintInstructionToFile(std::ostream& stream, const Instruction& instruction) {
	stream << std::hex << "0x" << instruction.address << ": 0x" << (unsigned int)instruction.opCode << " -- " << instruction.displayText << std::endl;
}

bool RomParser::IsAlreadyParsed(u16 address) {
	for (CodeSection& section : sections)
		if (address >= section.addressStart && address < section.addressEnd)
			return true;
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

bool RomParser::AreAllNOP(const u8* bytes, u16 from, u16 to) const {
	for (int i = from; i < to; i++)
		if (bytes[i] != 0)
			return false;
	return true;
}

std::string RomParser::ByteToHex(u8 byte) {
	std::stringstream stream;
	stream << std::setfill('0') << std::setw(2)	<< std::hex << (unsigned int)byte;
	return stream.str();
}