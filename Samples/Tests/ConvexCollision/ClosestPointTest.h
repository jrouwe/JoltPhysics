// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Test that interactively shows the algorithms from the ClosestPoints namespace
class ClosestPointTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ClosestPointTest)

	// Update the test, called before the physics update
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	void			TestLine(Vec3Arg inPosition, Vec3Arg inA, Vec3Arg inB);
	void			TestTri(Vec3Arg inPosition, Vec3Arg inA, Vec3Arg inB, Vec3Arg inC);
	void			TestTetra(Vec3Arg inPosition, Vec3Arg inA, Vec3Arg inB, Vec3Arg inC, Vec3Arg inD);
};
