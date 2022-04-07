// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Core/TickCounter.h>

#if defined(JPH_PLATFORM_WINDOWS)
	JPH_SUPPRESS_WARNING_PUSH
	JPH_MSVC_SUPPRESS_WARNING(5039) // winbase.h(13179): warning C5039: 'TpSetCallbackCleanupGroup': pointer or reference to potentially throwing function passed to 'extern "C"' function under -EHc. Undefined behavior may occur if this function throws an exception.
	#define WIN32_LEAN_AND_MEAN
	#include <Windows.h>
	JPH_SUPPRESS_WARNING_POP
#elif defined(JPH_PLATFORM_LINUX) || defined(JPH_PLATFORM_ANDROID)
	#include <fstream>
#elif defined(JPH_PLATFORM_MACOS) || defined(JPH_PLATFORM_IOS)
	#include <sys/types.h>
	#include <sys/sysctl.h>
#endif

JPH_NAMESPACE_BEGIN

#ifdef JPH_PLATFORM_WINDOWS_UWP

uint64 GetProcessorTickCount()
{
	LARGE_INTEGER count;
	QueryPerformanceCounter(&count);
	return uint64(count.QuadPart);
}

#endif // JPH_PLATFORM_WINDOWS_UWP

static const uint64 sProcessorTicksPerSecond = []() {
#if defined(JPH_PLATFORM_WINDOWS_UWP)
	LARGE_INTEGER frequency { };
	QueryPerformanceFrequency(&frequency);
	return uint64(frequency.QuadPart);
#elif defined(JPH_PLATFORM_WINDOWS)
	// Open the key where the processor speed is stored
	HKEY hkey;
	RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, 1, &hkey);

	// Query the speed in MHz
	uint mhz = 0;
	DWORD mhz_size = sizeof(uint);
	RegQueryValueExA(hkey, "~MHz", nullptr, nullptr, (LPBYTE)&mhz, &mhz_size);

	// Close key
	RegCloseKey(hkey);

	// Initialize amount of cycles per second
	return uint64(mhz) * 1000000UL;
#elif defined(JPH_PLATFORM_BLUE)
	return JPH_PLATFORM_BLUE_GET_TICK_FREQUENCY();
#elif defined(JPH_PLATFORM_LINUX) || defined(JPH_PLATFORM_ANDROID) 
	// Open /proc/cpuinfo
    ifstream ifs("/proc/cpuinfo");
    if (ifs.is_open())
	{
		// Read all lines
		while (ifs.good())
		{
			// Get next line
			string line;
			getline(ifs, line);
		
		#if defined(JPH_CPU_X64)
			const char *cpu_str = "cpu MHz";
		#elif defined(JPH_CPU_ARM64)
			const char *cpu_str = "BogoMIPS";
		#else
			#error Unsupported CPU architecture
		#endif

			// Check if line starts with correct string
			const size_t num_chars = strlen(cpu_str);
			if (strncmp(line.c_str(), cpu_str, num_chars) == 0)
			{
				// Find ':'
				string::size_type pos = line.find(':', num_chars);
				if (pos != string::npos)
				{		
					// Convert to number
					string freq = line.substr(pos + 1);
					return uint64(stod(freq) * 1000000.0);
				}
			}
		}
	}

	JPH_ASSERT(false);
    return uint64(0);
#elif defined(JPH_PLATFORM_MACOS) || defined(JPH_PLATFORM_IOS)
	// Use sysctl to get the processor frequency
	int mib[2];
    mib[0] = CTL_HW;
    mib[1] = HW_CPU_FREQ;
    uint64 freq = 1;
    size_t len = sizeof(freq);
    sysctl(mib, 2, &freq, &len, nullptr, 0);
	return freq;
#else
	#error Undefined
#endif
}();

uint64 GetProcessorTicksPerSecond()
{
	return sProcessorTicksPerSecond;
}

JPH_NAMESPACE_END
