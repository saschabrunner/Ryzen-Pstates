#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>

#include <Windows.h>
#include "lib/OlsApi.h"
#include "lib/OlsDef.h"
#include "lib/argh/argh.h"

#include "Cpuid.h"
#include "PowerState.h"

static constexpr unsigned int PSTATES[]
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

struct Params
{
	unsigned int pstate{ UINT_MAX };
	std::optional<unsigned int> fid;
	std::optional<unsigned int> did;
	std::optional<unsigned int> vid;
};

// prototypes
int initWinRing0();
Params parseArguments(int argc, char* argv[]);
void updatePstate(const Params& params);

int main(int argc, char* argv[]) {
	int ret = initWinRing0();
	if (ret)
	{
		return ret;
	}

	Params params = parseArguments(argc, argv);

	try
	{
		updatePstate(params);
	}
	catch (const std::exception& e)
	{
		std::cerr << e.what() << "\nExiting..." << std::endl;
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

Params parseArguments(int argc, char* argv[])
{
	argh::parser argParser(argc, argv);

	Params params;

	// PState
	auto curArg = argParser({ "-p", "--pstate" });
	if (curArg)
	{
		curArg >> params.pstate;
		if (params.pstate > 7) {
			std::cerr << "Pstate must be between 0 and 7" << std::endl;
			exit(-1);
		}
	}
	else
	{
		std::cerr << "Required parameter --pstate (-p) missing" << std::endl;
		exit(-1);
	}

	// FID
	curArg = argParser({ "-f", "--fid" });
	if (curArg)
	{
		unsigned int temp;
		curArg >> temp;
		params.fid = std::optional<unsigned int>(temp);
	}

	// DID
	curArg = argParser({ "-d", "--did" });
	if (curArg)
	{
		unsigned int temp;
		curArg >> temp;
		params.did = std::optional<unsigned int>(temp);
	}

	// FID
	curArg = argParser({ "-v", "--vid" });
	if (curArg)
	{
		unsigned int temp;
		curArg >> temp;
		params.vid = std::optional<unsigned int>(temp);
	}

	return params;
}

void updatePstate(const Params& params)
{
	DWORD eax;
	DWORD edx;
	Rdmsr(PSTATES[params.pstate], &eax, &edx);

	uint64_t pstateVal = eax | ((uint64_t)edx << 32);
	PowerState powerState(pstateVal);

	std::cout << "Current pstate:" << std::endl;
	powerState.print();
	std::cout << "--------------------------------------------------" << std::endl;

	if (params.fid)
	{
		powerState.setFid(*params.fid);
	}

	if (params.did)
	{
		powerState.setDid(*params.did);
	}

	if (params.vid)
	{
		powerState.setVid(*params.vid);
	}

	std::cout << "New pstate:" << std::endl;
	powerState.print();
	std::cout << "--------------------------------------------------" << std::endl;

	pstateVal = powerState.getPstate();
	eax = pstateVal & 0xFFFFFFFF;
	edx = pstateVal >> 32;

	// according to register reference, this msr needs to be set for every thread
	// note: 8 threads are assumed, less actual threads shouldn't break anything
	DWORD_PTR mask = 0x1;
	while (mask < 0xFF) {
		WrmsrTx(PSTATES[params.pstate], eax, edx, mask);
		mask = mask << 1;
	}

	std::cout << "Pstate updated" << std::endl;
}
