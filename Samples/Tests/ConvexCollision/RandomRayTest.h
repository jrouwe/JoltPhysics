// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Tests a lot of random rays against convex shapes
class RandomRayTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, RandomRayTest)

	// Update the test, called before the physics update
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	template <typename A, typename Context>
	void			TestRay(const char *inTestName, RVec3Arg inRenderOffset, const A &inA, const Context &inContext, float (*inCompareFunc)(const Context &inContext, Vec3Arg inRayOrigin, Vec3Arg inRayDirection));
};
