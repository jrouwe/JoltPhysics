// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test shows interaction between a fast moving (CCD) object and a soft body
class SoftBodyVsFastMovingTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftBodyVsFastMovingTest)

	// See: Test
	virtual void		Initialize() override;
};
