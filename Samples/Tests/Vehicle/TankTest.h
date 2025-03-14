// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Vehicle/VehicleTest.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>

class TankTest : public VehicleTest
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, TankTest)

	// Description of the test
	virtual const char *		GetDescription() const override
	{
		return	"Shows how a tank could be made with a vehicle constraint.\n"
				"Use the arrow keys to drive. Shift to brake. Enter to fire.";
	}

	// Destructor
	virtual						~TankTest() override;

	// See: Test
	virtual void				Initialize() override;
	virtual void				ProcessInput(const ProcessInputParams &inParams) override;
	virtual void				PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void				SaveState(StateRecorder &inStream) const override;
	virtual void				RestoreState(StateRecorder &inStream) override;
	virtual void				SaveInputState(StateRecorder &inStream) const override;
	virtual void				RestoreInputState(StateRecorder &inStream) override;

	virtual void				GetInitialCamera(CameraState &ioState) const override;
	virtual RMat44				GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;

private:
	Body *						mTankBody;									///< The body of the tank
	Body *						mTurretBody;								///< The body of the turret of the tank
	Body *						mBarrelBody;								///< The body of the barrel of the tank
	Ref<VehicleConstraint>		mVehicleConstraint;							///< The vehicle constraint
	Ref<HingeConstraint>		mTurretHinge;								///< Hinge connecting tank body and turret
	Ref<HingeConstraint>		mBarrelHinge;								///< Hinge connecting tank turret and barrel
	float						mReloadTime = 0.0f;							///< How long it still takes to reload the main gun
	RVec3						mCameraPivot = RVec3::sZero();				///< The camera pivot, recorded before the physics update to align with the drawn world

	// Player input
	float						mForward = 0.0f;
	float						mPreviousForward = 1.0f;					///< Keeps track of last car direction so we know when to brake and when to accelerate
	float						mLeftRatio = 0.0f;
	float						mRightRatio = 0.0f;
	float						mBrake = 0.0f;
	float						mTurretHeading = 0.0f;
	float						mBarrelPitch = 0.0f;
	bool						mFire = false;
};
