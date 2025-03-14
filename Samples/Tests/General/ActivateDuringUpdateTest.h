// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class ActivateDuringUpdateTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ActivateDuringUpdateTest)

	// Description of the test
	virtual const char *	GetDescription() const override
	{
		return	"Three initially colliding boxes where only 1 is awake and has a high velocity.\n"
				"The 2nd and 3rd box should wake up at the same time and not pass through each other.";
	}

	// Initialize the test
	virtual void			Initialize() override;
};
