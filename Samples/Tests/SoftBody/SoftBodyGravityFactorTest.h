// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class SoftBodyGravityFactorTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftBodyGravityFactorTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return "Shows soft bodies with various gravity factor values.";
	}

	// See: Test
	virtual void		Initialize() override;
};
