// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Constraints/ContactConstraintManager.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>

PhysicsTestContext::PhysicsTestContext(float inDeltaTime, int inCollisionSteps, int inIntegrationSubSteps, int inWorkerThreads) :
#ifdef JPH_DISABLE_TEMP_ALLOCATOR
	mTempAllocator(new TempAllocatorMalloc()),
#else
	mTempAllocator(new TempAllocatorImpl(4 * 1024 * 1024)),
#endif
	mJobSystem(new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, inWorkerThreads)),
	mDeltaTime(inDeltaTime),
	mCollisionSteps(inCollisionSteps),
	mIntegrationSubSteps(inIntegrationSubSteps)
{
	// Create physics system
	mSystem = new PhysicsSystem();
	mSystem->Init(1024, 0, 4096, 1024, mBroadPhaseLayerInterface, BroadPhaseCanCollide, ObjectCanCollide);
}

PhysicsTestContext::~PhysicsTestContext()
{
	delete mSystem;
	delete mJobSystem;
	delete mTempAllocator;
}

void PhysicsTestContext::ZeroGravity()
{
	mSystem->SetGravity(Vec3::sZero());
}

Body &PhysicsTestContext::CreateFloor()
{
	BodyCreationSettings settings;
	settings.SetShape(new BoxShape(Vec3(100.0f, 1.0f, 100.0f), 0.0f));
	settings.mPosition = Vec3(0.0f, -1.0f, 0.0f);
	settings.mMotionType = EMotionType::Static;
	settings.mObjectLayer = Layers::NON_MOVING;

	Body &floor = *mSystem->GetBodyInterface().CreateBody(settings);
	mSystem->GetBodyInterface().AddBody(floor.GetID(), EActivation::DontActivate);
	return floor;
}

Body &PhysicsTestContext::CreateBody(const ShapeSettings *inShapeSettings, Vec3Arg inPosition, QuatArg inRotation, EMotionType inMotionType, EMotionQuality inMotionQuality, ObjectLayer inLayer, EActivation inActivation)
{
	BodyCreationSettings settings;
	settings.SetShapeSettings(inShapeSettings);
	settings.mPosition = inPosition;
	settings.mRotation = inRotation;
	settings.mMotionType = inMotionType;
	settings.mMotionQuality = inMotionQuality;
	settings.mObjectLayer = inLayer;
	settings.mLinearDamping = 0.0f;
	settings.mAngularDamping = 0.0f;
	settings.mCollisionGroup.SetGroupID(0);
	Body &body = *mSystem->GetBodyInterface().CreateBody(settings);
	mSystem->GetBodyInterface().AddBody(body.GetID(), inActivation);
	return body;
}

Body &PhysicsTestContext::CreateBox(Vec3Arg inPosition, QuatArg inRotation, EMotionType inMotionType, EMotionQuality inMotionQuality, ObjectLayer inLayer, Vec3Arg inHalfExtent, EActivation inActivation)
{
	return CreateBody(new BoxShapeSettings(inHalfExtent), inPosition, inRotation, inMotionType, inMotionQuality, inLayer, inActivation);
}

Body &PhysicsTestContext::CreateSphere(Vec3Arg inPosition, float inRadius, EMotionType inMotionType, EMotionQuality inMotionQuality, ObjectLayer inLayer, EActivation inActivation)
{
	return CreateBody(new SphereShapeSettings(inRadius), inPosition, Quat::sIdentity(), inMotionType, inMotionQuality, inLayer, inActivation);
}

void PhysicsTestContext::Simulate(float inTotalTime, function<void()> inPreStepCallback)
{
	const int cNumSteps = int(round(inTotalTime / mDeltaTime));
	for (int s = 0; s < cNumSteps; ++s)
	{
		inPreStepCallback();
		mSystem->Update(mDeltaTime, mCollisionSteps, mIntegrationSubSteps, mTempAllocator, mJobSystem);
	#ifndef JPH_DISABLE_TEMP_ALLOCATOR
		JPH_ASSERT(static_cast<TempAllocatorImpl *>(mTempAllocator)->IsEmpty());
	#endif // JPH_DISABLE_TEMP_ALLOCATOR
	}
}

void PhysicsTestContext::SimulateSingleStep()
{
	mSystem->Update(mDeltaTime, mCollisionSteps, mIntegrationSubSteps, mTempAllocator, mJobSystem);
#ifndef JPH_DISABLE_TEMP_ALLOCATOR
	JPH_ASSERT(static_cast<TempAllocatorImpl *>(mTempAllocator)->IsEmpty());
#endif // JPH_DISABLE_TEMP_ALLOCATOR
}

Vec3 PhysicsTestContext::PredictPosition(Vec3Arg inPosition, Vec3Arg inVelocity, Vec3Arg inAcceleration, float inTotalTime) const
{
	// Integrate position using a Symplectic Euler step (just like the PhysicsSystem)
	Vec3 pos = inPosition;
	Vec3 vel = inVelocity;

	const float delta_time = GetSubStepDeltaTime();
	const int cNumSteps = int(round(inTotalTime / delta_time));
	for (int s = 0; s < cNumSteps; ++s)
	{
		vel += inAcceleration * delta_time;
		pos += vel * delta_time;
	}
	return pos;
}

// Predict rotation assuming ballistic motion using initial orientation, angular velocity angular acceleration and time
Quat PhysicsTestContext::PredictOrientation(QuatArg inRotation, Vec3Arg inAngularVelocity, Vec3Arg inAngularAcceleration, float inTotalTime) const
{
	// Integrate position using a Symplectic Euler step (just like the PhysicsSystem)
	Quat rot = inRotation;
	Vec3 vel = inAngularVelocity;

	const float delta_time = GetSubStepDeltaTime();
	const int cNumSteps = int(round(inTotalTime / delta_time));
	for (int s = 0; s < cNumSteps; ++s)
	{
		vel += inAngularAcceleration * delta_time;
		float vel_len = vel.Length();
		if (vel_len != 0.0f)
			rot = Quat::sRotation(vel / vel_len, vel_len * delta_time) * rot;
	}
	return rot;
}
