// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test tests a body that activates during the simulation step to check if it does collision detection with any other bodies during that step
// To do so it uses 3 boxes that all initially collide. The left most box is the only one awake and has a high velocity.
// The second box should not pass through the third box.
class ActivateDuringUpdateTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ActivateDuringUpdateTest)

	// Initialize the test
	virtual void			Initialize() override;
};
