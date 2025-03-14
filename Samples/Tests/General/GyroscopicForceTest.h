// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

class GyroscopicForceTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, GyroscopicForceTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		// See: https://en.wikipedia.org/wiki/Tennis_racket_theorem
		return "Shows how to enable gyroscopic forces to create the Dzhanibekov effect.";
	}

	// See: Test
	virtual void		Initialize() override;
};
