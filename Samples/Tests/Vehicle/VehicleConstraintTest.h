// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Vehicle/VehicleTest.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

// This test shows how a vehicle could be made with the vehicle constraint.
class VehicleConstraintTest : public VehicleTest
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(VehicleConstraintTest)

	// Destructor
	virtual						~VehicleConstraintTest() override;

	// See: Test
	virtual void				Initialize() override;
	virtual void				PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	virtual void				GetInitialCamera(CameraState &ioState) const override;
	virtual Mat44				GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;

	virtual void				CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	static int					sCollisionMode;

	Body *						mCarBody;									///< The vehicle
	Ref<VehicleConstraint>		mVehicleConstraint;							///< The vehicle constraint
	Ref<VehicleCollisionTester>	mTesters[2];								///< Collision testers for the wheel
	float						mPreviousForward = 1.0f;					///< Keeps track of last car direction so we know when to brake and when to accelerate
};
