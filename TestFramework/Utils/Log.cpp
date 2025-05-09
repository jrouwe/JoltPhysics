// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>
#include <Utils/Log.h>
#include <cstdarg>

// Trace to TTY
void TraceImpl(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[2048];
	vsnprintf(buffer, sizeof(buffer) - 1 /* leave space for newline char */, inFMT, list);
	va_end(list);

#ifdef JPH_PLATFORM_WINDOWS
	// Log to the output window
	strcat_s(buffer, "\n");
	OutputDebugStringA(buffer);
#else
	// Log to the console
	printf("%s\n", buffer);
#endif

}

void Alert(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	Trace("Alert: %s", buffer);

#ifdef JPH_PLATFORM_WINDOWS
	MessageBoxA(nullptr, buffer, "Alert", MB_OK);
#endif // JPH_PLATFORM_WINDOWS
}

void FatalError [[noreturn]] (const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	Trace("Fatal Error: %s", buffer);

#ifdef JPH_PLATFORM_WINDOWS
	MessageBoxA(nullptr, buffer, "Fatal Error", MB_OK);
#endif // JPH_PLATFORM_WINDOWS
	exit(1);
}
