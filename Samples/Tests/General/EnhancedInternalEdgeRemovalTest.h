// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class EnhancedInternalEdgeRemovalTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, EnhancedInternalEdgeRemovalTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return "Shows bodies using enhanced edge removal vs bodies that don't.";
	}

	// See: Test
	virtual void	Initialize() override;
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	void			CreateSlidingObjects(RVec3Arg inStart);

	BodyID			mLevelBall;
};
