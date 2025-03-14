// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class ContactManifoldTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ContactManifoldTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return "Spawns objects at an angle to test if the contact manifold is calculated correctly.";
	}

	// See: Test
	virtual void		Initialize() override;
};
