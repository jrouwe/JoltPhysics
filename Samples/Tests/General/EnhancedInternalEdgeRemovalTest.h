// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

/// Demonstrates the enhanced internal edge removal mode by sliding two shapes over a mesh that has only active edges
class EnhancedInternalEdgeRemovalTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, EnhancedInternalEdgeRemovalTest)

	// See: Test
	virtual void	Initialize() override;
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	void			CreateSlidingObjects(RVec3Arg inStart);

	BodyID			mLevelBall;
};
