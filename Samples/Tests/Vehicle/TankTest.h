// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Vehicle/VehicleTest.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>

// This test shows how a tank could be made with the vehicle constraint.
class TankTest : public VehicleTest
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(TankTest)

	// Destructor
	virtual						~TankTest() override;

	// See: Test
	virtual void				Initialize() override;
	virtual void				PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	virtual void				GetInitialCamera(CameraState &ioState) const override;
	virtual Mat44				GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;

private:
	Body *						mTankBody;									///< The body of the tank
	Body *						mTurretBody;								///< The body of the turret of the tank
	Body *						mBarrelBody;								///< The body of the barrel of the tank
	Ref<VehicleConstraint>		mVehicleConstraint;							///< The vehicle constraint
	Ref<HingeConstraint>		mTurretHinge;								///< Hinge connecting tank body and turret
	Ref<HingeConstraint>		mBarrelHinge;								///< Hinge connecting tank turret and barrel
	float						mPreviousForward = 1.0f;					///< Keeps track of last car direction so we know when to brake and when to accelerate
	float						mReloadTime = 0.0f;							///< How long it still takes to reload the main gun
};
