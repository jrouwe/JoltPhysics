// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class LoadSaveBinaryTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, LoadSaveBinaryTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return "Tests the binary serialization system by creating a number of shapes, storing them, loading them and then simulating them.";
	}

	// See: Test
	virtual void		Initialize() override;
};
