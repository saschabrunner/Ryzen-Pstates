#include <cstdint>
#include <iostream>

#include <Windows.h>
#include "lib/OlsApi.h"

#include "PowerState.h"


int main() {
	if (!InitializeOls())
	{
		std::cerr << "Failed to initialize WinRing0" << std::endl;
		return -1;
	}

	DWORD eax;
	DWORD edx;
	Rdmsr(0xC0010064, &eax, &edx);

	uint64_t pstate = eax | ((uint64_t)edx << 32);

	int fid = pstate & 0xff;
	int did = (pstate & 0x3f00) >> 8;
	int vid = (pstate & 0x3fc000) >> 14;
	double ratio = (25.0 * fid) / (12.5 * did);
	double vcore = 1.55 - (0.00625 * vid);

	std::cout << "FID: " << fid
		<< "\nDID: " << did
		<< "\nVID: " << vid
		<< "\nRatio: " << ratio
		<< "\nVCore: " << vcore << std::endl;

	DeinitializeOls();

	return 0;
}