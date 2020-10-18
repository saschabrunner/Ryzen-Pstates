#pragma once
#include <cstdint>

class PowerState
{
public:
	PowerState(uint64_t pstate);
	virtual ~PowerState();

	uint8_t getFid() const;
	uint8_t getDid() const;
	uint8_t getVid() const;

	double calculateRatio() const;
	static double calculateRatio(uint8_t fid, uint8_t did);
	double calculateVcore() const;
	static double calculateVcore(uint8_t vid);
	double calculateFrequency() const;
	static double calculateFrequency(uint8_t fid, uint8_t did);

	bool setFid(unsigned int fid);
	bool setDid(unsigned int did);
	bool setVid(unsigned int vid);

	void print() const;
	uint64_t getPstate() const;

	// some sane limits for all the values
	static constexpr uint8_t FID_MIN{ 0x10 };
	static constexpr uint8_t FID_MAX{ 0xFF };
	static constexpr uint8_t DID_MIN{ 0x08 };
	static constexpr uint8_t DID_MAX{ 0x1A }; // some higher dividers are reserved
	static constexpr uint8_t VID_MIN{ 0x20 }; // 1.35V
	static constexpr uint8_t VID_MAX{ 0xA8 }; // 0.5V

private:
	uint64_t pstate;

	void setBits(uint8_t value, uint8_t length, uint8_t offset);
};
