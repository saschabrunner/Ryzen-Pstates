#include "Cpuid.h"

#include <cstring>
#include <iostream>
#include <string>

// support GCC and MS VC++ compilers
#if defined(__GNUC__)
#include <cpuid.h>
#elif defined (_WIN32)
#include <intrin.h>
#endif

// constants
constexpr char CPU_MANUFACTURER_AMD[]{ "AuthenticAMD" };
constexpr unsigned int CPU_FAMILY_ZEN{ 0x17 };
constexpr unsigned int CPU_MODEL_RAVEN_RIDGE{ 0x11 };

// prototypes
static void cpuid(int registers[4], int level);

bool validateCpu()
{
	int registers[4]; // EAX, EBX, ECX, EDX
	cpuid(registers, 0); // get manufacturer id

	// manufacturer string is stored in EBX, EDX, ECX, in that order
	char manufacturer[13];
	memcpy(&manufacturer[0], &registers[1], sizeof(int));
	memcpy(&manufacturer[4], &registers[3], sizeof(int));
	memcpy(&manufacturer[8], &registers[2], sizeof(int));
	manufacturer[12] = '\0';

	std::string manufacturerStr(manufacturer);

	if (manufacturerStr != CPU_MANUFACTURER_AMD) {
		std::cerr << "CPU is not AMD" << std::endl;
		return false;
	}

	cpuid(registers, 1); // get family and model

	/* EAX Register
	 * |  31   30   29   28 | 27   26   25   24   23   22   21   20 | 19   18   17   16 |
	 * | Reserved           | Extended Family ID                    | Extended Model ID |

	 * |  15   14 | 13   12 | 11   10    9    8 |  7    6    5    4 |  3    2    1    0 |
	 * | Reserved | ProcType| Family ID         | Model             | Stepping ID       |
	 */
	unsigned int family = ((registers[0] >> 8) & 0xf) + ((registers[0] >> 20) & 0xff);
	unsigned int model = ((registers[0] >> 4) & 0xf) + (((registers[0] >> 16) & 0xf) << 4);

	if (family != CPU_FAMILY_ZEN) {
		std::cerr << "CPU is not AMD Zen" << std::endl;
		return false;
	}

	if (model != CPU_MODEL_RAVEN_RIDGE) {
		std::cerr << "CPU is not AMD Raven Ridge" << std::endl;
		return false;
	}

	return true;
}

static void cpuid(int registers[4], int level)
{
#if defined(__GNUC__)
	__cpuid((unsigned int)level,
		(unsigned int*)&registers[0],
		(unsigned int*)&registers[1],
		(unsigned int*)&registers[2],
		(unsigned int*)&registers[3]);
#elif defined(_WIN32)
	__cpuid(registers, level);
#endif
}
