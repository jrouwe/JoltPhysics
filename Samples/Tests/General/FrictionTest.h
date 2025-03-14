// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class FrictionTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, FrictionTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return "Bodies with varying friction.";
	}

	// See: Test
	virtual void		Initialize() override;
};
