// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>
#include <fstream>

namespace JPH {

static void DummyTrace(const char *inFMT, ...) 
{ 
	JPH_ASSERT(false); 
};

TraceFunction Trace = DummyTrace;

#ifdef JPH_ENABLE_ASSERTS

static bool DummyAssertFailed(const char *inExpression, const char *inMessage, const char *inFile, uint inLine)
{ 
	return true; // Trigger breakpoint
};

AssertFailedFunction AssertFailed = DummyAssertFailed;

#endif // JPH_ENABLE_ASSERTS

} // JPH