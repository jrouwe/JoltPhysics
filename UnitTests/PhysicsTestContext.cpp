// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Constraints/ContactConstraintManager.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Core/JobSystemThreadPool.h>
#include <Jolt/Core/TempAllocator.h>
#include <Jolt/Core/StreamWrapper.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRendererRecorder.h>
#endif

PhysicsTestContext::PhysicsTestContext(float inDeltaTime, int inCollisionSteps, int inWorkerThreads, uint inMaxBodies, uint inMaxBodyPairs, uint inMaxContactConstraints) :
#ifdef JPH_DISABLE_TEMP_ALLOCATOR
	mTempAllocator(new TempAllocatorMalloc()),
#else
	mTempAllocator(new TempAllocatorImpl(4 * 1024 * 1024)),
#endif
	mJobSystem(new JobSystemThreadPool(cMaxPhysicsJobs, cMaxPhysicsBarriers, inWorkerThreads)),
	mDeltaTime(inDeltaTime),
	mCollisionSteps(inCollisionSteps)
{
	// Create physics system
	mSystem = new PhysicsSystem();
	mSystem->Init(inMaxBodies, 0, inMaxBodyPairs, inMaxContactConstraints, mBroadPhaseLayerInterface, mObjectVsBroadPhaseLayerFilter, mObjectVsObjectLayerFilter);
}

PhysicsTestContext::~PhysicsTestContext()
{
#ifdef JPH_DEBUG_RENDERER
	delete mDebugRenderer;
	delete mStreamWrapper;
	delete mStream;
#endif // JPH_DEBUG_RENDERER
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
	settings.mPosition = RVec3(0.0f, -1.0f, 0.0f);
	settings.mMotionType = EMotionType::Static;
	settings.mObjectLayer = Layers::NON_MOVING;

	Body &floor = *mSystem->GetBodyInterface().CreateBody(settings);
	mSystem->GetBodyInterface().AddBody(floor.GetID(), EActivation::DontActivate);
	return floor;
}

Body &PhysicsTestContext::CreateBody(const ShapeSettings *inShapeSettings, RVec3Arg inPosition, QuatArg inRotation, EMotionType inMotionType, EMotionQuality inMotionQuality, ObjectLayer inLayer, EActivation inActivation)
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

Body &PhysicsTestContext::CreateBox(RVec3Arg inPosition, QuatArg inRotation, EMotionType inMotionType, EMotionQuality inMotionQuality, ObjectLayer inLayer, Vec3Arg inHalfExtent, EActivation inActivation)
{
	return CreateBody(new BoxShapeSettings(inHalfExtent), inPosition, inRotation, inMotionType, inMotionQuality, inLayer, inActivation);
}

Body &PhysicsTestContext::CreateSphere(RVec3Arg inPosition, float inRadius, EMotionType inMotionType, EMotionQuality inMotionQuality, ObjectLayer inLayer, EActivation inActivation)
{
	return CreateBody(new SphereShapeSettings(inRadius), inPosition, Quat::sIdentity(), inMotionType, inMotionQuality, inLayer, inActivation);
}

EPhysicsUpdateError PhysicsTestContext::SimulateSingleStep()
{
	EPhysicsUpdateError errors = mSystem->Update(mDeltaTime, mCollisionSteps, mTempAllocator, mJobSystem);
#ifndef JPH_DISABLE_TEMP_ALLOCATOR
	JPH_ASSERT(static_cast<TempAllocatorImpl *>(mTempAllocator)->IsEmpty());
#endif // JPH_DISABLE_TEMP_ALLOCATOR
#ifdef JPH_DEBUG_RENDERER
	if (mDebugRenderer != nullptr)
	{
		mSystem->DrawBodies(BodyManager::DrawSettings(), mDebugRenderer);
		mSystem->DrawConstraints(mDebugRenderer);
		mDebugRenderer->EndFrame();
	}
#endif // JPH_DEBUG_RENDERER
	return errors;
}

EPhysicsUpdateError PhysicsTestContext::Simulate(float inTotalTime, function<void()> inPreStepCallback)
{
	EPhysicsUpdateError errors = EPhysicsUpdateError::None;

	const int cNumSteps = int(round(inTotalTime / mDeltaTime));
	for (int s = 0; s < cNumSteps; ++s)
	{
		inPreStepCallback();
		errors |= SimulateSingleStep();
	}

	return errors;
}

RVec3 PhysicsTestContext::PredictPosition(RVec3Arg inPosition, Vec3Arg inVelocity, Vec3Arg inAcceleration, float inTotalTime) const
{
	// Integrate position using a Symplectic Euler step (just like the PhysicsSystem)
	RVec3 pos = inPosition;
	Vec3 vel = inVelocity;

	const float delta_time = GetStepDeltaTime();
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

	const float delta_time = GetStepDeltaTime();
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

#ifdef JPH_DEBUG_RENDERER

void PhysicsTestContext::RecordDebugOutput(const char *inFileName)
{
	mStream = new ofstream;
	mStream->open(inFileName, ofstream::out | ofstream::binary | ofstream::trunc);
	if (mStream->is_open())
	{
		mStreamWrapper = new StreamOutWrapper(*mStream);
		mDebugRenderer = new DebugRendererRecorder(*mStreamWrapper);
	}
	else
	{
		delete mStream;
		mStream = nullptr;
	}
}

#endif // JPH_DEBUG_RENDERER
