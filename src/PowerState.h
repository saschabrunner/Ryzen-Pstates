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

	void setFid(uint8_t fid);
	void setDid(uint8_t did);
	void setVid(uint8_t vid);

	void print();

private:
	uint64_t pstate;

	void setBits(uint8_t value, uint8_t length, uint8_t offset);
};
