// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class DampingTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, DampingTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return "Tests various values for linear and angular damping.";
	}

	// See: Test
	virtual void		Initialize() override;
};
