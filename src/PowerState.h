#pragma once
#include <cstdint>

class PowerState
{
public:
	PowerState(int pstate, uint64_t value);
	virtual ~PowerState();

	int getPstate() const;
	unsigned int getRegister() const;
	static unsigned int getRegister(int pstate);
	uint8_t getFid() const;
	uint8_t getDid() const;
	uint8_t getVid() const;

	double calculateRatio() const;
	static double calculateRatio(uint8_t fid, uint8_t did);
	double calculateVcore() const;
	static double calculateVcore(uint8_t vid);
	double calculateFrequency() const;
	static double calculateFrequency(uint8_t fid, uint8_t did);

	void setFid(unsigned int fid);
	void setDid(unsigned int did);
	void setVid(unsigned int vid);

	void print() const;
	uint64_t getValue() const;

	// some sane limits for all the values
	static constexpr uint8_t FID_MIN{ 0x10 };
	static constexpr uint8_t FID_MAX{ 0xFF };
	static constexpr uint8_t DID_MIN{ 0x08 };
	static constexpr uint8_t DID_MAX{ 0x1A }; // some higher dividers are reserved
	static constexpr uint8_t VID_MIN{ 0x20 }; // 1.35V, could be changed to 0 to unlock higher limits
	static constexpr uint8_t VID_MAX{ 0xA8 }; // 0.5V

private:
	int pstate;
	uint64_t value;

	void setBits(uint8_t value, uint8_t length, uint8_t offset);

	static constexpr unsigned int REGISTERS[]
	{
		0xC0010064,
		0xC0010065,
		0xC0010066,
		0xC0010067,
		0xC0010068,
		0xC0010069,
		0xC001006A,
		0xC001006B
	};
};
