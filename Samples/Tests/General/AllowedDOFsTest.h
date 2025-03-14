// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class AllowedDOFsTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, AllowedDOFsTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return	"Shows all permutations of allowed degrees of freedom for a body (see EAllowedDOFs).\n"
				"The boxes are constrained to the world using a distance constraint, press C to show it.";
	}

	// See: Test
	virtual void		Initialize() override;

private:
	BodyIDVector		mBodies;
};
