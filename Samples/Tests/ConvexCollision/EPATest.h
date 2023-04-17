// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Does a single box vs sphere test without convex radius for visually debugging the EPA algorithm
class EPATest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, EPATest)

	// Update the test, called before the physics update
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	bool			CollideBoxSphere(Mat44Arg inMatrix, const AABox &inBox, const Sphere &inSphere) const;
};
