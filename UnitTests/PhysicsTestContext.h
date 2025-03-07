// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Constraints/TwoBodyConstraint.h>
#include "Layers.h"

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <fstream>
JPH_SUPPRESS_WARNINGS_STD_END

namespace JPH {
	class TempAllocator;
	class JobSystem;
	class DebugRendererRecorder;
	class StreamOutWrapper;
};

// Helper class used in test cases for creating and manipulating physics objects
class PhysicsTestContext
{
public:
	// Constructor / destructor
						PhysicsTestContext(float inDeltaTime = 1.0f / 60.0f, int inCollisionSteps = 1, int inWorkerThreads = 0, uint inMaxBodies = 1024, uint inMaxBodyPairs = 4096, uint inMaxContactConstraints = 1024);
						~PhysicsTestContext();

	// Set the gravity to zero
	void				ZeroGravity();

	// Create a floor at Y = 0
	Body &				CreateFloor();

	/// Create a body and add it to the world
	Body &				CreateBody(const ShapeSettings *inShapeSettings, RVec3Arg inPosition, QuatArg inRotation, EMotionType inMotionType, EMotionQuality inMotionQuality, ObjectLayer inLayer, EActivation inActivation);

	// Create a box and add it to the world
	Body &				CreateBox(RVec3Arg inPosition, QuatArg inRotation, EMotionType inMotionType, EMotionQuality inMotionQuality, ObjectLayer inLayer, Vec3Arg inHalfExtent, EActivation inActivation = EActivation::Activate);

	// Create a sphere and add it to the world
	Body &				CreateSphere(RVec3Arg inPosition, float inRadius, EMotionType inMotionType, EMotionQuality inMotionQuality, ObjectLayer inLayer, EActivation inActivation = EActivation::Activate);

	// Create a constraint and add it to the world
	template <typename T>
	T &					CreateConstraint(Body &inBody1, Body &inBody2, const TwoBodyConstraintSettings &inSettings)
	{
		T *constraint = static_cast<T *>(inSettings.Create(inBody1, inBody2));
		mSystem->AddConstraint(constraint);
		return *constraint;
	}

	// Call the update with zero delta time
	EPhysicsUpdateError SimulateNoDeltaTime();

	// Simulate only for one delta time step
	EPhysicsUpdateError	SimulateSingleStep();

	// Simulate the world for inTotalTime time
	EPhysicsUpdateError	Simulate(float inTotalTime, function<void()> inPreStepCallback = []() { });

	// Predict position assuming ballistic motion using initial position, velocity acceleration and time
	RVec3				PredictPosition(RVec3Arg inPosition, Vec3Arg inVelocity, Vec3Arg inAcceleration, float inTotalTime) const;

	// Predict rotation assuming ballistic motion using initial orientation, angular velocity angular acceleration and time
	Quat				PredictOrientation(QuatArg inRotation, Vec3Arg inAngularVelocity, Vec3Arg inAngularAcceleration, float inTotalTime) const;

	// Access to the physics system
	PhysicsSystem *		GetSystem() const
	{
		return mSystem;
	}

	// Access to the body interface
	BodyInterface &		GetBodyInterface() const
	{
		return mSystem->GetBodyInterface();
	}

	// Get delta time for simulation step
	inline float		GetDeltaTime() const
	{
		return mDeltaTime;
	}

	// Get delta time for a simulation collision step
	inline float		GetStepDeltaTime() const
	{
		return mDeltaTime / mCollisionSteps;
	}

	// Get the temporary allocator
	TempAllocator *		GetTempAllocator() const
	{
		return mTempAllocator;
	}

	// Get the job system
	JobSystem *			GetJobSystem() const
	{
		return mJobSystem;
	}

#ifdef JPH_DEBUG_RENDERER
	// Write the debug output to a file to be able to replay it with JoltViewer
	void				RecordDebugOutput(const char *inFileName);
#endif // JPH_DEBUG_RENDERER

private:
	TempAllocator *		mTempAllocator;
	JobSystem *			mJobSystem;
	BPLayerInterfaceImpl mBroadPhaseLayerInterface;
	ObjectVsBroadPhaseLayerFilterImpl mObjectVsBroadPhaseLayerFilter;
	ObjectLayerPairFilterImpl mObjectVsObjectLayerFilter;
	PhysicsSystem *		mSystem;
#ifdef JPH_DEBUG_RENDERER
	DebugRendererRecorder *mDebugRenderer = nullptr;
	ofstream *			mStream = nullptr;
	StreamOutWrapper *	mStreamWrapper = nullptr;
#endif // JPH_DEBUG_RENDERER
	float				mDeltaTime;
	int					mCollisionSteps;
};
