// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/DropSample.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h> // Assuming Layers.h is in the Samples directory or include path

JPH_IMPLEMENT_RTTI_VIRTUAL(DropSample)
{
	JPH_ADD_BASE_CLASS(DropSample, Test)
}

const char *DropSample::GetDescription() const
{
	return "A sample that simulates a dropping cube.";
}

void DropSample::Initialize()
{
	// Floor
	CreateFloor();

	// Create a box shape
	float box_half_extent = 2.0f;
	BoxShapeSettings box_shape_settings(Vec3(box_half_extent, box_half_extent, box_half_extent));

	// Create the shape
	ShapeSettings::ShapeResult box_shape_result = box_shape_settings.Create();
	ShapeRefC box_shape = box_shape_result.Get(); // We don't expect an error here, but you can check box_shape_result for HasError() / GetError()

	// Create the settings for the body itself.
	BodyCreationSettings box_settings(box_shape, RVec3(0.0_r, 20.0_r, 0.0_r), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);

	// Create the actual rigid body
	mBodyInterface->CreateAndAddBody(box_settings, EActivation::Activate);
}

void DropSample::GetInitialCamera(CameraState& ioState) const
{
	// Position the camera looking at the origin
	ioState.mPos = RVec3(0, 25, 25);
	ioState.mForward = Vec3(0, -0.5f, -1).Normalized();
}
