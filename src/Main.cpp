#include <cstdint>
#include <iostream>

#include <Windows.h>
#include "lib/OlsApi.h"

#include "Cpuid.h"
#include "PowerState.h"


int main() {
	if (!validateCpu())
	{
		return -1;
	}

	if (!InitializeOls())
	{
		std::cerr << "Failed to initialize WinRing0" << std::endl;
		return -1;
	}

	DWORD eax;
	DWORD edx;
	Rdmsr(0xC0010064, &eax, &edx);

	uint64_t pstate = eax | ((uint64_t)edx << 32);
	PowerState powerState(pstate);

	powerState.print();

	// change the power state and print the changes
	powerState.setFid(110);
	powerState.setDid(12);
	powerState.setVid(93);
	powerState.print();

	DeinitializeOls();

	return 0;
}
