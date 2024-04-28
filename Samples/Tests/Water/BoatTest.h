// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Collision/ContactListener.h>

// This test shows how a boat could be constructed.
class BoatTest : public Test, public ContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, BoatTest)

	// If this test implements a contact listener, it should be returned here
	virtual ContactListener *	GetContactListener() override		{ return this; }

	// See: Test
	virtual void				Initialize() override;
	virtual void				ProcessInput(const ProcessInputParams &inParams) override;
	virtual void				PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void				SaveInputState(StateRecorder &inStream) const override;
	virtual void				RestoreInputState(StateRecorder &inStream) override;
	virtual void				SaveState(StateRecorder &inStream) const override;
	virtual void				RestoreState(StateRecorder &inStream) override;
	virtual void				GetInitialCamera(CameraState &ioState) const override;
	virtual RMat44				GetCameraPivot(float inCameraHeading, float inCameraPitch) const override { return mCameraPivot; }

	// See: ContactListener
	virtual void				OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;
	virtual void				OnContactRemoved(const SubShapeIDPair &inSubShapePair) override;

private:
	void						UpdateCameraPivot();

	// Determines the water surface position at a given XZ position
	RVec3						GetWaterSurfacePosition(RVec3Arg inXZPosition) const;

	// Configuration
	static constexpr float		cMaxWaterHeight = 5.0f;
	static constexpr float		cMinWaterHeight = 3.0f;
	static constexpr float		cWaterWidth = 100.0f;

	static constexpr float		cHalfBoatLength = 4.0f;
	static constexpr float		cHalfBoatTopWidth = 1.5f;
	static constexpr float		cHalfBoatBottomWidth = 1.2f;
	static constexpr float		cBoatBowLength = 2.0f;
	static constexpr float		cHalfBoatHeight = 0.75f;

	static constexpr float		cBoatMass = 1000.0f;
	static constexpr float		cBoatBuoyancy = 3.0f;
	static constexpr float		cBoatLinearDrag = 0.5f;
	static constexpr float		cBoatAngularDrag = 0.7f;

	static constexpr float		cBarrelMass = 50.0f;
	static constexpr float		cBarrelBuoyancy = 1.5f;
	static constexpr float		cBarrelLinearDrag = 0.5f;
	static constexpr float		cBarrelAngularDrag = 0.1f;

	static constexpr float		cForwardAcceleration = 15.0f;
	static constexpr float		cSteerAcceleration = 1.5f;

	// The boat
	Body *						mBoatBody;

	// The sensor that detects objects in the water
	BodyID						mWaterSensor;

	// The camera pivot, recorded before the physics update to align with the drawn world
	RMat44						mCameraPivot = RMat44::sIdentity();

	// Keeping track of which bodies are in the water
	Mutex						mBodiesInWaterMutex;
	BodyIDVector				mBodiesInWater;

	// Time
	float						mTime = 0.0f;

	// Player input
	float						mForward = 0.0f;
	float						mRight = 0.0f;
};
