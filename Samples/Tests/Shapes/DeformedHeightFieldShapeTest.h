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
	// Get the center of the path at time inTime, this follows a path that resembles the Jolt logo
	Vec3					GetPathCenter(float inTime) const;

	// Size of the terrain
	static constexpr int	cSampleCount = 128;

	// Size of a block in the terrain
	static constexpr int	cBlockSize = 4;

	// Bits to mask out index within a block
	static constexpr int	cBlockMask = cBlockSize - 1;

	// The list of original height samples, we keep this to avoid precision loss of repeatedly decompressing and recompressing height samples
	Array<float>			mHeightSamples;

	// The height field shape
	Ref<HeightFieldShape>	mHeightField;

	// ID of the height field body
	BodyID					mHeightFieldID;

	// Current time
	float					mTime = 0.0f;
};
