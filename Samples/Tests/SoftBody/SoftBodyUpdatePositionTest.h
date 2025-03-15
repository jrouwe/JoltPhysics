// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class SoftBodyUpdatePositionTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftBodyUpdatePositionTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return	"This test tests soft bodies with and without 'update position' and 'make rotation identity'.\n"
				"The labels of the bodies that don't update their position will stay in place.\n"
				"If you turn on 'Draw Bounding Boxes' then you will see that the cubes that with 'make rotation identity' have a smaller bounding box.";
	}

	// See: Test
	virtual void		Initialize() override;
};
