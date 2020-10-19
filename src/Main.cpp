#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <thread>

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
void printUsage();
void updatePstate(const Params& params, int numThreads);
int getNumberOfHardwareThreads();

int main(int argc, char* argv[]) {
	int ret = initWinRing0();
	if (ret)
	{
		return ret;
	}

	int numThreads = getNumberOfHardwareThreads();
	Params params = parseArguments(argc, argv);

	try
	{
		updatePstate(params, numThreads);
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
		return -1;
	case OLS_DLL_DRIVER_NOT_LOADED:
		std::cerr << "WinRing0: DLL - driver not loaded" << std::endl;
		return -1;
	case OLS_DLL_DRIVER_NOT_FOUND:
		std::cerr << "WinRing0: DLL - driver not found" << std::endl;
		return -1;
	case OLS_DLL_DRIVER_UNLOADED:
		std::cerr << "WinRing0: DLL - driver unloaded by another process" << std::endl;
		return -1;
	case OLS_DLL_DRIVER_NOT_LOADED_ON_NETWORK:
		std::cerr << "WinRing0: DLL - running from network share, driver can't be loaded" << std::endl;
		return -1;
	case OLS_DLL_UNKNOWN_ERROR:
		std::cerr << "WinRing0: DLL - unknown error" << std::endl;
		return -1;
	default:
		std::cerr << "WinRing0: DLL - unknown dll status code '" << dllStatus << "'";
		return -1;
	}

	return 0;
}

Params parseArguments(int argc, char* argv[])
{
	if (argc == 1)
	{
		printUsage();
		exit(-1);
	}

	argh::parser argParser(argc, argv);
	Params params;

	// PState
	auto curArg = argParser({ "-p", "--pstate" });
	if (curArg)
	{
		curArg >> params.pstate;
		if (params.pstate > 7) {
			std::cerr << "Pstate must be between 0 and 7" << std::endl;
			printUsage();
			exit(-1);
		}
	}
	else
	{
		std::cerr << "Required parameter --pstate (-p) missing" << std::endl;
		printUsage();
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

void printUsage()
{
	std::cout << "Options:\n"
		<< "-p, --pstate	Required, Selects PState to change (0 - 7)\n"
		<< "-f, --fid	New FID to set (" << +PowerState::FID_MIN << " - " << +PowerState::FID_MAX << ")\n"
		<< "-d, --did	New DID to set (" << +PowerState::DID_MIN << " - " << +PowerState::DID_MAX << ")\n"
		<< "-v, --vid	New VID to set (" << +PowerState::VID_MIN << " - " << +PowerState::VID_MAX << ")\n\n"
		<< "Example: ryzen_pstates -p=1 -f=102 -d=12 -v=96" << std::endl;
}

void updatePstate(const Params& params, int numThreads)
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
	// we create a thread affinity mask for all the threads on the system, and then
	// loop and set the msr with a separate mask for each individual thread
	DWORD_PTR maxMask = -1; // initialize all bits to one

	// shift bits to represent actual number of threads in system
	maxMask = maxMask >> (sizeof(DWORD_PTR) * 8 - numThreads);

	// affinity mask for one thread each
	DWORD_PTR mask = 0x1;

	while (mask < 0xFF) {
		WrmsrTx(PSTATES[params.pstate], eax, edx, mask);
		mask = mask << 1;
	}

	std::cout << "Pstate updated" << std::endl;
}

int getNumberOfHardwareThreads()
{
	int numHardwareThreads = std::thread::hardware_concurrency();

	if (numHardwareThreads == 0)
	{
		throw std::runtime_error("The number of hardware threads could not be determined");
	}

	std::cout << "Detected " << numHardwareThreads << "hardware threads on CPU" << std::endl;

	// check if we have more threads than we can mask with the size of DWORD_PTR
	if (numHardwareThreads > sizeof(DWORD_PTR) * 8)
	{
		// with so many threads, windows probably has multiple processor groups
		// https://docs.microsoft.com/en-us/windows/win32/procthread/processor-groups
		// this would need additional code to implement, since WinRing0 uses SetThreadAffinityMask,
		// which can't change processor groups and we need to set pstates on every thread
		throw std::runtime_error("The number of hardware threads is too high, this is not yet supported");
	}

	return numHardwareThreads;
}
