// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Core/ScopeExit.h>

TEST_SUITE("ScopeExitTest")
{
	TEST_CASE("TestScopeExitOrder")
	{
		int value = 0;
		{
			// Last created should be first destroyed
			JPH_SCOPE_EXIT([&value]{ CHECK(value == 1); value = 2; });
			JPH_SCOPE_EXIT([&value]{ CHECK(value == 0); value = 1; });
			CHECK(value == 0);
		}
		CHECK(value == 2);
	}

	TEST_CASE("TestScopeExitRelease")
	{
		int value = 0;
		{
			ScopeExit scope_exit([&value]{ value++; });
			CHECK(value == 0);
			// Don't call the exit function anymore
			scope_exit.Release();
		}
		CHECK(value == 0);
	}

	TEST_CASE("TestScopeExitInvoke")
	{
		int value = 0;
		{
			ScopeExit scope_exit([&value]{ value++; });
			CHECK(value == 0);
			scope_exit.Invoke();
			CHECK(value == 1);
			// Should not call again on exit
		}
		CHECK(value == 1);
	}
}
