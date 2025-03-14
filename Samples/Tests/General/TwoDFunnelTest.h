// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class TwoDFunnelTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, TwoDFunnelTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return "Shows how to create a 2D simulation.";
	}

	// See: Test
	virtual void		Initialize() override;
};
