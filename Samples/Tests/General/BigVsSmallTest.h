// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class BigVsSmallTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, BigVsSmallTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return "A small box falling on a big triangle to test for numerical precision errors.";
	}

	// See: Test
	virtual void		Initialize() override;
};
