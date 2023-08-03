// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test tests soft bodies with and without 'update position' and 'make rotation identity'
// If you turn on 'Draw World Transforms' you will see that 2 cubes will stay at their initial position.
// If you turn on 'Draw Bounding Boxes' then you will see that the cubes that didn't have 'make rotation identity' will have a bigger bounding box.
class SoftBodyUpdatePositionTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SoftBodyUpdatePositionTest)

	// See: Test
	virtual void		Initialize() override;
};
