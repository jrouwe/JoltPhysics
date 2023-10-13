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
	virtual void		Initialize() override;

	// Update the test, called before the physics update
	virtual void		PrePhysicsUpdate(const PreUpdateParams &inParams) override;

private:
	Ref<HeightFieldShape> mHeightField;
};
