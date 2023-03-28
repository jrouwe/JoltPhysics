// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
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
	virtual RMat44				GetCameraPivot(float inCameraHeading, float inCameraPitch) const override;

	virtual void				CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	static inline float			sInitialRollAngle = 0;
	static inline float			sMaxRollAngle = DegreesToRadians(60.0f);
	static inline float			sMaxSteeringAngle = DegreesToRadians(30.0f);
	static inline int			sCollisionMode = 2;
	static inline bool			sFourWheelDrive = false;
	static inline bool			sAntiRollbar = true;
	static inline bool			sLimitedSlipDifferentials = true;
	static inline float			sMaxEngineTorque = 500.0f;
	static inline float			sClutchStrength = 10.0f;
	static inline float			sFrontSuspensionCasterAngle = 0.0f;
	static inline float 		sFrontSuspensionKinPinAngle = 0.0f;
	static inline float			sFrontSuspensionMinLength = 0.3f;
	static inline float			sFrontSuspensionMaxLength = 0.5f;
	static inline float			sFrontSuspensionFrequency = 1.5f;
	static inline float			sFrontSuspensionDamping = 0.5f;
	static inline float 		sRearSuspensionCasterAngle = 0.0f;
	static inline float 		sRearSuspensionKingPinAngle = 0.0f;
	static inline float			sRearSuspensionMinLength = 0.3f;
	static inline float			sRearSuspensionMaxLength = 0.5f;
	static inline float			sRearSuspensionFrequency = 1.5f;
	static inline float			sRearSuspensionDamping = 0.5f;

	Body *						mCarBody;									///< The vehicle
	Ref<VehicleConstraint>		mVehicleConstraint;							///< The vehicle constraint
	Ref<VehicleCollisionTester>	mTesters[3];								///< Collision testers for the wheel
	float						mPreviousForward = 1.0f;					///< Keeps track of last car direction so we know when to brake and when to accelerate
};
