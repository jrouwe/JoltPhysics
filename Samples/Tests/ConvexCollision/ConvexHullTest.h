// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Simple test to create a convex hull
class ConvexHullTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(ConvexHullTest)

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	// A list of predefined points to feed the convex hull algorithm
	using Points = vector<Vec3>;
	vector<Points>			mPoints;

	// Which index in the list we're currently using
	size_t					mIteration = 0;

	// If we run out of points, we start creating random points
	default_random_engine	mRandom { 12345 };
};
