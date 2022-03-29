// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test spawns a lot of objects and drops them into a funnel to check for performance / stability issues
class FunnelTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(FunnelTest)

	// See: Test
	virtual void		Initialize() override;
	virtual void		GetInitialCamera(CameraState &ioState) const override;
};
