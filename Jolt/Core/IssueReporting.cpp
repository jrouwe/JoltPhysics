// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

JPH_NAMESPACE_BEGIN

static void DummyTrace([[maybe_unused]] const char *inFMT, ...)
{
	// Please override the 'Trace' function pointer with your own implementation so you can see the output of the physics system.
	// Triggering a breakpoint to make it obvious that this needs to be done.
	JPH_ASSERT(false, "Please override Trace");
};

TraceFunction Trace = DummyTrace;

#ifdef JPH_ENABLE_ASSERTS

static bool DummyAssertFailed(const char *inExpression, const char *inMessage, const char *inFile, uint inLine)
{
	return true; // Trigger breakpoint
};

AssertFailedFunction AssertFailed = DummyAssertFailed;

#endif // JPH_ENABLE_ASSERTS

JPH_NAMESPACE_END
