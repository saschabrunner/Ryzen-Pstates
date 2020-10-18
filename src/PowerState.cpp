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

bool PowerState::setFid(unsigned int fid)
{
	if (fid > FID_MAX || fid < FID_MIN) {
		std::cerr << "Requested FID '" << +fid << "' out of bounds "
			"(must be between " << +FID_MIN << " and " << +FID_MAX << ")" << std::endl;
		return false;
	}

	setBits(fid, 8, 0);
	return true;
}

bool PowerState::setDid(unsigned int did)
{
	if (did > DID_MAX || did < DID_MIN) {
		std::cerr << "Requested DID '" << +did << "' out of bounds "
			"(must be between " << +DID_MIN << " and " << +DID_MAX << ")" << std::endl;
		return false;
	}

	setBits(did, 6, 8);
	return true;
}

bool PowerState::setVid(unsigned int vid)
{
	if (vid > VID_MAX || vid < VID_MIN) {
		std::cerr << "Requested VID '" << +vid << "' out of bounds "
			"(must be between " << +VID_MIN << " and " << +VID_MAX << ")" << std::endl;
		return false;
	}

	setBits(vid, 8, 14);
	return true;
}

void PowerState::print() const
{
	std::cout << "FID: " << +getFid()
		<< "\nDID: " << +getDid()
		<< "\nVID: " << +getVid()
		<< "\nRatio: " << calculateRatio()
		<< "\nFrequency (MHz): " << calculateFrequency()
		<< "\nVCore (V): " << calculateVcore() << std::endl;
}

uint64_t PowerState::getPstate() const
{
	return pstate;
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
