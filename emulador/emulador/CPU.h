#pragma once

/*
				GB								GBC
Cpu 			4mhz 							8mhz
Ram 			8k 								32k
Vram 			8k 								16k
Resolution 		160x144 						160x144
Max Tiles 		256 (8x8 px) - 360 onscreen 	512 (8x8 px) - 360 onscreen
Max Sprites 	40 (8x8 px 10 per line) 		40 (8x8 px 10 per line)
Colors 			4 								4 per palette - 8 palletes (0-7) from 32768 colors
Sound chip 		GBZ80 PAPU 						GBZ80 PAPU

DOCS
http://marc.rawer.de/Gameboy/Docs/GBCPUman.pdf
http://bgb.bircd.org/pandocs.htm
https://rednex.github.io/rgbds/gbz80.7.html

*/

#include <cstdint>

#include "IState.h"
#include "CPUFlags.h"

class MMU;
class InterruptServiceRoutine;

enum CPU8BitReg {
	// this order asumes the system is little endian
	// TODO find solution that works for big endian too
	f = 0,
	a,
	c,
	b,
	e,
	d,
	l,
	h
};

enum CPU16BitReg {
	af = 0,
	bc,
	de,
	hl
};

class CPU : IState {
public:
	CPU();
	virtual ~CPU();
	
	MMU* mmu = nullptr;
	InterruptServiceRoutine* interruptService = nullptr;

	// registers
	uint8_t registers[8] = { 0 }; //TODO 0xB0, 0x01, 0x13, 0x00, 0xD8, 0x00, 0x4D, 0x01 at start or after bios?
	uint16_t pc = 0, sp = 0; //TODO pc = 0x0100, sp = 0xFFFE; at start or after bios?

	uint8_t lastOpCycles = 0;
	bool isHalted = false;

	uint8_t Read8BitReg(CPU8BitReg reg) const;
	uint16_t Read16BitReg(CPU16BitReg reg) const;

	void Write8BitReg(CPU8BitReg reg, uint8_t value);
	void Write16BitReg(CPU16BitReg reg, uint16_t value);

	uint8_t ReadAcc() const;
	void WriteAcc(uint8_t value);

	uint16_t ReadHL() const;
	void WriteHL(uint16_t value);

	void Push16(uint16_t value);
	uint16_t Pop16();
	// registers

	// memory
	uint8_t ReadMemory(uint16_t address);
	void WriteMemory(uint16_t address, uint8_t value);

	uint8_t ReadAtPC();
	uint8_t ReadOpCode();
	// memory

	// flags
	uint8_t ReadFlags() const;
	void WriteFlags(uint8_t newFlags);
	void ResetFlags();
	void SetFlag(FlagBit flagBit, bool set);
	bool HasFlag(FlagBit flagBit) const;

	void UpdateZeroFlag(uint8_t value);
	void UpdateNegativeFlag(uint8_t value);
	void UpdateHalfCarryFlag(uint8_t previous, uint8_t current, bool isAdd);
	void UpdateCarryFlag(uint8_t previous, uint8_t current, bool isAdd);

	void UpdateZeroFlag(uint16_t value);
	void UpdateNegativeFlag(uint16_t value);
	void UpdateHalfCarryFlag(uint16_t previous, uint16_t current, bool isAdd);
	void UpdateCarryFlag(uint16_t previous, uint16_t current, bool isAdd);
	// flags

	// API
	void CallOpCode(uint8_t opCode);
	void CallCBOpCode(uint8_t opCode);

	virtual void Load(std::ifstream& stream) const override;
	virtual void Save(std::ofstream& stream) const override;
	//

	// instructions
	void ADCAr8(CPU8BitReg reg);
	void ADCAHL();
	void ADCAn8();
	void ADDAr8(CPU8BitReg reg);
	void ADDAHL();
	void ADDAn8();
	void ANDAr8(CPU8BitReg reg);
	void ANDAHL();
	void ANDAn8();
	void ADDHLr16(CPU16BitReg reg);
	void ADDHLSP();
	void ADDSPe8();
	void BITu3r8(uint8_t bit, CPU8BitReg reg);
	void BITu3HL(uint8_t bit);
	void CALLn16();
	void CALLccn16(bool condition);
	void CCF();
	void CPAr8(CPU8BitReg reg);
	void CPAHL();
	void CPAn8();
	void CPL();
	void DAA();
	void DECr8(CPU8BitReg reg);
	void DECHL();
	void DECr16(CPU16BitReg reg);
	void DECSP();
	void DI();
	void EI();
	void HALT();
	void INCr8(CPU8BitReg reg);
	void INCHL();
	void INCr16(CPU16BitReg reg);
	void INCSP();
	void JPHL();
	void JPn16();
	void JPccn16(bool condition);
	void JRe8();
	void JRcce8(bool condition);
	void LDr8r8(CPU8BitReg leftReg, CPU8BitReg rightReg);
	void LDr8n8(CPU8BitReg reg);
	void LDr16n16(CPU16BitReg reg);
	void LDHLr8(CPU8BitReg reg);
	void LDHLn8();
	void LDr8HL(CPU8BitReg reg);
	void LDr16A(CPU16BitReg reg);
	void LDn16A();
	void LDHn8A();
	void LDHCA();
	void LDAr16(CPU16BitReg reg);
	void LDAn16();
	void LDHAn8();
	void LDHAC();
	void LDHLincA();
	void LDHLdecA();
	void LDAHLinc();
	void LDAHLdec();
	void LDSPn16();
	void LDn16SP();
	void LDHLSPe8();
	void LDSPHL();
	void NOP();
	void ORAr8(CPU8BitReg reg);
	void ORAHL();
	void ORAn8();
	void POPr16(CPU16BitReg reg);
	void PUSHr16(CPU16BitReg reg);
	void RESu3r8(uint8_t bit, CPU8BitReg reg);
	void RESu3HL(uint8_t bit);
	void RETcc(bool condition);
	void RET();
	void RETI();
	void RLr8(CPU8BitReg reg);
	void RLHL();
	void RLA();
	void RLCr8(CPU8BitReg reg);
	void RLCHL();
	void RLCA();
	void RRr8(CPU8BitReg reg);
	void RRHL();
	void RRA();
	void RRCr8(CPU8BitReg reg);
	void RRCHL();
	void RRCA();
	void RSTvec(uint8_t vec);
	void SBCAr8(CPU8BitReg reg);
	void SBCAHL();
	void SBCAn8();
	void SCF();
	void SETu3r8(uint8_t bit, CPU8BitReg reg);
	void SETu3HL(uint8_t bit);
	void SLAr8(CPU8BitReg reg);
	void SLAHL();
	void SRAr8(CPU8BitReg reg);
	void SRAHL();
	void SRLr8(CPU8BitReg reg);
	void SRLHL();
	void STOP();
	void SUBAr8(CPU8BitReg reg);
	void SUBAHL();
	void SUBAn8();
	void SWAPr8(CPU8BitReg reg);
	void SWAPHL();
	void XORAr8(CPU8BitReg reg);
	void XORAHL();
	void XORAn8();
	// instructions

};
