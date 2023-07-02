// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test tests all permutations of allowed degrees of freedom (see EAllowedDOFs)
class AllowedDOFsTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, AllowedDOFsTest)

	// See: Test
	virtual void		Initialize() override;
	virtual void		PostPhysicsUpdate(float inDeltaTime) override;

private:
	BodyIDVector		mBodies;
};
