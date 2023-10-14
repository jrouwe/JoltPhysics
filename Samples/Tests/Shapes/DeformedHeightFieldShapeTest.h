// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>

// This test shows how to deform a height field shape after it has been created
class DeformedHeightFieldShapeTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, DeformedHeightFieldShapeTest)

	// Initialize the test
	virtual void			Initialize() override;

	// Update the test, called before the physics update
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// Test will never be deterministic since we're modifying the height field shape and not saving it
	virtual bool			IsDeterministic() const override							{ return false; }

private:
	static constexpr int	cSampleCount = 128;
	static constexpr int	cBlockSize = 4;

	Array<float>			mHeightSamples;
	Ref<HeightFieldShape>	mHeightField;
	BodyID					mTerrainID;
	float					mTime = 0.0f;
};
