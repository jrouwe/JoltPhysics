// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/DynamicMeshTest.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>
#include <Utils/ShapeCreator.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(DynamicMeshTest)
{
	JPH_ADD_BASE_CLASS(DynamicMeshTest, Test)
}

void DynamicMeshTest::Initialize()
{
	// Floor
	CreateFloor();

	constexpr float cTorusRadius = 3.0f;
	constexpr float cTubeRadius = 1.0f;

	// Create torus
	RefConst<Shape> mesh_shape = ShapeCreator::CreateTorusMesh(cTorusRadius, cTubeRadius);
	BodyCreationSettings settings(mesh_shape, RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);

	// Mesh cannot calculate its mass, we must provide it
	settings.mOverrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided;
	settings.mMassPropertiesOverride.SetMassAndInertiaOfSolidBox(2.0f * Vec3(cTorusRadius, cTubeRadius, cTorusRadius), 1000.0f);

	mBodyInterface->CreateAndAddBody(settings, EActivation::Activate);

	// Wall of blocks
	RefConst<Shape> box_shape = new BoxShape(Vec3::sReplicate(0.5f));
	for (int i = 0; i < 7; ++i)
		for (int j = i / 2; j < 7 - (i + 1) / 2; ++j)
		{
			RVec3 position(-3.5f + j * 1.0f + (i & 1? 0.5f : 0.0f), 0.5f + i * 1.0f, 0.0f);
			Body &wall = *mBodyInterface->CreateBody(BodyCreationSettings(box_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
			mBodyInterface->AddBody(wall.GetID(), EActivation::Activate);
		}
}
