// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>
#include <Utils/Log.h>
#include <cstdarg>

#include <Cocoa/Cocoa.h>

// Trace to TTY
void TraceImpl(const char *inFMT, ...)
{
	// Format the message
	va_list list;
	va_start(list, inFMT);
	char buffer[1024];
	vsnprintf(buffer, sizeof(buffer), inFMT, list);
	va_end(list);

	// Log to the console
	printf("%s\n", buffer);
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

	NSAlert *alert = [[[NSAlert alloc] init] autorelease];
	alert.messageText = [NSString stringWithCString: buffer encoding: NSUTF8StringEncoding];
	[alert runModal];
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

	NSAlert *alert = [[[NSAlert alloc] init] autorelease];
	alert.messageText = [NSString stringWithCString: buffer encoding: NSUTF8StringEncoding];
	[alert runModal];
	
	exit(1);
}
