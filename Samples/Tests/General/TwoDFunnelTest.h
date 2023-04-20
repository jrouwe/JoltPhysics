// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This is a test that shows how to create a 2D simulation
class TwoDFunnelTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, TwoDFunnelTest)

	// See: Test
	virtual void		Initialize() override;
};
