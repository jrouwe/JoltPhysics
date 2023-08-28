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
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);
	strcat_s(buffer, "\n");

	// Log to the output window
	OutputDebugStringA(buffer);
}

void FatalError [[noreturn]] (const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);

	Trace("Fatal Error: %s", buffer);

	MessageBoxA(nullptr, buffer, "Fatal Error", MB_OK);
	exit(1);
}
