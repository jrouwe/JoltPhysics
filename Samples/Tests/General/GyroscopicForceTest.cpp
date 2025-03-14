// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/GyroscopicForceTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(GyroscopicForceTest)
{
	JPH_ADD_BASE_CLASS(GyroscopicForceTest, Test)
}

void GyroscopicForceTest::Initialize()
{
	// Floor
	CreateFloor();

	StaticCompoundShapeSettings compound;
	compound.AddShape(Vec3::sZero(), Quat::sIdentity(), new BoxShape(Vec3(0.5f, 5.0f, 0.5f)));
	compound.AddShape(Vec3(1.5f, 0, 0), Quat::sIdentity(), new BoxShape(Vec3(1.0f, 0.5f, 0.5f)));
	compound.SetEmbedded();

	// One body without gyroscopic force
	BodyCreationSettings bcs(&compound, RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
	bcs.mLinearDamping = 0.0f;
	bcs.mAngularDamping = 0.0f;
	bcs.mAngularVelocity = Vec3(10, 1, 0);
	bcs.mGravityFactor = 0.0f;
	BodyID id = mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
	SetBodyLabel(id, "Gyroscopic force off");

	// One body with gyroscopic force
	bcs.mPosition += RVec3(10, 0, 0);
	bcs.mApplyGyroscopicForce = true;
	id = mBodyInterface->CreateAndAddBody(bcs, EActivation::Activate);
	SetBodyLabel(id, "Gyroscopic force on");
}
