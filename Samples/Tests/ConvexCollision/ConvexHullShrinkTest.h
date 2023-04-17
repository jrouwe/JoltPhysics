// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Create a convex hull, shrink it with the convex radius and expand it again to check the error
class ConvexHullShrinkTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ConvexHullShrinkTest)

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Number used to scale the terrain and camera movement to the scene
	virtual float			GetWorldScale() const override								{ return 0.2f; }

private:
	// A list of predefined points to feed the convex hull algorithm
	using Points = Array<Vec3>;
	Array<Points>			mPoints;

	// Which index in the list we're currently using
	size_t					mIteration = 0;
};
