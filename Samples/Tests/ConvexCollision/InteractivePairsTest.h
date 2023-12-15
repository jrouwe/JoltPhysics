// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// Renders pairs of objects and their collisions. Use Z, X, C keys to control distance.
class InteractivePairsTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, InteractivePairsTest)

	// Process input
	virtual void	ProcessInput(const ProcessInputParams &inParams) override;

	// Update the test, called before the physics update
	virtual void	PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	void			TestBoxVsBox(Vec3Arg inTranslationA, Vec3Arg inRotationA, float inConvexRadiusA, const AABox &inA, Vec3Arg inTranslationB, Vec3Arg inRotationB, float inConvexRadiusB, const AABox &inB);
	void			TestSphereVsBox(Vec3Arg inTranslationA, float inRadiusA, Vec3Arg inTranslationB, Vec3Arg inRotationB, float inConvexRadiusB, const AABox &inB);
	void			TestSphereVsSphere(Vec3Arg inTranslationA, float inRadiusA, Vec3Arg inTranslationB, float inRadiusB, bool inTreatSphereAsPointWithConvexRadius);

	bool			mKeyboardMode = false;
	float			mDistance = 3.0f;
};
