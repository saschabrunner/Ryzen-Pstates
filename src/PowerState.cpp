#include "PowerState.h"

#include <iostream>

PowerState::PowerState(uint64_t pstate)
	:pstate(pstate)
{
}

PowerState::~PowerState()
{
}

uint8_t PowerState::getFid() const
{
	return pstate & 0xff;
}

uint8_t PowerState::getDid() const
{
	return (pstate & 0x3f00) >> 8;
}

uint8_t PowerState::getVid() const
{
	return (uint8_t)((pstate & 0x3fc000) >> 14);
}

double PowerState::calculateRatio() const
{
	return calculateRatio(getFid(), getDid());
}

double PowerState::calculateRatio(uint8_t fid, uint8_t did)
{
	return (25.0 * fid) / (12.5 * did);
}

double PowerState::calculateVcore() const
{
	return calculateVcore(getVid());
}

double PowerState::calculateVcore(uint8_t vid)
{
	return 1.55 - (0.00625 * vid);
}

double PowerState::calculateFrequency() const
{
	return calculateRatio() * 100;
}

double PowerState::calculateFrequency(uint8_t fid, uint8_t did)
{
	return calculateRatio(fid, did) * 100;
}

void PowerState::setFid(uint8_t fid)
{
	setBits(fid, 8, 0);
}

void PowerState::setDid(uint8_t did)
{
	setBits(did, 6, 8);
}

void PowerState::setVid(uint8_t vid)
{
	setBits(vid, 8, 14);
}

void PowerState::print()
{
	std::cout << "FID: " << +getFid()
		<< "\nDID: " << +getDid()
		<< "\nVID: " << +getVid()
		<< "\nRatio: " << calculateRatio()
		<< "\nFrequency (MHz): " << calculateFrequency()
		<< "\nVCore (V): " << calculateVcore() << std::endl;
}

void PowerState::setBits(uint8_t value, uint8_t length, uint8_t offset) {
	// example:
	// pstate = 10010110
	// value = 10
	// length = 2
	// offset = 4
	// (1 << length) = 00000011
	// ((1 << length) - 1) << offset) = 00110000
	// (pstate & ((1 << length) - 1) << offset) = 00010000
	// (pstate ^ (pstate & ((1 << length) - 1) << offset)) = 10000110
	// (value << offset) = 00100000
	// (pstate ^ (pstate & ((1 << length) - 1) << offset)) + (value << offset) = 10100110
	pstate = (pstate ^ (pstate & (((uint64_t)1 << length) - 1) << offset)) + ((uint64_t)value << offset);
}
