#include <cstdint>
#include <iostream>

#include <Windows.h>
#include "lib/OlsApi.h"
#include "lib/OlsDef.h"

#include "Cpuid.h"
#include "PowerState.h"

#include <bitset>

static constexpr unsigned int PSTATE_P0{ 0xC0010064 };
static constexpr unsigned int PSTATE_P1{ 0xC0010065 };
static constexpr unsigned int PSTATE_P2{ 0xC0010066 };

// prototypes
int initWinRing0();

int main() {
	int ret = initWinRing0();
	if (ret)
	{
		return ret;
	}

	DWORD eax;
	DWORD edx;
	Rdmsr(PSTATE_P1, &eax, &edx);

	uint64_t pstate = eax | ((uint64_t)edx << 32);
	PowerState powerState(pstate);

	// set fid to 100 for testing
	powerState.setFid(100);

	pstate = powerState.getPstate();
	eax = pstate & 0xFFFFFFFF;
	edx = pstate >> 32;

	// according to register reference, this msr needs to be set for every thread
	DWORD_PTR mask = 0x1;
	while (mask < 0xFF) {
		WrmsrTx(PSTATE_P1, eax, edx, mask);
		mask = mask << 1;
	}

	DeinitializeOls();

	return 0;
}

int initWinRing0()
{
	if (!validateCpu())
	{
		return -1;
	}

	if (!InitializeOls())
	{
		std::cerr << "WinRing0: Failed to initialize" << std::endl;
		return -1;
	}

	DWORD dllStatus = GetDllStatus();

	switch (dllStatus) {
	case OLS_DLL_NO_ERROR:
		break;
	case OLS_DLL_UNSUPPORTED_PLATFORM:
		std::cerr << "WinRing0: DLL - unsupported platform" << std::endl;
		break;
	case OLS_DLL_DRIVER_NOT_LOADED:
		std::cerr << "WinRing0: DLL - driver not loaded" << std::endl;
		break;
	case OLS_DLL_DRIVER_NOT_FOUND:
		std::cerr << "WinRing0: DLL - driver not found" << std::endl;
		break;
	case OLS_DLL_DRIVER_UNLOADED:
		std::cerr << "WinRing0: DLL - driver unloaded by another process" << std::endl;
		break;
	case OLS_DLL_DRIVER_NOT_LOADED_ON_NETWORK:
		std::cerr << "WinRing0: DLL - running from network share, driver can't be loaded" << std::endl;
		break;
	case OLS_DLL_UNKNOWN_ERROR:
		std::cerr << "WinRing0: DLL - unknown error" << std::endl;
		break;
	default:
		break;
	}

	return 0;
}
