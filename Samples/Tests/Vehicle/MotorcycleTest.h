// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Vehicle/VehicleTest.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

// This test shows how a motorcycle could be made with the vehicle constraint.
/// Note: The motor cycle controller is still in development and may need a lot of tweaks/hacks to work properly!
class MotorcycleTest : public VehicleTest
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(MotorcycleTest)

	// Destructor
	virtual						~MotorcycleTest() override;

	// See: Test
	virtual void				Initialize() override;
	virtual void				PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void				SaveState(StateRecorder& inStream) const override;
	virtual void				RestoreState(StateRecorder& inStream) override;

	virtual void				GetInitialCamera(CameraState &ioState) const override;
	virtual RMat44				GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;

private:
	Body *						mMotorcycleBody;							///< The vehicle
	Ref<VehicleConstraint>		mVehicleConstraint;							///< The vehicle constraint
	float						mPreviousForward = 1.0f;					///< Keeps track of last motorcycle direction so we know when to brake and when to accelerate
	float						mCurrentRight = 0.0f;						///< Keeps track of the current steering angle (radians)
};
