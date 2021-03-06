#include "CPU.h"
#include "MMU.h"
#include "InterruptServiceRoutine.h"
#include "Logger.h"

#include <iomanip>
#include <sstream>

std::string opCodeNames[256] = {
    "NOP", 			"LD ",		"LD ",	"INC ",		"INC ", 		"DEC ", 		"LD ", 		"RLC ", 		"LD ",	"ADD ", 	"LD ",	"DEC ", 	"INC ", 		"DEC ", 	"LD ",		"RRC A",
    "STOP",			"LD ",		"LD ",	"INC ", 		"INC ", 		"DEC ", 		"LD ", 		"RL ", 		"JR ", 		"ADD ", 	"LD ",	"DEC ", 	"INC ",		"DEC ", 	"LD ", 		"RR ",
    "JR NZ,", 		"LD ",		"LDI ",	"INC ", 		"INC ", 		"DEC ", 		"LD ", 		"DAA", 			"JR Z,", 		"ADD ", 	"LDI ",	"DEC ", 	"INC ", 		"DEC ", 	"LD ", 		"CPL",
    "JR NC,", 		"LD ",		"LDD ",	"INC ", 		"INC ", 	"DEC ", 	"LD ", 	"SCF", 			"JR C,", 		"ADD ", 	"LDD ",	"DEC ", 	"INC ", 		"DEC ", 	"LD ", 		"CCF",
    "LD ",		"LD ", 		"LD ",		"LD ", 		"LD ", 		"LD ", 		"LD ",	"LD ", 		"LD ", 		"LD ", 		"LD ", 		"LD ", 	"LD ",		"LD ", 	"LD ",	"LD ",
    "LD ", 		"LD ", 		"LD ", 		"LD ", 		"LD ", 		"LD ", 		"LD ",	"LD ", 		"LD ", 		"LD ", 		"LD ", 		"LD ", 	"LD ", 		"LD ", 	"LD ",	"LD ",
    "LD ", 		"LD ", 		"LD ", 		"LD ", 		"LD ", 		"LD ", 		"LD ",	"LD ", 		"LD ", 		"LD ", 		"LD ", 		"LD ", 	"LD ", 		"LD ", 	"LD ",	"LD ",
    "LD ",	"LD ",	"LD ",	"LD ",	"LD ", 	"LD ",	"HALT", 		"LD ",	"LD ", 		"LD ", 		"LD ", 		"LD ", 	"LD ", 		"LD ", 	"LD ",	"LD ",
    "ADD ", 		"ADD ", 		"ADD ", 		"ADD ", 		"ADD ", 		"ADD ", 		"ADD ",	"ADD ", 		"ADC ", 		"ADC ", 		"ADC ", 		"ADC ", 	"ADC ", 		"ADC ", 	"ADC ",	"ADC ",
    "SUB ", 		"SUB ", 		"SUB ", 		"SUB ", 		"SUB ", 		"SUB ", 		"SUB ",	"SUB ", 		"SBC ", 		"SBC ", 		"SBC ", 		"SBC ", 	"SBC ", 		"SBC ", 	"SBC ",	"SBC ",
    "AND ",		"AND ", 		"AND ", 		"AND ", 		"AND ", 		"AND ", 		"AND ", 	"AND ", 		"XOR ", 		"XOR ", 		"XOR ", 		"XOR ", 	"XOR ", 		"XOR ", 	"XOR ", 	"XOR ",
    "OR ",			"OR ", 		"OR ", 		"OR ", 		"OR ", 		"OR ", 		"OR ", 		"OR ", 		"CP ", 		"CP ",			"CP ", 		"CP ", 	"CP ", 		"CP ", 	"CP ", 		"CP ",
    "RET NZ", 		"POP ", 		"JP NZ,",		"JP ", 		"CALL NZ,", 	"PUSH ", 		"ADD ", 		"RST ", 		"RET Z", 		"RET", 			"JP Z,", 		"CB", 		"CALL Z,", 	"CALL ", 	"ADC ",		"RST ",
    "RET NC", 		"POP ", 		"JP NC,",		"XX", 			"CALL NC,",	"PUSH ", 		"SUB ", 		"RST ", 		"RET C", 		"RETI", 		"JP C,", 		"XX", 		"CALL C,", 	"XX", 		"SBC ", 		"RST ",
    "LDH ",	"POP ", 		"LDH ",	"XX", 			"XX", 			"PUSH ", 		"AND ", 		"RST ",		"ADD ", 	"JP ", 		"LD ", 	"XX", 		"XX", 			"XX",		"XOR ", 		"RST ",
    "LDH ",	"POP ", 		"LDH ", 	"DI", 			"XX", 			"PUSH ",		"OR ", 		"RST ", 		"LDHL ", 	"LD ", 	"LD ",	"EI", 		"XX", 			"XX", 		"CP ", 		"RST ",
};

std::string cbOpCodeNames[256] = {
    "RLC B", 	"RLC C", 	"RLC D", 	"RLC E", 	"RLC H", 	"RLC L", 	"RLC (HL)",		"RLC A", 	"RRC B", 	"RRC C", 	"RRC D", 	"RRC E", 	"RRC H", 	"RRC L", 	"RRC (HL)", 	"RRC A",
    "RL B", 	"RL C", 	"RL D", 	"RL E", 	"RL H", 	"RL L", 	"RL (HL)", 		"RL A", 	"RR B", 	"RR C", 	"RR D", 	"RR E", 	"RR H", 	"RR L", 	"RR (HL)", 		"RR A",
    "SLA B", 	"SLA C", 	"SLA D", 	"SLA E", 	"SLA H", 	"SLA L", 	"SLA (HL)",		"SLA A", 	"SRA B", 	"SRA C", 	"SRA D", 	"SRA E",	"SRA H", 	"SRA L", 	"SRA (HL)", 	"SRA A",
    "SWAP B",	"SWAP C", 	"SWAP D", 	"SWAP E", 	"SWAP H", 	"SWAP L", 	"SWAP (HL)", 	"SWAP A", 	"SRL B", 	"SRL C", 	"SRL D", 	"SRL E",	"SRL H", 	"SRL L", 	"SRL (HL)", 	"SRL A",
    "BIT 0,B", 	"BIT 0,C", 	"BIT 0,D", 	"BIT 0,E", 	"BIT 0,H", 	"BIT 0,L", 	"BIT 0,(HL)",	"BIT 0,A", 	"BIT 1,B", 	"BIT 1,C", 	"BIT 1,D", 	"BIT 1,E", 	"BIT 1,H", 	"BIT 1,L", 	"BIT 1,(HL)",	"BIT 1,A",
    "BIT 2,B", 	"BIT 2,C", 	"BIT 2,D", 	"BIT 2,E", 	"BIT 2,H", 	"BIT 2,L", 	"BIT 2,(HL)",	"BIT 2,A", 	"BIT 3,B", 	"BIT 3,C", 	"BIT 3,D", 	"BIT 3,E", 	"BIT 3,H", 	"BIT 3,L", 	"BIT 3,(HL)",	"BIT 3,A",
    "BIT 4,B", 	"BIT 4,C", 	"BIT 4,D", 	"BIT 4,E", 	"BIT 4,H", 	"BIT 4,L", 	"BIT 4,(HL)",	"BIT 4,A", 	"BIT 5,B", 	"BIT 5,C", 	"BIT 5,D", 	"BIT 5,E", 	"BIT 5,H", 	"BIT 5,L", 	"BIT 5,(HL)",	"BIT 5,A",
    "BIT 6,B", 	"BIT 6,C", 	"BIT 6,D", 	"BIT 6,E", 	"BIT 6,H", 	"BIT 6,L", 	"BIT 6,(HL)",	"BIT 6,A", 	"BIT 7,B", 	"BIT 7,C", 	"BIT 7,D", 	"BIT 7,E", 	"BIT 7,H", 	"BIT 7,L", 	"BIT 7,(HL)",	"BIT 7,A",
    "RES 0,B", 	"RES 0,C", 	"RES 0,D", 	"RES 0,E", 	"RES 0,H", 	"RES 0,L", 	"RES 0,(HL)",	"RES 0,A", 	"RES 1,B", 	"RES 1,C", 	"RES 1,D", 	"RES 1,E", 	"RES 1,H", 	"RES 1,L", 	"RES 1,(HL)",	"RES 1,A",
    "RES 2,B", 	"RES 2,C", 	"RES 2,D", 	"RES 2,E", 	"RES 2,H", 	"RES 2,L", 	"RES 2,(HL)",	"RES 2,A", 	"RES 3,B", 	"RES 3,C", 	"RES 3,D", 	"RES 3,E", 	"RES 3,H", 	"RES 3,L", 	"RES 3,(HL)",	"RES 3,A",
    "RES 4,B", 	"RES 4,C", 	"RES 4,D", 	"RES 4,E", 	"RES 4,H", 	"RES 4,L", 	"RES 4,(HL)",	"RES 4,A", 	"RES 5,B", 	"RES 5,C", 	"RES 5,D", 	"RES 5,E", 	"RES 5,H", 	"RES 5,L", 	"RES 5,(HL)",	"RES 5,A",
    "RES 6,B", 	"RES 6,C", 	"RES 6,D", 	"RES 6,E", 	"RES 6,H", 	"RES 6,L", 	"RES 6,(HL)",	"RES 6,A", 	"RES 7,B", 	"RES 7,C", 	"RES 7,D", 	"RES 7,E", 	"RES 7,H", 	"RES 7,L", 	"RES 7,(HL)",	"RES 7,A",
    "SET 0,B", 	"SET 0,C", 	"SET 0,D", 	"SET 0,E", 	"SET 0,H", 	"SET 0,L", 	"SET 0,(HL)",	"SET 0,A", 	"SET 1,B", 	"SET 1,C", 	"SET 1,D", 	"SET 1,E", 	"SET 1,H", 	"SET 1,L", 	"SET 1,(HL)",	"SET 1,A",
    "SET 2,B", 	"SET 2,C", 	"SET 2,D", 	"SET 2,E", 	"SET 2,H", 	"SET 2,L", 	"SET 2,(HL)",	"SET 2,A", 	"SET 3,B", 	"SET 3,C", 	"SET 3,D", 	"SET 3,E", 	"SET 3,H", 	"SET 3,L", 	"SET 3,(HL)",	"SET 3,A",
    "SET 4,B" 	"SET 4,C", 	"SET 4,D", 	"SET 4,E", 	"SET 4,H", 	"SET 4,L", 	"SET 4,(HL)",	"SET 4,A", 	"SET 5,B", 	"SET 5,C", 	"SET 5,D", 	"SET 5,E", 	"SET 5,H", 	"SET 5,L", 	"SET 5,(HL)",	"SET 5,A",
    "SET 6,B", 	"SET 6,C", 	"SET 6,D", 	"SET 6,E", 	"SET 6,H", 	"SET 6,L", 	"SET 6,(HL)",	"SET 6,A", 	"SET 7,B", 	"SET 7,C", 	"SET 7,D", 	"SET 7,E", 	"SET 7,H", 	"SET 7,L", 	"SET 7,(HL)",	"SET 7,A",
};

CPU::CPU(MMU& mmu, InterruptServiceRoutine& interruptService) : mmu(mmu), interruptService(interruptService) {}
CPU::~CPU() {}

u8 CPU::Step() {
	lastOpCycles = 0;

	if (interruptService.IE & interruptService.IF) {
		if (isHalted) {
            if (log) Logger::instance->log("cpu unhalted\n");
			isHalted = false;
			lastOpCycles = 1;
		}
		if (interruptService.IME) {
			for (int i = 0; i < 5 ; i++) {
				InterruptFlag flag = (InterruptFlag)(1 << i);
				if (interruptService.IsInterruptSet(flag) && interruptService.IsInterruptEnabled(flag)) {
					interruptService.IME = false;
					interruptService.ResetInterruptFlag(flag);
					Push16(pc); // cpu.lastOpCycles = 2
					pc = 0x40 + i * 8; //0x40, 0x48, 0x50, 0x58, 0x60
					lastOpCycles += 3; // +2 idle cycles, +1 updating PC
					break;
				}
			}
		}
	}

	if (interruptService.eiDelay) {
		interruptService.IME = true;
		interruptService.eiDelay = false;
	}

	// used only for debugging to break at specific instructions
	if (pc == 0x0419)
		int a = 0;

	if (!isHalted) {
		opCode = ReadAtPC();
		if (opCode == 0xCB) {
			opCode = ReadAtPC();
			CallCBOpCode(opCode);
		} else
			CallOpCode(opCode);
    } else {
        lastOpCycles = 1;
    }

	return lastOpCycles;
}

u8 CPU::Read(u16 address) {
    if (address == 0xFF4D) {
        return key1 | 0x7E;
    }
    return 0xFF;
}

void CPU::Write(u8 value, u16 address) {
    if (address == 0xFF4D) {
        key1 = (key1 & 0xFE) | (value & 0x01);
    }
}

std::string CPU::reg8ToString(CPU8BitReg reg) {
    return r8Names[reg] + " = " + Logger::instance->u8ToHex(Read8BitReg(reg));
}

std::string CPU::reg16ToString(CPU16BitReg reg) {
    return r16Names[reg] + " = " + Logger::instance->u16ToHex(Read16BitReg(reg));
}

bool CPU::IsDoubleSpeedEnabled() const {
    return (key1 & 0x80) == 0x80;
}

u8 CPU::Read8BitReg(CPU8BitReg reg) const {
	return registers[reg];
}

void CPU::Write8BitReg(CPU8BitReg reg, u8 value) {
	if (reg == CPU8BitReg::f)
		value &= 0xF0;
	registers[reg] = value;
}

u16 CPU::Read16BitReg(CPU16BitReg reg) const {
	u16* regs16 = (u16*)registers;
	return regs16[reg];
}

void CPU::Write16BitReg(CPU16BitReg reg, u16 value) {
	u16* regs16 = (u16*)registers;
	if (reg == CPU16BitReg::af)
		value &= 0xFFF0;
	regs16[reg] = value;
}

void CPU::SetFlag(FlagBit flagBit, bool set) {
	u8 flags = registers[CPU8BitReg::f];
	if (set)
		Write8BitReg(CPU8BitReg::f, flags | flagBit);
	else
		Write8BitReg(CPU8BitReg::f, flags & ~flagBit);
}

bool CPU::HasFlag(FlagBit flagBit) const {
	u8 flags = registers[CPU8BitReg::f];
	return (flags & flagBit) != 0;
}

bool CPU::UpdateZeroFlag(u8 value) {
	bool set = value == 0;
	SetFlag(FlagBit::Zero, set);
	return set;
}

bool CPU::UpdateZeroFlag(u16 value) {
	bool set = value == 0;
	SetFlag(FlagBit::Zero, set);
	return set;
}

bool CPU::UpdateHalfCarryFlag(u8 previous, u8 current, bool isAdd) {
	bool set = isAdd ? (current & 0x0F) < (previous & 0x0F) : (current & 0x0F) > (previous & 0x0F);
	SetFlag(FlagBit::HalfCarry, set);
	return set;
}

bool CPU::UpdateHalfCarryFlag(u16 previous, u16 current, bool isAdd) {
	bool set = isAdd ? (current & 0x0FFF) < (previous & 0x0FFF) : (current & 0x0FFF) > (previous & 0x0FFF);
	SetFlag(FlagBit::HalfCarry, set);
	return set;
}

bool CPU::UpdateCarryFlag(u8 previous, u8 current, bool isAdd) {
	bool set = isAdd ? current < previous : current > previous;
	SetFlag(FlagBit::Carry, set);
	return set;
}

bool CPU::UpdateCarryFlag(u16 previous, u16 current, bool isAdd) {
	bool set = isAdd ? current < previous : current > previous;
	SetFlag(FlagBit::Carry, set);
	return set;
}

u8 CPU::ReadAcc() const {
	return Read8BitReg(CPU8BitReg::a);
}

void CPU::WriteAcc(u8 value) {
	Write8BitReg(CPU8BitReg::a, value);
}

u16 CPU::ReadHL() const {
	return Read16BitReg(CPU16BitReg::hl);
}

void CPU::WriteHL(u16 value) {
	Write16BitReg(CPU16BitReg::hl, value);
}

u8 CPU::ReadMemory(u16 address) {
	lastOpCycles++;
	return mmu.Read(address);
}

void CPU::WriteMemory(u16 address, u8 value) {
	lastOpCycles++;
	mmu.Write(address, value);
}

u8 CPU::ReadAtPC() {
	u8 valueAtPC = mmu.Read(pc);
	if (!haltBug)
		pc++;
	else
		haltBug = false;
	lastOpCycles++;
	return valueAtPC;
}

void CPU::Push16(u16 value) {
	WriteMemory(sp - 1, value >> 8);
	WriteMemory(sp - 2, (u8)value);
	sp -= 2;
}

u16 CPU::Pop16() {
	u16 lsb = ReadMemory(sp);
	u16 msb = ReadMemory(sp + 1) << 8;
	sp += 2;
	return msb | lsb;
}

void CPU::Load(std::ifstream& stream) const {
	stream.read((char*)registers, 8);
	stream.read((char*)&pc, 2);
	stream.read((char*)&sp, 2);

	//TODO deserialize IME, isHalted
}

void CPU::Save(std::ofstream& stream) const {
	stream.write((const char*)registers, 8);
	stream.write((const char*)&pc, 2);
	stream.write((const char*)&sp, 2);

	//TODO serialize IME, isHalted
}

void CPU::CallOpCode(u8 opCode) {
    if (log) {
        Logger::instance->log(pc - 1);
        Logger::instance->log(opCodeNames[opCode]);
    }
	switch (opCode) {
	case 0x00: NOP(); break;
	case 0x01: LDr16n16(CPU16BitReg::bc); break;
	case 0x02: LDr16A(CPU16BitReg::bc); break;
	case 0x03: INCr16(CPU16BitReg::bc); break;
	case 0x04: INCr8(CPU8BitReg::b); break;
	case 0x05: DECr8(CPU8BitReg::b); break;
	case 0x06: LDr8n8(CPU8BitReg::b); break;
	case 0x07: RLCA(); break;
	case 0x08: LDn16SP(); break;
	case 0x09: ADDHLr16(CPU16BitReg::bc); break;
	case 0x0A: LDAr16(CPU16BitReg::bc); break;
	case 0x0B: DECr16(CPU16BitReg::bc); break;
	case 0x0C: INCr8(CPU8BitReg::c); break;
	case 0x0D: DECr8(CPU8BitReg::c); break;
	case 0x0E: LDr8n8(CPU8BitReg::c); break;
	case 0x0F: RRCA(); break;

	case 0x10: STOP(); break;
	case 0x11: LDr16n16(CPU16BitReg::de); break;
	case 0x12: LDr16A(CPU16BitReg::de); break;
	case 0x13: INCr16(CPU16BitReg::de); break;
	case 0x14: INCr8(CPU8BitReg::d); break;
	case 0x15: DECr8(CPU8BitReg::d); break;
	case 0x16: LDr8n8(CPU8BitReg::d); break;
	case 0x17: RLA(); break;
	case 0x18: JRe8(); break;
	case 0x19: ADDHLr16(CPU16BitReg::de); break;
	case 0x1A: LDAr16(CPU16BitReg::de); break;
	case 0x1B: DECr16(CPU16BitReg::de); break;
	case 0x1C: INCr8(CPU8BitReg::e); break;
	case 0x1D: DECr8(CPU8BitReg::e); break;
	case 0x1E: LDr8n8(CPU8BitReg::e); break;
	case 0x1F: RRA(); break;

	case 0x20: JRcce8(!HasFlag(FlagBit::Zero)); break;
	case 0x21: LDr16n16(CPU16BitReg::hl); break;
	case 0x22: LDHLincA(); break;
	case 0x23: INCr16(CPU16BitReg::hl); break;
	case 0x24: INCr8(CPU8BitReg::h); break;
	case 0x25: DECr8(CPU8BitReg::h); break;
	case 0x26: LDr8n8(CPU8BitReg::h); break;
	case 0x27: DAA(); break;
	case 0x28: JRcce8(HasFlag(FlagBit::Zero)); break;
	case 0x29: ADDHLr16(CPU16BitReg::hl); break;
	case 0x2A: LDAHLinc(); break;
	case 0x2B: DECr16(CPU16BitReg::hl); break;
	case 0x2C: INCr8(CPU8BitReg::l); break;
	case 0x2D: DECr8(CPU8BitReg::l); break;
	case 0x2E: LDr8n8(CPU8BitReg::l); break;
	case 0x2F: CPL(); break;

	case 0x30: JRcce8(!HasFlag(FlagBit::Carry)); break;
	case 0x31: LDSPn16(); break;
	case 0x32: LDHLdecA(); break;
	case 0x33: INCSP(); break;
	case 0x34: INCHL(); break;
	case 0x35: DECHL(); break;
	case 0x36: LDHLn8(); break;
	case 0x37: SCF(); break;
	case 0x38: JRcce8(HasFlag(FlagBit::Carry)); break;
	case 0x39: ADDHLSP(); break;
	case 0x3A: LDAHLdec(); break;
	case 0x3B: DECSP(); break;
	case 0x3C: INCr8(CPU8BitReg::a); break;
	case 0x3D: DECr8(CPU8BitReg::a); break;
	case 0x3E: LDr8n8(CPU8BitReg::a); break;
	case 0x3F: CCF(); break;

	case 0x40: LDr8r8(CPU8BitReg::b, CPU8BitReg::b); break;
	case 0x41: LDr8r8(CPU8BitReg::b, CPU8BitReg::c); break;
	case 0x42: LDr8r8(CPU8BitReg::b, CPU8BitReg::d); break;
	case 0x43: LDr8r8(CPU8BitReg::b, CPU8BitReg::e); break;
	case 0x44: LDr8r8(CPU8BitReg::b, CPU8BitReg::h); break;
	case 0x45: LDr8r8(CPU8BitReg::b, CPU8BitReg::l); break;
	case 0x46: LDr8HL(CPU8BitReg::b); break;
	case 0x47: LDr8r8(CPU8BitReg::b, CPU8BitReg::a); break;
	case 0x48: LDr8r8(CPU8BitReg::c, CPU8BitReg::b); break;
	case 0x49: LDr8r8(CPU8BitReg::c, CPU8BitReg::c); break;
	case 0x4A: LDr8r8(CPU8BitReg::c, CPU8BitReg::d); break;
	case 0x4B: LDr8r8(CPU8BitReg::c, CPU8BitReg::e); break;
	case 0x4C: LDr8r8(CPU8BitReg::c, CPU8BitReg::h); break;
	case 0x4D: LDr8r8(CPU8BitReg::c, CPU8BitReg::l); break;
	case 0x4E: LDr8HL(CPU8BitReg::c); break;
	case 0x4F: LDr8r8(CPU8BitReg::c, CPU8BitReg::a); break;

	case 0x50: LDr8r8(CPU8BitReg::d, CPU8BitReg::b); break;
	case 0x51: LDr8r8(CPU8BitReg::d, CPU8BitReg::c); break;
	case 0x52: LDr8r8(CPU8BitReg::d, CPU8BitReg::d); break;
	case 0x53: LDr8r8(CPU8BitReg::d, CPU8BitReg::e); break;
	case 0x54: LDr8r8(CPU8BitReg::d, CPU8BitReg::h); break;
	case 0x55: LDr8r8(CPU8BitReg::d, CPU8BitReg::l); break;
	case 0x56: LDr8HL(CPU8BitReg::d); break;
	case 0x57: LDr8r8(CPU8BitReg::d, CPU8BitReg::a); break;
	case 0x58: LDr8r8(CPU8BitReg::e, CPU8BitReg::b); break;
	case 0x59: LDr8r8(CPU8BitReg::e, CPU8BitReg::c); break;
	case 0x5A: LDr8r8(CPU8BitReg::e, CPU8BitReg::d); break;
	case 0x5B: LDr8r8(CPU8BitReg::e, CPU8BitReg::e); break;
	case 0x5C: LDr8r8(CPU8BitReg::e, CPU8BitReg::h); break;
	case 0x5D: LDr8r8(CPU8BitReg::e, CPU8BitReg::l); break;
	case 0x5E: LDr8HL(CPU8BitReg::e); break;
	case 0x5F: LDr8r8(CPU8BitReg::e, CPU8BitReg::a); break;

	case 0x60: LDr8r8(CPU8BitReg::h, CPU8BitReg::b); break;
	case 0x61: LDr8r8(CPU8BitReg::h, CPU8BitReg::c); break;
	case 0x62: LDr8r8(CPU8BitReg::h, CPU8BitReg::d); break;
	case 0x63: LDr8r8(CPU8BitReg::h, CPU8BitReg::e); break;
	case 0x64: LDr8r8(CPU8BitReg::h, CPU8BitReg::h); break;
	case 0x65: LDr8r8(CPU8BitReg::h, CPU8BitReg::l); break;
	case 0x66: LDr8HL(CPU8BitReg::h); break;
	case 0x67: LDr8r8(CPU8BitReg::h, CPU8BitReg::a); break;
	case 0x68: LDr8r8(CPU8BitReg::l, CPU8BitReg::b); break;
	case 0x69: LDr8r8(CPU8BitReg::l, CPU8BitReg::c); break;
	case 0x6A: LDr8r8(CPU8BitReg::l, CPU8BitReg::d); break;
	case 0x6B: LDr8r8(CPU8BitReg::l, CPU8BitReg::e); break;
	case 0x6C: LDr8r8(CPU8BitReg::l, CPU8BitReg::h); break;
	case 0x6D: LDr8r8(CPU8BitReg::l, CPU8BitReg::l); break;
	case 0x6E: LDr8HL(CPU8BitReg::l); break;
	case 0x6F: LDr8r8(CPU8BitReg::l, CPU8BitReg::a); break;

	case 0x70: LDHLr8(CPU8BitReg::b); break;
	case 0x71: LDHLr8(CPU8BitReg::c); break;
	case 0x72: LDHLr8(CPU8BitReg::d); break;
	case 0x73: LDHLr8(CPU8BitReg::e); break;
	case 0x74: LDHLr8(CPU8BitReg::h); break;
	case 0x75: LDHLr8(CPU8BitReg::l); break;
	case 0x76: HALT(); break;
	case 0x77: LDHLr8(CPU8BitReg::a); break;
	case 0x78: LDr8r8(CPU8BitReg::a, CPU8BitReg::b); break;
	case 0x79: LDr8r8(CPU8BitReg::a, CPU8BitReg::c); break;
	case 0x7A: LDr8r8(CPU8BitReg::a, CPU8BitReg::d); break;
	case 0x7B: LDr8r8(CPU8BitReg::a, CPU8BitReg::e); break;
	case 0x7C: LDr8r8(CPU8BitReg::a, CPU8BitReg::h); break;
	case 0x7D: LDr8r8(CPU8BitReg::a, CPU8BitReg::l); break;
	case 0x7E: LDr8HL(CPU8BitReg::a); break;
	case 0x7F: LDr8r8(CPU8BitReg::a, CPU8BitReg::a); break;

	case 0x80: ADDAr8(CPU8BitReg::b); break;
	case 0x81: ADDAr8(CPU8BitReg::c); break;
	case 0x82: ADDAr8(CPU8BitReg::d); break;
	case 0x83: ADDAr8(CPU8BitReg::e); break;
	case 0x84: ADDAr8(CPU8BitReg::h); break;
	case 0x85: ADDAr8(CPU8BitReg::l); break;
	case 0x86: ADDAHL(); break;
	case 0x87: ADDAr8(CPU8BitReg::a); break;
	case 0x88: ADCAr8(CPU8BitReg::b); break;
	case 0x89: ADCAr8(CPU8BitReg::c); break;
	case 0x8A: ADCAr8(CPU8BitReg::d); break;
	case 0x8B: ADCAr8(CPU8BitReg::e); break;
	case 0x8C: ADCAr8(CPU8BitReg::h); break;
	case 0x8D: ADCAr8(CPU8BitReg::l); break;
	case 0x8E: ADCAHL(); break;
	case 0x8F: ADCAr8(CPU8BitReg::a); break;

	case 0x90: SUBAr8(CPU8BitReg::b); break;
	case 0x91: SUBAr8(CPU8BitReg::c); break;
	case 0x92: SUBAr8(CPU8BitReg::d); break;
	case 0x93: SUBAr8(CPU8BitReg::e); break;
	case 0x94: SUBAr8(CPU8BitReg::h); break;
	case 0x95: SUBAr8(CPU8BitReg::l); break;
	case 0x96: SUBAHL(); break;
	case 0x97: SUBAr8(CPU8BitReg::a); break;
	case 0x98: SBCAr8(CPU8BitReg::b); break;
	case 0x99: SBCAr8(CPU8BitReg::c); break;
	case 0x9A: SBCAr8(CPU8BitReg::d); break;
	case 0x9B: SBCAr8(CPU8BitReg::e); break;
	case 0x9C: SBCAr8(CPU8BitReg::h); break;
	case 0x9D: SBCAr8(CPU8BitReg::l); break;
	case 0x9E: SBCAHL(); break;
	case 0x9F: SBCAr8(CPU8BitReg::a); break;

	case 0xA0: ANDAr8(CPU8BitReg::b); break;
	case 0xA1: ANDAr8(CPU8BitReg::c); break;
	case 0xA2: ANDAr8(CPU8BitReg::d); break;
	case 0xA3: ANDAr8(CPU8BitReg::e); break;
	case 0xA4: ANDAr8(CPU8BitReg::h); break;
	case 0xA5: ANDAr8(CPU8BitReg::l); break;
	case 0xA6: ANDAHL(); break;
	case 0xA7: ANDAr8(CPU8BitReg::a); break;
	case 0xA8: XORAr8(CPU8BitReg::b); break;
	case 0xA9: XORAr8(CPU8BitReg::c); break;
	case 0xAA: XORAr8(CPU8BitReg::d); break;
	case 0xAB: XORAr8(CPU8BitReg::e); break;
	case 0xAC: XORAr8(CPU8BitReg::h); break;
	case 0xAD: XORAr8(CPU8BitReg::l); break;
	case 0xAE: XORAHL(); break;
	case 0xAF: XORAr8(CPU8BitReg::a); break;

	case 0xB0: ORAr8(CPU8BitReg::b); break;
	case 0xB1: ORAr8(CPU8BitReg::c); break;
	case 0xB2: ORAr8(CPU8BitReg::d); break;
	case 0xB3: ORAr8(CPU8BitReg::e); break;
	case 0xB4: ORAr8(CPU8BitReg::h); break;
	case 0xB5: ORAr8(CPU8BitReg::l); break;
	case 0xB6: ORAHL(); break;
	case 0xB7: ORAr8(CPU8BitReg::a); break;
	case 0xB8: CPAr8(CPU8BitReg::b); break;
	case 0xB9: CPAr8(CPU8BitReg::c); break;
	case 0xBA: CPAr8(CPU8BitReg::d); break;
	case 0xBB: CPAr8(CPU8BitReg::e); break;
	case 0xBC: CPAr8(CPU8BitReg::h); break;
	case 0xBD: CPAr8(CPU8BitReg::l); break;
	case 0xBE: CPAHL(); break;
	case 0xBF: CPAr8(CPU8BitReg::a); break;

	case 0xC0: RETcc(!HasFlag(FlagBit::Zero)); break;
	case 0xC1: POPr16(CPU16BitReg::bc); break;
	case 0xC2: JPccn16(!HasFlag(FlagBit::Zero)); break;
	case 0xC3: JPn16(); break;
	case 0xC4: CALLccn16(!HasFlag(FlagBit::Zero)); break;
	case 0xC5: PUSHr16(CPU16BitReg::bc); break;
	case 0xC6: ADDAn8(); break;
	case 0xC7: RSTvec(0x00); break;
	case 0xC8: RETcc(HasFlag(FlagBit::Zero)); break;
	case 0xC9: RET(); break;
	case 0xCA: JPccn16(HasFlag(FlagBit::Zero)); break;
	case 0xCB: /*CB!*/ break;
	case 0xCC: CALLccn16(HasFlag(FlagBit::Zero)); break;
	case 0xCD: CALLn16(); break;
	case 0xCE: ADCAn8(); break;
	case 0xCF: RSTvec(0x08); break;

	case 0xD0: RETcc(!HasFlag(FlagBit::Carry)); break;
	case 0xD1: POPr16(CPU16BitReg::de); break;
	case 0xD2: JPccn16(!HasFlag(FlagBit::Carry)); break;
	case 0xD3: /**/ break;
	case 0xD4: CALLccn16(!HasFlag(FlagBit::Carry)); break;
	case 0xD5: PUSHr16(CPU16BitReg::de); break;
	case 0xD6: SUBAn8(); break;
	case 0xD7: RSTvec(0x10); break;
	case 0xD8: RETcc(HasFlag(FlagBit::Carry)); break;
	case 0xD9: RETI(); break;
	case 0xDA: JPccn16(HasFlag(FlagBit::Carry)); break;
	case 0xDB: /**/ break;
	case 0xDC: CALLccn16(HasFlag(FlagBit::Carry)); break;
	case 0xDD: /**/ break;
	case 0xDE: SBCAn8(); break;
	case 0xDF: RSTvec(0x18); break;

	case 0xE0: LDHn8A(); break;
	case 0xE1: POPr16(CPU16BitReg::hl); break;
	case 0xE2: LDHCA(); break;
	case 0xE3: /**/ break;
	case 0xE4: /**/ break;
	case 0xE5: PUSHr16(CPU16BitReg::hl); break;
	case 0xE6: ANDAn8(); break;
	case 0xE7: RSTvec(0x20); break;
	case 0xE8: ADDSPe8(); break;
	case 0xE9: JPHL(); break;
	case 0xEA: LDn16A(); break;
	case 0xEB: /**/ break;
	case 0xEC: /**/ break;
	case 0xED: /**/ break;
	case 0xEE: XORAn8(); break;
	case 0xEF: RSTvec(0x28); break;

	case 0xF0: LDHAn8(); break;
	case 0xF1: POPr16(CPU16BitReg::af); break;
	case 0xF2: LDHAC(); break;
	case 0xF3: DI(); break;
	case 0xF4: /**/ break;
	case 0xF5: PUSHr16(CPU16BitReg::af); break;
	case 0xF6: ORAn8(); break;
	case 0xF7: RSTvec(0x30); break;
	case 0xF8: LDHLSPe8(); break;
	case 0xF9: LDSPHL(); break;
	case 0xFA: LDAn16(); break;
	case 0xFB: EI(); break;
	case 0xFC: /**/ break;
	case 0xFD: /**/ break;
	case 0xFE: CPAn8(); break;
	case 0xFF: RSTvec(0x38); break;
	}
}

void CPU::CallCBOpCode(u8 opCode) {
    if (log) {
        Logger::instance->log(pc - 1);
        Logger::instance->log(cbOpCodeNames[opCode]);
    }
	switch (opCode)
	{
	case 0x00: RLCr8(CPU8BitReg::b); break;
	case 0x01: RLCr8(CPU8BitReg::c); break;
	case 0x02: RLCr8(CPU8BitReg::d); break;
	case 0x03: RLCr8(CPU8BitReg::e); break;
	case 0x04: RLCr8(CPU8BitReg::h); break;
	case 0x05: RLCr8(CPU8BitReg::l); break;
	case 0x06: RLCHL(); break;
	case 0x07: RLCr8(CPU8BitReg::a); break;
	case 0x08: RRCr8(CPU8BitReg::b); break;
	case 0x09: RRCr8(CPU8BitReg::c); break;
	case 0x0A: RRCr8(CPU8BitReg::d); break;
	case 0x0B: RRCr8(CPU8BitReg::e); break;
	case 0x0C: RRCr8(CPU8BitReg::h); break;
	case 0x0D: RRCr8(CPU8BitReg::l); break;
	case 0x0E: RRCHL(); break;
	case 0x0F: RRCr8(CPU8BitReg::a); break;

	case 0x10: RLr8(CPU8BitReg::b); break;
	case 0x11: RLr8(CPU8BitReg::c); break;
	case 0x12: RLr8(CPU8BitReg::d); break;
	case 0x13: RLr8(CPU8BitReg::e); break;
	case 0x14: RLr8(CPU8BitReg::h); break;
	case 0x15: RLr8(CPU8BitReg::l); break;
	case 0x16: RLHL(); break;
	case 0x17: RLr8(CPU8BitReg::a); break;
	case 0x18: RRr8(CPU8BitReg::b); break;
	case 0x19: RRr8(CPU8BitReg::c); break;
	case 0x1A: RRr8(CPU8BitReg::d); break;
	case 0x1B: RRr8(CPU8BitReg::e); break;
	case 0x1C: RRr8(CPU8BitReg::h); break;
	case 0x1D: RRr8(CPU8BitReg::l); break;
	case 0x1E: RRHL(); break;
	case 0x1F: RRr8(CPU8BitReg::a); break;

	case 0x20: SLAr8(CPU8BitReg::b); break;
	case 0x21: SLAr8(CPU8BitReg::c); break;
	case 0x22: SLAr8(CPU8BitReg::d); break;
	case 0x23: SLAr8(CPU8BitReg::e); break;
	case 0x24: SLAr8(CPU8BitReg::h); break;
	case 0x25: SLAr8(CPU8BitReg::l); break;
	case 0x26: SLAHL(); break;
	case 0x27: SLAr8(CPU8BitReg::a); break;
	case 0x28: SRAr8(CPU8BitReg::b); break;
	case 0x29: SRAr8(CPU8BitReg::c); break;
	case 0x2A: SRAr8(CPU8BitReg::d); break;
	case 0x2B: SRAr8(CPU8BitReg::e); break;
	case 0x2C: SRAr8(CPU8BitReg::h); break;
	case 0x2D: SRAr8(CPU8BitReg::l); break;
	case 0x2E: SRAHL(); break;
	case 0x2F: SRAr8(CPU8BitReg::a); break;

	case 0x30: SWAPr8(CPU8BitReg::b); break;
	case 0x31: SWAPr8(CPU8BitReg::c); break;
	case 0x32: SWAPr8(CPU8BitReg::d); break;
	case 0x33: SWAPr8(CPU8BitReg::e); break;
	case 0x34: SWAPr8(CPU8BitReg::h); break;
	case 0x35: SWAPr8(CPU8BitReg::l); break;
	case 0x36: SWAPHL(); break;
	case 0x37: SWAPr8(CPU8BitReg::a); break;
	case 0x38: SRLr8(CPU8BitReg::b); break;
	case 0x39: SRLr8(CPU8BitReg::c); break;
	case 0x3A: SRLr8(CPU8BitReg::d); break;
	case 0x3B: SRLr8(CPU8BitReg::e); break;
	case 0x3C: SRLr8(CPU8BitReg::h); break;
	case 0x3D: SRLr8(CPU8BitReg::l); break;
	case 0x3E: SRLHL(); break;
	case 0x3F: SRLr8(CPU8BitReg::a); break;

	case 0x40: BITu3r8(0, CPU8BitReg::b); break;
	case 0x41: BITu3r8(0, CPU8BitReg::c); break;
	case 0x42: BITu3r8(0, CPU8BitReg::d); break;
	case 0x43: BITu3r8(0, CPU8BitReg::e); break;
	case 0x44: BITu3r8(0, CPU8BitReg::h); break;
	case 0x45: BITu3r8(0, CPU8BitReg::l); break;
	case 0x46: BITu3HL(0); break;
	case 0x47: BITu3r8(0, CPU8BitReg::a); break;
	case 0x48: BITu3r8(1, CPU8BitReg::b); break;
	case 0x49: BITu3r8(1, CPU8BitReg::c); break;
	case 0x4A: BITu3r8(1, CPU8BitReg::d); break;
	case 0x4B: BITu3r8(1, CPU8BitReg::e); break;
	case 0x4C: BITu3r8(1, CPU8BitReg::h); break;
	case 0x4D: BITu3r8(1, CPU8BitReg::l); break;
	case 0x4E: BITu3HL(1); break;
	case 0x4F: BITu3r8(1, CPU8BitReg::a); break;

	case 0x50: BITu3r8(2, CPU8BitReg::b); break;
	case 0x51: BITu3r8(2, CPU8BitReg::c); break;
	case 0x52: BITu3r8(2, CPU8BitReg::d); break;
	case 0x53: BITu3r8(2, CPU8BitReg::e); break;
	case 0x54: BITu3r8(2, CPU8BitReg::h); break;
	case 0x55: BITu3r8(2, CPU8BitReg::l); break;
	case 0x56: BITu3HL(2); break;
	case 0x57: BITu3r8(2, CPU8BitReg::a); break;
	case 0x58: BITu3r8(3, CPU8BitReg::b); break;
	case 0x59: BITu3r8(3, CPU8BitReg::c); break;
	case 0x5A: BITu3r8(3, CPU8BitReg::d); break;
	case 0x5B: BITu3r8(3, CPU8BitReg::e); break;
	case 0x5C: BITu3r8(3, CPU8BitReg::h); break;
	case 0x5D: BITu3r8(3, CPU8BitReg::l); break;
	case 0x5E: BITu3HL(3); break;
	case 0x5F: BITu3r8(3, CPU8BitReg::a); break;

	case 0x60: BITu3r8(4, CPU8BitReg::b); break;
	case 0x61: BITu3r8(4, CPU8BitReg::c); break;
	case 0x62: BITu3r8(4, CPU8BitReg::d); break;
	case 0x63: BITu3r8(4, CPU8BitReg::e); break;
	case 0x64: BITu3r8(4, CPU8BitReg::h); break;
	case 0x65: BITu3r8(4, CPU8BitReg::l); break;
	case 0x66: BITu3HL(4); break;
	case 0x67: BITu3r8(4, CPU8BitReg::a); break;
	case 0x68: BITu3r8(5, CPU8BitReg::b); break;
	case 0x69: BITu3r8(5, CPU8BitReg::c); break;
	case 0x6A: BITu3r8(5, CPU8BitReg::d); break;
	case 0x6B: BITu3r8(5, CPU8BitReg::e); break;
	case 0x6C: BITu3r8(5, CPU8BitReg::h); break;
	case 0x6D: BITu3r8(5, CPU8BitReg::l); break;
	case 0x6E: BITu3HL(5); break;
	case 0x6F: BITu3r8(5, CPU8BitReg::a); break;

	case 0x70: BITu3r8(6, CPU8BitReg::b); break;
	case 0x71: BITu3r8(6, CPU8BitReg::c); break;
	case 0x72: BITu3r8(6, CPU8BitReg::d); break;
	case 0x73: BITu3r8(6, CPU8BitReg::e); break;
	case 0x74: BITu3r8(6, CPU8BitReg::h); break;
	case 0x75: BITu3r8(6, CPU8BitReg::l); break;
	case 0x76: BITu3HL(6); break;
	case 0x77: BITu3r8(6, CPU8BitReg::a); break;
	case 0x78: BITu3r8(7, CPU8BitReg::b); break;
	case 0x79: BITu3r8(7, CPU8BitReg::c); break;
	case 0x7A: BITu3r8(7, CPU8BitReg::d); break;
	case 0x7B: BITu3r8(7, CPU8BitReg::e); break;
	case 0x7C: BITu3r8(7, CPU8BitReg::h); break;
	case 0x7D: BITu3r8(7, CPU8BitReg::l); break;
	case 0x7E: BITu3HL(7); break;
	case 0x7F: BITu3r8(7, CPU8BitReg::a); break;

	case 0x80: RESu3r8(0, CPU8BitReg::b); break;
	case 0x81: RESu3r8(0, CPU8BitReg::c); break;
	case 0x82: RESu3r8(0, CPU8BitReg::d); break;
	case 0x83: RESu3r8(0, CPU8BitReg::e); break;
	case 0x84: RESu3r8(0, CPU8BitReg::h); break;
	case 0x85: RESu3r8(0, CPU8BitReg::l); break;
	case 0x86: RESu3HL(0); break;
	case 0x87: RESu3r8(0, CPU8BitReg::a); break;
	case 0x88: RESu3r8(1, CPU8BitReg::b); break;
	case 0x89: RESu3r8(1, CPU8BitReg::c); break;
	case 0x8A: RESu3r8(1, CPU8BitReg::d); break;
	case 0x8B: RESu3r8(1, CPU8BitReg::e); break;
	case 0x8C: RESu3r8(1, CPU8BitReg::h); break;
	case 0x8D: RESu3r8(1, CPU8BitReg::l); break;
	case 0x8E: RESu3HL(1); break;
	case 0x8F: RESu3r8(1, CPU8BitReg::a); break;

	case 0x90: RESu3r8(2, CPU8BitReg::b); break;
	case 0x91: RESu3r8(2, CPU8BitReg::c); break;
	case 0x92: RESu3r8(2, CPU8BitReg::d); break;
	case 0x93: RESu3r8(2, CPU8BitReg::e); break;
	case 0x94: RESu3r8(2, CPU8BitReg::h); break;
	case 0x95: RESu3r8(2, CPU8BitReg::l); break;
	case 0x96: RESu3HL(2); break;
	case 0x97: RESu3r8(2, CPU8BitReg::a); break;
	case 0x98: RESu3r8(3, CPU8BitReg::b); break;
	case 0x99: RESu3r8(3, CPU8BitReg::c); break;
	case 0x9A: RESu3r8(3, CPU8BitReg::d); break;
	case 0x9B: RESu3r8(3, CPU8BitReg::e); break;
	case 0x9C: RESu3r8(3, CPU8BitReg::h); break;
	case 0x9D: RESu3r8(3, CPU8BitReg::l); break;
	case 0x9E: RESu3HL(3); break;
	case 0x9F: RESu3r8(3, CPU8BitReg::a); break;

	case 0xA0: RESu3r8(4, CPU8BitReg::b); break;
	case 0xA1: RESu3r8(4, CPU8BitReg::c); break;
	case 0xA2: RESu3r8(4, CPU8BitReg::d); break;
	case 0xA3: RESu3r8(4, CPU8BitReg::e); break;
	case 0xA4: RESu3r8(4, CPU8BitReg::h); break;
	case 0xA5: RESu3r8(4, CPU8BitReg::l); break;
	case 0xA6: RESu3HL(4); break;
	case 0xA7: RESu3r8(4, CPU8BitReg::a); break;
	case 0xA8: RESu3r8(5, CPU8BitReg::b); break;
	case 0xA9: RESu3r8(5, CPU8BitReg::c); break;
	case 0xAA: RESu3r8(5, CPU8BitReg::d); break;
	case 0xAB: RESu3r8(5, CPU8BitReg::e); break;
	case 0xAC: RESu3r8(5, CPU8BitReg::h); break;
	case 0xAD: RESu3r8(5, CPU8BitReg::l); break;
	case 0xAE: RESu3HL(5); break;
	case 0xAF: RESu3r8(5, CPU8BitReg::a); break;

	case 0xB0: RESu3r8(6, CPU8BitReg::b); break;
	case 0xB1: RESu3r8(6, CPU8BitReg::c); break;
	case 0xB2: RESu3r8(6, CPU8BitReg::d); break;
	case 0xB3: RESu3r8(6, CPU8BitReg::e); break;
	case 0xB4: RESu3r8(6, CPU8BitReg::h); break;
	case 0xB5: RESu3r8(6, CPU8BitReg::l); break;
	case 0xB6: RESu3HL(6); break;
	case 0xB7: RESu3r8(6, CPU8BitReg::a); break;
	case 0xB8: RESu3r8(7, CPU8BitReg::b); break;
	case 0xB9: RESu3r8(7, CPU8BitReg::c); break;
	case 0xBA: RESu3r8(7, CPU8BitReg::d); break;
	case 0xBB: RESu3r8(7, CPU8BitReg::e); break;
	case 0xBC: RESu3r8(7, CPU8BitReg::h); break;
	case 0xBD: RESu3r8(7, CPU8BitReg::l); break;
	case 0xBE: RESu3HL(7); break;
	case 0xBF: RESu3r8(7, CPU8BitReg::a); break;

	case 0xC0: SETu3r8(0, CPU8BitReg::b); break;
	case 0xC1: SETu3r8(0, CPU8BitReg::c); break;
	case 0xC2: SETu3r8(0, CPU8BitReg::d); break;
	case 0xC3: SETu3r8(0, CPU8BitReg::e); break;
	case 0xC4: SETu3r8(0, CPU8BitReg::h); break;
	case 0xC5: SETu3r8(0, CPU8BitReg::l); break;
	case 0xC6: SETu3HL(0); break;
	case 0xC7: SETu3r8(0, CPU8BitReg::a); break;
	case 0xC8: SETu3r8(1, CPU8BitReg::b); break;
	case 0xC9: SETu3r8(1, CPU8BitReg::c); break;
	case 0xCA: SETu3r8(1, CPU8BitReg::d); break;
	case 0xCB: SETu3r8(1, CPU8BitReg::e); break;
	case 0xCC: SETu3r8(1, CPU8BitReg::h); break;
	case 0xCD: SETu3r8(1, CPU8BitReg::l); break;
	case 0xCE: SETu3HL(1); break;
	case 0xCF: SETu3r8(1, CPU8BitReg::a); break;

	case 0xD0: SETu3r8(2, CPU8BitReg::b); break;
	case 0xD1: SETu3r8(2, CPU8BitReg::c); break;
	case 0xD2: SETu3r8(2, CPU8BitReg::d); break;
	case 0xD3: SETu3r8(2, CPU8BitReg::e); break;
	case 0xD4: SETu3r8(2, CPU8BitReg::h); break;
	case 0xD5: SETu3r8(2, CPU8BitReg::l); break;
	case 0xD6: SETu3HL(2); break;
	case 0xD7: SETu3r8(2, CPU8BitReg::a); break;
	case 0xD8: SETu3r8(3, CPU8BitReg::b); break;
	case 0xD9: SETu3r8(3, CPU8BitReg::c); break;
	case 0xDA: SETu3r8(3, CPU8BitReg::d); break;
	case 0xDB: SETu3r8(3, CPU8BitReg::e); break;
	case 0xDC: SETu3r8(3, CPU8BitReg::h); break;
	case 0xDD: SETu3r8(3, CPU8BitReg::l); break;
	case 0xDE: SETu3HL(3); break;
	case 0xDF: SETu3r8(3, CPU8BitReg::a); break;

	case 0xE0: SETu3r8(4, CPU8BitReg::b); break;
	case 0xE1: SETu3r8(4, CPU8BitReg::c); break;
	case 0xE2: SETu3r8(4, CPU8BitReg::d); break;
	case 0xE3: SETu3r8(4, CPU8BitReg::e); break;
	case 0xE4: SETu3r8(4, CPU8BitReg::h); break;
	case 0xE5: SETu3r8(4, CPU8BitReg::l); break;
	case 0xE6: SETu3HL(4); break;
	case 0xE7: SETu3r8(4, CPU8BitReg::a); break;
	case 0xE8: SETu3r8(5, CPU8BitReg::b); break;
	case 0xE9: SETu3r8(5, CPU8BitReg::c); break;
	case 0xEA: SETu3r8(5, CPU8BitReg::d); break;
	case 0xEB: SETu3r8(5, CPU8BitReg::e); break;
	case 0xEC: SETu3r8(5, CPU8BitReg::h); break;
	case 0xED: SETu3r8(5, CPU8BitReg::l); break;
	case 0xEE: SETu3HL(5); break;
	case 0xEF: SETu3r8(5, CPU8BitReg::a); break;

	case 0xF0: SETu3r8(6, CPU8BitReg::b); break;
	case 0xF1: SETu3r8(6, CPU8BitReg::c); break;
	case 0xF2: SETu3r8(6, CPU8BitReg::d); break;
	case 0xF3: SETu3r8(6, CPU8BitReg::e); break;
	case 0xF4: SETu3r8(6, CPU8BitReg::h); break;
	case 0xF5: SETu3r8(6, CPU8BitReg::l); break;
	case 0xF6: SETu3HL(6); break;
	case 0xF7: SETu3r8(6, CPU8BitReg::a); break;
	case 0xF8: SETu3r8(7, CPU8BitReg::b); break;
	case 0xF9: SETu3r8(7, CPU8BitReg::c); break;
	case 0xFA: SETu3r8(7, CPU8BitReg::d); break;
	case 0xFB: SETu3r8(7, CPU8BitReg::e); break;
	case 0xFC: SETu3r8(7, CPU8BitReg::h); break;
	case 0xFD: SETu3r8(7, CPU8BitReg::l); break;
	case 0xFE: SETu3HL(7); break;
	case 0xFF: SETu3r8(7, CPU8BitReg::a); break;
	}
}

void CPU::ADCA(u8 value) {
	u8 current = ReadAcc();
	u8 result = current + value;

    if (log) Logger::instance->log("A," + Logger::instance->u8ToHex(value), reg8ToString(CPU8BitReg::a));

	// save carry state before updating
	bool addCarry = HasFlag(FlagBit::Carry);

	bool carrySet = UpdateCarryFlag(current, result, true);
	bool halfCarrySet = UpdateHalfCarryFlag(current, result, true);

	if (addCarry) {
		// update "current" to properly detect carry when adding 1
		current = result;
		result += 1;
		// only update carry if it wasn't set by the previous sum
		if (!carrySet)
			UpdateCarryFlag(current, result, true);
		if (!halfCarrySet)
			UpdateHalfCarryFlag(current, result, true);
	}

	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
}

void CPU::SBCA(u8 value) {
	u8 current = ReadAcc();
	u8 result = current - value;

    if (log) Logger::instance->log("A," + Logger::instance->u8ToHex(value), reg8ToString(CPU8BitReg::a));

	// save carry state before updating
	bool subCarry = HasFlag(FlagBit::Carry);

	bool carrySet = UpdateCarryFlag(current, result, false);
	bool halfCarrySet = UpdateHalfCarryFlag(current, result, false);

	if (subCarry) {
		// update "current" to properly detect carry when substracting 1
		current = result;
		result -= 1;
		// only update carry if it wasn't set by the previous sub
		if (!carrySet)
			UpdateCarryFlag(current, result, false);
		if (!halfCarrySet)
			UpdateHalfCarryFlag(current, result, false);
	}

	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, true);
}

void CPU::ADCAr8(CPU8BitReg reg) {
	ADCA(Read8BitReg(reg));
}

void CPU::ADCAHL() {
	ADCA(ReadMemory(ReadHL()));
}

void CPU::ADCAn8() {
	ADCA(ReadAtPC());
}

void CPU::ADDAr8(CPU8BitReg reg) {
	u8 current = ReadAcc();
	u8 result = current + Read8BitReg(reg);

    if (log) Logger::instance->log("A," + r8Names[reg], reg8ToString(CPU8BitReg::a) + " _ " + reg8ToString(reg));

	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	UpdateHalfCarryFlag(current, result, true);
	UpdateCarryFlag(current, result, true);
}

void CPU::ADDAHL() {
	u8 mem = ReadMemory(ReadHL());
	u8 current = ReadAcc();

    if (log) Logger::instance->log("A,[HL]", reg8ToString(CPU8BitReg::a) + " _ " + reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(mem));

	u8 result = current + mem;
	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	UpdateHalfCarryFlag(current, result, true);
	UpdateCarryFlag(current, result, true);
}

void CPU::ADDAn8() {
	u8 constant = ReadAtPC();
	u8 current = ReadAcc();

    if (log) Logger::instance->log("A,$" + Logger::instance->u8ToHex(constant), reg8ToString(CPU8BitReg::a));

	u8 result = current + constant;
	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	UpdateHalfCarryFlag(current, result, true);
	UpdateCarryFlag(current, result, true);
}

void CPU::ADDHLr16(CPU16BitReg reg) {
	u16 rCurrent = ReadHL();

    if (log) Logger::instance->log("HL," + r16Names[reg], reg16ToString(CPU16BitReg::hl) + " _ " + reg16ToString(reg));

	u16 result = rCurrent + Read16BitReg(reg);
	WriteHL(result);
	lastOpCycles++;

	SetFlag(FlagBit::Negative, false);
	UpdateHalfCarryFlag(rCurrent, result, true);
	UpdateCarryFlag(rCurrent, result, true);
}

void CPU::ADDHLSP() {
	u16 current = ReadHL();

    if (log) Logger::instance->log("HL,SP", reg16ToString(CPU16BitReg::hl) + " _ SP = " + Logger::instance->u16ToHex(sp));

	u16 result = current + sp;
	WriteHL(result);
	lastOpCycles++;

	SetFlag(FlagBit::Negative, false);
	UpdateHalfCarryFlag(current, result, true);
	UpdateCarryFlag(current, result, true);
}

void CPU::ADDSPe8() {
	s8 offset = (s8)ReadAtPC();
	u16 current = sp;

    if (log) Logger::instance->log("SP," + Logger::instance->s8ToString(offset), "SP = " + Logger::instance->u16ToHex(sp));

	u16 result = sp + offset;
	sp = result;
	lastOpCycles += 2;
	
	SetFlag(FlagBit::Zero, false);
	SetFlag(FlagBit::Negative, false);
	// Carry and HalfCarry from lsb to pass blargg tests https://github.com/Drenn1/GameYob/issues/15
	UpdateHalfCarryFlag((u8)current, (u8)result, true);
	UpdateCarryFlag((u8)current, (u8)result, true);
}

void CPU::ANDAr8(CPU8BitReg reg) {
	u8 result = ReadAcc() & Read8BitReg(reg);
    if (log) Logger::instance->log("A," + r8Names[reg], reg8ToString(CPU8BitReg::a) + " _ " + reg8ToString(reg));
	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, true);
	SetFlag(FlagBit::Carry, false);
}

void CPU::ANDAHL() {
	u8 mem = ReadMemory(ReadHL());
    if (log) Logger::instance->log("A,[HL]", reg8ToString(CPU8BitReg::a) + " _ " + reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(mem));

	u8 result = ReadAcc() & mem;
	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, true);
	SetFlag(FlagBit::Carry, false);
}

void CPU::ANDAn8() {
	u8 constant = ReadAtPC();

    if (log) Logger::instance->log("A," + Logger::instance->u8ToHex(constant), reg8ToString(CPU8BitReg::a));

	u8 result = ReadAcc() & constant;
	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, true);
	SetFlag(FlagBit::Carry, false);
}

void CPU::BITu3r8(u8 bit, CPU8BitReg reg) {
    if (log) Logger::instance->log("", reg8ToString(reg));

	u8 result = Read8BitReg(reg) & (1 << bit);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, true);
}

void CPU::BITu3HL(u8 bit) {
	u8 mem = ReadMemory(ReadHL());

    if (log) Logger::instance->log("", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(mem));

	u8 result = mem & (1 << bit);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, true);
}

void CPU::CALLn16() {
	u16 address = ReadAtPC() | (ReadAtPC() << 8);
	Push16(pc);
	pc = address;
	lastOpCycles++;
	//no flags affected

    if (log) Logger::instance->log("" + Logger::instance->u16ToHex(address),"");
}

void CPU::CALLccn16(bool condition) {
	u16 address = ReadAtPC() | (ReadAtPC() << 8);
	if (log) Logger::instance->log("" + Logger::instance->u16ToHex(address), reg8ToString(CPU8BitReg::f));
	if (condition) {
		Push16(pc);
		pc = address;
		lastOpCycles++;
	}
	//no flags affected
}

void CPU::CCF() {
	SetFlag(FlagBit::Carry, !HasFlag(FlagBit::Carry));

	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);

    if (log) Logger::instance->log("", "");
}

void CPU::CPAr8(CPU8BitReg reg) {
	u8 current = ReadAcc();
	u8 result = current - Read8BitReg(reg);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, true);
	UpdateHalfCarryFlag(current, result, false);
	UpdateCarryFlag(current, result, false);

    if (log) Logger::instance->log("A," + r8Names[reg], reg8ToString(CPU8BitReg::a) + " _ " + reg8ToString(reg));
}

void CPU::CPAHL() {
	u8 mem = ReadMemory(ReadHL());
	u8 current = ReadAcc();
	u8 result = current - mem;

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, true);
	UpdateHalfCarryFlag(current, result, false);
	UpdateCarryFlag(current, result, false);

    if (log) Logger::instance->log("A,[HL]", reg8ToString(CPU8BitReg::a) + " _ " + reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(mem));
}

void CPU::CPAn8() {
	if (pc == 0x0ca9)
		int a = 0;
	u8 constant = ReadAtPC();
	u8 current = ReadAcc();
	u8 result = current - constant;

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, true);
	UpdateHalfCarryFlag(current, result, false);
	UpdateCarryFlag(current, result, false);

    if (log) Logger::instance->log("A," + Logger::instance->u8ToHex(constant), reg8ToString(CPU8BitReg::a));
}

void CPU::CPL() {
	WriteAcc(~ReadAcc());

	SetFlag(FlagBit::Negative, true);
	SetFlag(FlagBit::HalfCarry, true);

    if (log) Logger::instance->log("", "");
}

void CPU::DAA() {
	// https://ehaskins.com/2018-01-30%20Z80%20DAA/
	// https://www.reddit.com/r/EmuDev/comments/cdtuyw/gameboy_emulator_fails_blargg_daa_test/
	u8 old = ReadAcc();
	u8 result = old;

	if (HasFlag(FlagBit::Negative)) {
		if (HasFlag(FlagBit::HalfCarry))
			result -= 0x06;
		if (HasFlag(FlagBit::Carry)) {
			result -= 0x60;
			SetFlag(FlagBit::Carry, true);
		}
	} else {
		if ((old & 0x0F) > 0x09 || HasFlag(FlagBit::HalfCarry))
			result += 0x06;
		if (old > 0x99 || HasFlag(FlagBit::Carry)) {
			result += 0x60;
			SetFlag(FlagBit::Carry, true);
		}
	}

    WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::HalfCarry, false);

    if (log) Logger::instance->log("", "");
}

void CPU::DECr8(CPU8BitReg reg) {
    if (log) Logger::instance->log(r8Names[reg], reg8ToString(reg));
	u8 current = Read8BitReg(reg);
	u8 result = current - 1;
	Write8BitReg(reg, result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, true);
	UpdateHalfCarryFlag(current, result, false);
}

void CPU::DECHL() {
    if (log) Logger::instance->log("[HL]", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(ReadHL()));
	u8 current = ReadMemory(ReadHL());
	u8 result = current - 1;
	WriteMemory(ReadHL(), result);
	
	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, true);
	UpdateHalfCarryFlag(current, result, false);
}

void CPU::DECr16(CPU16BitReg reg) {
    if (log) Logger::instance->log(r16Names[reg], reg16ToString(reg));
	u16 current = Read16BitReg(reg);
	u16 result = current - 1;
	Write16BitReg(reg, result);
	lastOpCycles++;
	//no flags affected
}

void CPU::DECSP() {
    if (log) Logger::instance->log("SP", "SP = " + Logger::instance->u16ToHex(sp));
	sp--;
	lastOpCycles++;
	//no flags affected
}

void CPU::DI() {
	interruptService.IME = false;
	//no flags affected
    if (log) Logger::instance->log("", "");
}

void CPU::EI() {
	interruptService.eiDelay = true;
	//no flags affected
    if (log) Logger::instance->log("", "");
}

void CPU::HALT() {
	isHalted = true;
	if (!interruptService.IME && (interruptService.IE & interruptService.IF))
		haltBug = true;
	//no flags affected
    if (log) Logger::instance->log("", "");
}

void CPU::INCr8(CPU8BitReg reg) {
    if (log) Logger::instance->log(r8Names[reg], reg8ToString(reg));

	u8 current = Read8BitReg(reg);
	u8 result = current + 1;
	Write8BitReg(reg, result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	UpdateHalfCarryFlag(current, result, true);
}

void CPU::INCHL() {
    if (log) Logger::instance->log("[HL]", reg16ToString(CPU16BitReg::hl) + "[HL] = " + Logger::instance->u8ToHex(ReadHL()));
	u8 current = ReadMemory(ReadHL());
	u8 result = current + 1;
	WriteMemory(ReadHL(), result);
	
	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	UpdateHalfCarryFlag(current, result, true);
}

void CPU::INCr16(CPU16BitReg reg) {
    if (log) Logger::instance->log(r16Names[reg], reg16ToString(reg));
	u16 current = Read16BitReg(reg);
	u16 result = current + 1;
	Write16BitReg(reg, result);
	lastOpCycles++;
	//no flags affected
}

void CPU::INCSP() {
	sp++;
	lastOpCycles++;
	//no flags affected
    if (log) Logger::instance->log("SP", "SP = " + Logger::instance->u16ToHex(sp));
}

void CPU::JPn16() {
	u16 address = ReadAtPC() | (ReadAtPC() << 8);
	pc = address;
	lastOpCycles++;
	//no flags affected
    if (log) Logger::instance->log(Logger::instance->u16ToHex(address), "");
}

void CPU::JPccn16(bool condition) {
	u16 address = ReadAtPC() | (ReadAtPC() << 8);
	if (log) Logger::instance->log(Logger::instance->u16ToHex(address), reg8ToString(CPU8BitReg::f));
	if (condition) {
		pc = address;
		lastOpCycles++;
	}
	//no flags affected
}

void CPU::JPHL() {
	pc = ReadHL();
	//no flags affected
    if (log) Logger::instance->log("HL", reg16ToString(CPU16BitReg::hl));
}

void CPU::JRe8() {
    s8 offset = (s8)ReadAtPC();
	pc += offset;
	lastOpCycles++;
	//no flags affected
    if (log) Logger::instance->log(Logger::instance->s8ToString(offset), "");
}

void CPU::JRcce8(bool condition) {
    s8 offset = (s8)ReadAtPC();
	if (log) Logger::instance->log(Logger::instance->s8ToString(offset), reg8ToString(CPU8BitReg::f));
	if (condition) {
		pc += offset;
		lastOpCycles++;
	}
	//no flags affected
}

void CPU::LDr8r8(CPU8BitReg leftReg, CPU8BitReg rightReg) {
    if (log) Logger::instance->log(r8Names[leftReg] + "," + r8Names[rightReg], reg8ToString(rightReg));

	Write8BitReg(leftReg, Read8BitReg(rightReg));
	//no flags affected
}

void CPU::LDr8n8(CPU8BitReg reg) {
	u8 constant = ReadAtPC();
    if (log) Logger::instance->log(r8Names[reg] + "," + Logger::instance->u8ToHex(constant), "");

	Write8BitReg(reg, constant);
	//no flags affected
}

void CPU::LDr16n16(CPU16BitReg reg) {
	u16 constant = ReadAtPC() | (ReadAtPC() << 8);
    if (log) Logger::instance->log(r16Names[reg] + "," + Logger::instance->u16ToHex(constant), "");

	Write16BitReg(reg, constant);
	//no flags affected
}

void CPU::LDHLr8(CPU8BitReg reg) {
    if (log) Logger::instance->log("[HL]," + r8Names[reg], reg16ToString(CPU16BitReg::hl) + " _ " + reg8ToString(reg));

	WriteMemory(ReadHL(),Read8BitReg(reg));
	//no flags affected
}

void CPU::LDHLn8() {
	u8 constant = ReadAtPC();
    if (log) Logger::instance->log("[HL]," + Logger::instance->u8ToHex(constant), reg16ToString(CPU16BitReg::hl));

	WriteMemory(ReadHL(),constant);
	//no flags affected
}

void CPU::LDr8HL(CPU8BitReg reg) {
	u8 mem = ReadMemory(ReadHL());
    if (log) Logger::instance->log(r8Names[reg] + ",[HL]", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(mem));

	Write8BitReg(reg, mem);
	//no flags affected
}

void CPU::LDr16A(CPU16BitReg reg) {
    if (log) Logger::instance->log("[" + r16Names[reg] + "],A", reg16ToString(reg) + " _ " + reg8ToString(CPU8BitReg::a));

	WriteMemory(Read16BitReg(reg), ReadAcc());
	//no flags affected
}

void CPU::LDn16A() {
	u16 address = ReadAtPC() | (ReadAtPC() << 8);
    if (log) Logger::instance->log(Logger::instance->u16ToHex(address) + ",A", reg8ToString(CPU8BitReg::a));

	WriteMemory(address, ReadAcc());
	//no flags affected
}

void CPU::LDHn8A() {
	u8 addressOffset = ReadAtPC();
    if (log) Logger::instance->log("0xFF" + Logger::instance->u8ToHex(addressOffset) + ",A", reg8ToString(CPU8BitReg::a));

	WriteMemory(0xFF00 + addressOffset, ReadAcc());
	//no flags affected
}

void CPU::LDHCA() {
    if (log) Logger::instance->log("0xFF00 + C,A", reg8ToString(CPU8BitReg::c) + " _ " + reg8ToString(CPU8BitReg::a));

	WriteMemory(0xFF00 + Read8BitReg(CPU8BitReg::c), ReadAcc());
	//no flags affected
}

void CPU::LDAr16(CPU16BitReg reg) {
	u8 mem = ReadMemory(Read16BitReg(reg));
    if (log) Logger::instance->log("A,[" + r16Names[reg] + "]", reg16ToString(reg) + " _ [r] = " + Logger::instance->u8ToHex(mem));

	WriteAcc(mem);
	//no flags affected
}

void CPU::LDAn16() {
	u16 address = ReadAtPC() | (ReadAtPC() << 8);
	u8 mem = ReadMemory(address);
    if (log) Logger::instance->log("A,[" + Logger::instance->u16ToHex(address) + "]", "[r] = " + Logger::instance->u8ToHex(mem));

	WriteAcc(mem);
	//no flags affected
}

void CPU::LDHAn8() {
	u8 addressOffset = ReadAtPC();
	u8 mem = ReadMemory(0xFF00 + addressOffset);
    if (log) Logger::instance->log("A,[0xFF" + Logger::instance->u8ToHex(addressOffset) + "]", "[r] = " + Logger::instance->u8ToHex(mem));
	WriteAcc(mem);
	//no flags affected
}

void CPU::LDHAC() {
	u8 mem = ReadMemory(0xFF00 + Read8BitReg(CPU8BitReg::c));
    if (log) Logger::instance->log("A,[0xFF00 + C]", reg8ToString(CPU8BitReg::c) + " _ [r] = " + Logger::instance->u8ToHex(mem));
	WriteAcc(mem);
	//no flags affected
}

void CPU::LDHLincA() {
    if (log) Logger::instance->log("[HL++],A", reg16ToString(CPU16BitReg::hl) + " _ " + reg8ToString(CPU8BitReg::a));
	u16 address = ReadHL();
	WriteMemory(address,ReadAcc());
	WriteHL(++address);
	//no flags affected
}

void CPU::LDHLdecA() {
    if (log) Logger::instance->log("[HL--],A", reg16ToString(CPU16BitReg::hl) + " _ " + reg8ToString(CPU8BitReg::a));
	u16 address = ReadHL();
	WriteMemory(address, ReadAcc());
	WriteHL(--address);
	//no flags affected
}

void CPU::LDAHLinc() {
	u16 address = ReadHL();
    if (log) Logger::instance->log("A,[HL++]", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(ReadMemory(address)));
	WriteAcc(ReadMemory(address));
	WriteHL(++address);
	//no flags affected
}

void CPU::LDAHLdec() {
	u16 address = ReadHL();
    if (log) Logger::instance->log("A,[HL--]", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(ReadMemory(address)));
	WriteAcc(ReadMemory(address));
	WriteHL(--address);
	//no flags affected
}

void CPU::LDSPn16() {
	u16 address = ReadAtPC() | (ReadAtPC() << 8);
	sp = address;
	//no flags affected
    if (log) Logger::instance->log("SP," + Logger::instance->u16ToHex(address), "");
}

void CPU::LDn16SP() {
	u8 lsb = (u8)sp;
	u8 msb = (u8)(sp >> 8);

	u16 address = ReadAtPC() | (ReadAtPC() << 8);
    if (log) Logger::instance->log("[" + Logger::instance->u16ToHex(address) + "],SP", "SP = " + Logger::instance->u16ToHex(sp));

	WriteMemory(address, lsb);
	WriteMemory(address + 1, msb);
	//no flags affected
}

void CPU::LDHLSPe8() {
    s8 offset = (s8)ReadAtPC();
    if (log) Logger::instance->log("HL,SP+" + Logger::instance->s8ToString(offset), "SP = " + Logger::instance->u16ToHex(sp));

	u16 current = sp;
	u16 result = current + offset;
	WriteHL(result);
	lastOpCycles++;

	SetFlag(FlagBit::Zero, false);
	SetFlag(FlagBit::Negative, false);
	// Carry and HalfCarry from lsb to pass blargg tests https://github.com/Drenn1/GameYob/issues/15
	UpdateHalfCarryFlag((u8)current, (u8)result, true);
	UpdateCarryFlag((u8)current, (u8)result, true);
}

void CPU::LDSPHL() {
	sp = ReadHL();
	lastOpCycles++;
	//no flags affected
    if (log) Logger::instance->log("SP,HL", reg16ToString(CPU16BitReg::hl));
}

void CPU::NOP() {
	//do nothing
	//no flags affected
	//no extra cycles
    if (log) Logger::instance->log("", "");
}

void CPU::ORAr8(CPU8BitReg reg) {
	u8 result = ReadAcc() | Read8BitReg(reg);
    if (log) Logger::instance->log("A," + r8Names[reg], reg8ToString(reg));

	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, false);
}

void CPU::ORAHL() {
	u8 mem = ReadMemory(ReadHL());
    if (log) Logger::instance->log("A,[HL]", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(mem));

	u8 result = ReadAcc() | mem;
	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, false);
}

void CPU::ORAn8() {
	u8 constant = ReadAtPC();
    if (log) Logger::instance->log("A," + Logger::instance->u8ToHex(constant), "");

	u8 result = ReadAcc() | constant;
	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, false);
}

void CPU::POPr16(CPU16BitReg reg) {
	Write16BitReg(reg, Pop16());
	//no flags affected (except if reg is AF)
    if (log) Logger::instance->log(r16Names[reg], "(after) " + reg16ToString(reg) + " _ SP = " + Logger::instance->u16ToHex(sp));
}

void CPU::PUSHr16(CPU16BitReg reg) {
	Push16(Read16BitReg(reg));
	lastOpCycles++;
	//no flags affected
    if (log) Logger::instance->log(r16Names[reg], "(after) " + reg16ToString(reg) + " _ SP = " + Logger::instance->u16ToHex(sp));
}

void CPU::RESu3r8(u8 bit, CPU8BitReg reg) {
    if (log) Logger::instance->log("", reg8ToString(reg));
	u8 value = Read8BitReg(reg) & ~(1 << bit);
	Write8BitReg(reg, value);
	//no flags affected
}

void CPU::RESu3HL(u8 bit) {
	u16 address = ReadHL();
	u8 mem = ReadMemory(address);
	u8 value = mem & ~(1 << bit);
	WriteMemory(address, value);
	//no flags affected
    if (log) Logger::instance->log("", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(mem));
}

void CPU::RET() {
	pc = Pop16();
	lastOpCycles++;
	//no flags affected
    if (log) Logger::instance->log("", "");
}

void CPU::RETcc(bool condition) {
	if (log) Logger::instance->log("", reg8ToString(CPU8BitReg::f));
	lastOpCycles++;
	if (condition) {
		pc = Pop16();
		lastOpCycles++;
	}
	//no flags affected
}

void CPU::RETI() {
	RET();

	//TODO check if this has same bug of EI
	interruptService.IME = true;

	//no flags affected
    if (log) Logger::instance->log("", "");
}

void CPU::RLr8(CPU8BitReg reg) {
    if (log) Logger::instance->log("", reg8ToString(reg));
	u8 value = Read8BitReg(reg);
	u8 newCarry = value >> 7;
	value <<= 1;
	value |= (u8)HasFlag(FlagBit::Carry);
	Write8BitReg(reg, value);
	
	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::RLHL() {
	u16 address = ReadHL();
	u8 value = ReadMemory(address);
    if (log) Logger::instance->log("", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(value));
	u8 newCarry = value >> 7;
	value <<= 1;
	value |= (u8)HasFlag(FlagBit::Carry);
	WriteMemory(address,value);
	
	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::RLA() {
    if (log) Logger::instance->log("", reg8ToString(CPU8BitReg::a));
	u8 value = ReadAcc();
	u8 newCarry = value >> 7;
	value <<= 1;
	value |= (u8)HasFlag(FlagBit::Carry);
	WriteAcc(value);

	SetFlag(FlagBit::Zero, false);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::RLCr8(CPU8BitReg reg) {
    if (log) Logger::instance->log("", reg8ToString(reg));

	u8 value = Read8BitReg(reg);
	u8 newCarry = value >> 7;
	value <<= 1;
	value |= newCarry;
	Write8BitReg(reg, value);
	
	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::RLCHL() {
	u16 address = ReadHL();
	u8 value = ReadMemory(address);
    if (log) Logger::instance->log("", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(value));

	u8 newCarry = value >> 7;
	value <<= 1;
	value |= newCarry;
	WriteMemory(address, value);

	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::RLCA() {
    if (log) Logger::instance->log("", reg8ToString(CPU8BitReg::a));
	u8 value = ReadAcc();
	u8 newCarry = value >> 7;
	value <<= 1;
	value |= newCarry;
	WriteAcc(value);

	SetFlag(FlagBit::Zero, false);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::RRr8(CPU8BitReg reg) {
    if (log) Logger::instance->log("", reg8ToString(reg));
	u8 value = Read8BitReg(reg);
	u8 newCarry = value & 0x01;
	value >>= 1;
	value |= ((u8)HasFlag(FlagBit::Carry) << 7);
	Write8BitReg(reg, value);

	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::RRHL() {
	u16 address = ReadHL();
	u8 value = ReadMemory(address);
    if (log) Logger::instance->log("", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(value));
	u8 newCarry = value & 0x01;
	value >>= 1;
	value |= ((u8)HasFlag(FlagBit::Carry) << 7);
	WriteMemory(address, value);

	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::RRA() {
    if (log) Logger::instance->log("", reg8ToString(CPU8BitReg::a));
	u8 value = ReadAcc();
	u8 newCarry = value & 0x01;
	value >>= 1;
	value |= ((u8)HasFlag(FlagBit::Carry) << 7);
	WriteAcc(value);

	SetFlag(FlagBit::Zero, false);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::RRCr8(CPU8BitReg reg) {
    if (log) Logger::instance->log("", reg8ToString(reg));
	u8 value = Read8BitReg(reg);
	u8 newCarry = value & 0x01;
	value >>= 1;
	value |= (newCarry << 7);
	Write8BitReg(reg, value);

	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::RRCHL() {
	u16 address = ReadHL();
	u8 value = ReadMemory(address);
    if (log) Logger::instance->log("", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(value));
	u8 newCarry = value & 0x01;
	value >>= 1;
	value |= (newCarry << 7);
	WriteMemory(address, value);

	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::RRCA() {
    if (log) Logger::instance->log("", reg8ToString(CPU8BitReg::a));
	u8 value = ReadAcc();
	u8 newCarry = value & 0x01;
	value >>= 1;
	value |= (newCarry << 7);
	WriteAcc(value);

	SetFlag(FlagBit::Zero, false);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::RSTvec(u8 vec) {
	Push16(pc);
	pc = vec;
	lastOpCycles++;
	//no flags affected
    if (log) Logger::instance->log(Logger::instance->u8ToHex(vec), "");
}

void CPU::SBCAr8(CPU8BitReg reg) {
	SBCA(Read8BitReg(reg));
}

void CPU::SBCAHL() {
	SBCA(ReadMemory(ReadHL()));
}

void CPU::SBCAn8() {
	SBCA(ReadAtPC());
}

void CPU::SCF() {
	SetFlag(FlagBit::Carry, true);

	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Negative, false);
    if (log) Logger::instance->log("", "");
}

void CPU::SETu3r8(u8 bit, CPU8BitReg reg) {
    if (log) Logger::instance->log("", reg8ToString(reg));
	u8 value = Read8BitReg(reg) | (1 << bit);
	Write8BitReg(reg, value);
	//no flags affected
}

void CPU::SETu3HL(u8 bit) {
	u16 address = ReadHL();
	u8 value = ReadMemory(address);
    if (log) Logger::instance->log("", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(value));
	value |= (1 << bit);
	WriteMemory(address,value);
	//no flags affected
}

void CPU::SLAr8(CPU8BitReg reg) {
    if (log) Logger::instance->log("", reg8ToString(reg));
	u8 value = Read8BitReg(reg);
	u8 newCarry = value >> 7;
	value <<= 1;
	Write8BitReg(reg, value);
	
	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::SLAHL() {
	u16 address = ReadHL();
	u8 value = ReadMemory(address);
    if (log) Logger::instance->log("", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(value));

	u8 newCarry = value >> 7;
	value <<= 1;
	WriteMemory(address, value);

	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::SRAr8(CPU8BitReg reg) {
    if (log) Logger::instance->log("", reg8ToString(reg));

	u8 value = Read8BitReg(reg);
	u8 newSeven = value & 0x80;
	u8 newCarry = value & 0x01;
	value >>= 1;
	value |= newSeven;
	Write8BitReg(reg, value);

	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::SRAHL() {
	u16 address = ReadHL();
	u8 value = ReadMemory(address);
    if (log) Logger::instance->log("", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(value));

	u8 newSeven = value & 0x80;
	u8 newCarry = value & 0x01;
	value >>= 1;
	value |= newSeven;
	WriteMemory(address, value);

	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::SRLr8(CPU8BitReg reg) {
    if (log) Logger::instance->log("", reg8ToString(reg));
	u8 value = Read8BitReg(reg);
	u8 newCarry = value & 0x01;
	value >>= 1;
	Write8BitReg(reg, value);
	
	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::SRLHL() {
	u16 address = ReadHL();
	u8 value = ReadMemory(address);
    if (log) Logger::instance->log("", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(value));

	u8 newCarry = value & 0x01;
	value >>= 1;
	WriteMemory(address, value);

	UpdateZeroFlag(value);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, newCarry == 1);
}

void CPU::STOP() {
    //2 bytes? 0 cycles?
    if ((key1 & 0x01) == 1) {
        key1 &= 0xFE; // reset bit 0
        if (IsDoubleSpeedEnabled()) {
            key1 &= 0x7F;
            printf("Double Speed disabled\n");
        }
        else {
            key1 |= 0x80;
            printf("Double Speed enabled\n");
        }
    } else
        isHalted = true;
	// TODO turn off LCD
	//no flags affected
    if (log) Logger::instance->log("", "");
}

void CPU::SUBAr8(CPU8BitReg reg) {
    if (log) Logger::instance->log("A," + r8Names[reg], reg8ToString(CPU8BitReg::a) + " _ " + reg8ToString(reg));
	u8 current = ReadAcc();
	u8 result = current - Read8BitReg(reg);
	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, true);
	UpdateHalfCarryFlag(current, result, false);
	UpdateCarryFlag(current, result, false);
}

void CPU::SUBAHL() {
	u8 mem = ReadMemory(ReadHL());
    if (log) Logger::instance->log("A,[HL]", reg8ToString(CPU8BitReg::a) + " _ " + reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(mem));
	u8 current = ReadAcc();
	u8 result = current - mem;
	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, true);
	UpdateHalfCarryFlag(current, result, false);
	UpdateCarryFlag(current, result, false);
}

void CPU::SUBAn8() {
	u8 constant = ReadAtPC();
	u8 current = ReadAcc();
    if (log) Logger::instance->log("A," + Logger::instance->u8ToHex(constant), reg8ToString(CPU8BitReg::a));

	u8 result = current - constant;
	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, true);
	UpdateHalfCarryFlag(current, result, false);
	UpdateCarryFlag(current, result, false);
}

void CPU::SWAPr8(CPU8BitReg reg) {
    if (log) Logger::instance->log("", reg8ToString(reg));
	u8 value = Read8BitReg(reg);
	u8 result = (value >> 4) + (value << 4);
	Write8BitReg(reg, result);
	
	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, false);
}

void CPU::SWAPHL() {
	u16 address = ReadHL();
	u8 value = ReadMemory(address);
    if (log) Logger::instance->log("", reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(value));

	u8 result = (value >> 4) + (value << 4);
	WriteMemory(address, result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, false);
}

void CPU::XORAr8(CPU8BitReg reg) {
    if (log) Logger::instance->log("A," + r8Names[reg], reg8ToString(CPU8BitReg::a) + " _ " + reg8ToString(reg));
	u8 result = ReadAcc() ^ Read8BitReg(reg);
	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, false);
}

void CPU::XORAHL() {
	u8 value = ReadMemory(ReadHL());
    if (log) Logger::instance->log("A,[HL]", reg8ToString(CPU8BitReg::a) + " _ " + reg16ToString(CPU16BitReg::hl) + " _ [HL] = " + Logger::instance->u8ToHex(value));

	u8 result = ReadAcc() ^ value;
	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, false);
}

void CPU::XORAn8() {
	u8 constant = ReadAtPC();
    if (log) Logger::instance->log("A," + Logger::instance->u8ToHex(constant), reg8ToString(CPU8BitReg::a));

	u8 result = ReadAcc() ^ constant;
	WriteAcc(result);

	UpdateZeroFlag(result);
	SetFlag(FlagBit::Negative, false);
	SetFlag(FlagBit::HalfCarry, false);
	SetFlag(FlagBit::Carry, false);
}
