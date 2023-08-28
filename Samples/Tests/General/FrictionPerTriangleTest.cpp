// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/FrictionPerTriangleTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Geometry/Triangle.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(FrictionPerTriangleTest)
{
	JPH_ADD_BASE_CLASS(FrictionPerTriangleTest, Test)
}

void FrictionPerTriangleTest::Initialize()
{
	const int num_sections = 5;
	const float section_size = 50.0f;

	// Create a strip of triangles
	TriangleList triangles;
	for (int z = 0; z <= num_sections; ++z)
	{
		float z1 = section_size * (z - 0.5f * num_sections);
		float z2 = z1 + section_size;

		Float3 v1 = Float3(-100.0f, 0, z1);
		Float3 v2 = Float3(100.0f, 0, z1);
		Float3 v3 = Float3(-100.0f, 0, z2);
		Float3 v4 = Float3(100.0f, 0, z2);

		triangles.push_back(Triangle(v1, v3, v4, z));
		triangles.push_back(Triangle(v1, v4, v2, z));
	}

	// Create materials with increasing friction
	PhysicsMaterialList materials;
	for (uint i = 0; i <= num_sections; ++i)
	{
		float friction = float(i) / float(num_sections);
		materials.push_back(new MyMaterial("Friction " + ConvertToString(friction), Color::sGetDistinctColor(i), friction, 0.0f));
	}

	// A ramp
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new MeshShapeSettings(triangles, std::move(materials)), RVec3::sZero(), Quat::sRotation(Vec3::sAxisX(), 0.2f * JPH_PI), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// A box with friction 1 that slides down the ramp
	Ref<BoxShape> box_shape = new BoxShape(Vec3(2.0f, 2.0f, 2.0f), cDefaultConvexRadius, new MyMaterial("Box Friction 1", Color::sYellow, 1.0f, 0.0f));
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(box_shape, RVec3(0, 60.0f, -75.0f), Quat::sRotation(Vec3::sAxisX(), 0.2f * JPH_PI), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
}

void FrictionPerTriangleTest::sGetFrictionAndRestitution(const Body &inBody, const SubShapeID &inSubShapeID, float &outFriction, float &outRestitution)
{
	// Get the material that corresponds to the sub shape ID
	const PhysicsMaterial *material = inBody.GetShape()->GetMaterial(inSubShapeID);
	if (material == PhysicsMaterial::sDefault)
	{
		// This is the default material, use the settings from the body (note all bodies in our test have a material so this should not happen)
		outFriction = inBody.GetFriction();
		outRestitution = inBody.GetRestitution();
	}
	else
	{
		// If it's not the default material we know its a material that we created so we can cast it and get the values
		const MyMaterial *my_material = static_cast<const MyMaterial *>(material);
		outFriction = my_material->mFriction;
		outRestitution = my_material->mRestitution;
	}
}

void FrictionPerTriangleTest::sOverrideContactSettings(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	// Get the custom friction and restitution for both bodies
	float friction1, friction2, restitution1, restitution2;
	sGetFrictionAndRestitution(inBody1, inManifold.mSubShapeID1, friction1, restitution1);
	sGetFrictionAndRestitution(inBody2, inManifold.mSubShapeID2, friction2, restitution2);

	// Use the default formulas for combining friction and restitution
	ioSettings.mCombinedFriction = sqrt(friction1 * friction2);
	ioSettings.mCombinedRestitution = max(restitution1, restitution2);
}

void FrictionPerTriangleTest::OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	sOverrideContactSettings(inBody1, inBody2, inManifold, ioSettings);
}

void FrictionPerTriangleTest::OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	sOverrideContactSettings(inBody1, inBody2, inManifold, ioSettings);
}
