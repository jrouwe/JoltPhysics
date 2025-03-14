// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class SoftBodyVsFastMovingTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftBodyVsFastMovingTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return "Shows interaction between a fast moving (CCD) object and a soft body.";
	}

	// See: Test
	virtual void		Initialize() override;
};
