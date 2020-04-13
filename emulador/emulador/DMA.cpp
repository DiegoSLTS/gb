#include "DMA.h"
#include "MMU.h"
#include "Logger.h"

DMA::DMA(MMU& mmu, u8* oam) : mmu(mmu), oam(oam) {}
DMA::~DMA() {}

void DMA::Step(u8 cycles) {
	while (currentCycles < 160 && cycles > 0) {
		oam[currentCycles] = mmu.Read(addressBase + currentCycles);
        if (log) Logger::instance->log("OAM[" + Logger::u16ToHex(currentCycles) + "] = " + Logger::u8ToHex(oam[currentCycles]) + "\n");
		currentCycles++;
		cycles--;
        if (currentCycles >= 160 && log) Logger::instance->log("DMA finished\n");
	}
}

void DMA::StepHDMA() {
    if (!hdmaInProgress)
        return;

    DoHDMATransfer(0x10);

    if (remaining > 0) {
        remaining--;
    } else {
        remaining = 0x7F;
        hdmaInProgress = false;
    }
    
    // TODO emulate time spent
}

u8 DMA::Read(u16 address) {
	switch (address) {
	case 0xFF46:
		//TODO confirm this?
		return addressBase >> 8;
	case 0xFF51:
		return HDMASource >> 8;
	case 0xFF52:
		return HDMASource;
	case 0xFF53:
		return HDMADest >> 8;
	case 0xFF54:
		return HDMADest;
	case 0xFF55:
    {
        u8 bit7 = hdmaInProgress ? 0 : 0x80;
        return bit7 | remaining;
    }
	}

	return 0xFF;
}

void DMA::Write(u8 value, u16 address) {
    switch (address) {
    case 0xFF46:
        //TODO assert addressBase is valid?
        currentCycles = 0;
        addressBase = value << 8;
        if (log) Logger::instance->log("DMA started from " + Logger::u16ToHex(addressBase) + "\n");
        break;
    case 0xFF51:
        HDMASource &= 0x00FF;
        HDMASource |= (value << 8);
        if (log) Logger::instance->log("HDMA1 = " + Logger::u8ToHex(value) + "\n");
        break;
    case 0xFF52:
        HDMASource &= 0xFF00;
        HDMASource |= (value & 0xF0);
        if (log) Logger::instance->log("HDMA2 = " + Logger::u8ToHex(value & 0xF0) + "\n");
        break;
	case 0xFF53:
        HDMADest &= 0x80FF;
        HDMADest |= ((value & 0x1F) << 8);
        if (log) Logger::instance->log("HDMA3 = " + Logger::u8ToHex(value & 0x1F) + "\n");
        break;
	case 0xFF54:
        HDMADest &= 0xFF00;
        HDMADest |= (value & 0xF0);
        if (log) Logger::instance->log("HDMA4 = " + Logger::u8ToHex(value & 0xF0) + "\n");
        break;
	case 0xFF55:
        if (hdmaInProgress) {
            if ((value & 0x80) == 0)
                hdmaInProgress = false;
        } else {
            if ((value & 0x80) == 0) {
                if (log) Logger::instance->log("GDMA started\n");
                u16 count = ((value & 0x7F) + 1) * 0x0010;
                DoHDMATransfer(count);

                remaining = 0x7F;
                hdmaInProgress = false;
                // TODO emulate time spent
            } else {
                if (log) Logger::instance->log("HDMA started\n");
                hdmaInProgress = true;
                remaining = value & 0x7F;
            }
        }
        if (log) {
            u8 HDMA5 = Read(0xFF55);
            Logger::instance->log("HDMA5 = " + Logger::u8ToHex(value) + " ; " + Logger::u8ToHex(HDMA5) + "\n");
        }
        break;
	}
}

void DMA::Load(std::ifstream& stream) const {
    // reads 9 bytes for CurrentCycles and the rest of the properties
    stream.read((char*)&currentCycles, 9);
}

void DMA::Save(std::ofstream& stream) const {
    // writes 9 bytes for CurrentCycles and the rest of the properties
    stream.write((char*)&currentCycles, 9);
}

void DMA::DoHDMATransfer(u16 count) {
    if (log) {
        Logger::instance->log("Transfering\n");
        Logger::instance->log("Count: " + Logger::u8ToHex(count) + "\n");
        Logger::instance->log("Source: " + Logger::u16ToHex(HDMASource) + "\n");
        Logger::instance->log("Dest: " + Logger::u16ToHex(HDMADest) + "\n");
    }

    for (u16 i = 0; i < count; i++)
        mmu.Copy(HDMASource + i, HDMADest + i);

    HDMASource += count;
    HDMADest += count;

    if (log) Logger::instance->log("Transfer finished\n");
}