// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/SoftBody/SoftBodySensorTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/SoftBody/SoftBodyManifold.h>
#include <Utils/SoftBodyCreator.h>
#include <Renderer/DebugRendererImp.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SoftBodySensorTest)
{
	JPH_ADD_BASE_CLASS(SoftBodySensorTest, Test)
}

void SoftBodySensorTest::Initialize()
{
	// Install contact listener for soft bodies
	mPhysicsSystem->SetSoftBodyContactListener(this);

	// Floor
	CreateFloor();

	// Create cloth that's fixated at the corners
	SoftBodyCreationSettings cloth(SoftBodyCreator::CreateClothWithFixatedCorners(), RVec3(0, 10.0f, 0), Quat::sIdentity(), Layers::MOVING);
	mBodyInterface->CreateAndAddSoftBody(cloth, EActivation::Activate);

	// A sensor to detect the cloth
	BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(5.0f)), RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Static, Layers::SENSOR);
	sensor_settings.mIsSensor = true;
	mSensorID = mBodyInterface->CreateAndAddBody(sensor_settings, EActivation::DontActivate);

	// Sphere that falls on the cloth to check that we don't ignore this collision
	BodyCreationSettings bcs(new SphereShape(1.0f), RVec3(0, 15, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
	bcs.mMassPropertiesOverride.mMass = 100.0f;
	mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
}

void SoftBodySensorTest::OnSoftBodyContactAdded(const Body &inSoftBody, const SoftBodyManifold &inManifold)
{
	// Draw the vertices that are in the sensor
	RMat44 com = inSoftBody.GetCenterOfMassTransform();
	for (const SoftBodyVertex &v : inManifold.GetVertices())
		if (inManifold.GetContactBodyID(v) == mSensorID)
			DebugRenderer::sInstance->DrawMarker(com * v.mPosition, Color::sGreen, 0.1f);
}
