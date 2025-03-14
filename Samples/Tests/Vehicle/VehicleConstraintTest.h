// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Vehicle/VehicleTest.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>

class VehicleConstraintTest : public VehicleTest
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, VehicleConstraintTest)

	// Description of the test
	virtual const char *		GetDescription() const override
	{
		return	"Shows how a car could be made with a vehicle constraint.\n"
				"Use the arrow keys to drive. Z for hand brake.";
	}

	// Destructor
	virtual						~VehicleConstraintTest() override;

	// See: Test
	virtual void				Initialize() override;
	virtual void				ProcessInput(const ProcessInputParams &inParams) override;
	virtual void				PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void				SaveInputState(StateRecorder &inStream) const override;
	virtual void				RestoreInputState(StateRecorder &inStream) override;

	virtual void				GetInitialCamera(CameraState &ioState) const override;
	virtual RMat44				GetCameraPivot(float inCameraHeading, float inCameraPitch) const override { return mCameraPivot; }

	virtual void				CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	void						UpdateCameraPivot();

	static inline float			sInitialRollAngle = 0;
	static inline float			sMaxRollAngle = DegreesToRadians(60.0f);
	static inline float			sMaxSteeringAngle = DegreesToRadians(30.0f);
	static inline int			sCollisionMode = 2;
	static inline bool			sFourWheelDrive = false;
	static inline bool			sAntiRollbar = true;
	static inline bool			sLimitedSlipDifferentials = true;
	static inline bool			sOverrideGravity = false;					///< If true, gravity is overridden to always oppose the ground normal
	static inline float			sMaxEngineTorque = 500.0f;
	static inline float			sClutchStrength = 10.0f;
	static inline float			sFrontCasterAngle = 0.0f;
	static inline float			sFrontKingPinAngle = 0.0f;
	static inline float			sFrontCamber = 0.0f;
	static inline float			sFrontToe = 0.0f;
	static inline float			sFrontSuspensionForwardAngle = 0.0f;
	static inline float			sFrontSuspensionSidewaysAngle = 0.0f;
	static inline float			sFrontSuspensionMinLength = 0.3f;
	static inline float			sFrontSuspensionMaxLength = 0.5f;
	static inline float			sFrontSuspensionFrequency = 1.5f;
	static inline float			sFrontSuspensionDamping = 0.5f;
	static inline float			sRearSuspensionForwardAngle = 0.0f;
	static inline float			sRearSuspensionSidewaysAngle = 0.0f;
	static inline float			sRearCasterAngle = 0.0f;
	static inline float			sRearKingPinAngle = 0.0f;
	static inline float			sRearCamber = 0.0f;
	static inline float			sRearToe = 0.0f;
	static inline float			sRearSuspensionMinLength = 0.3f;
	static inline float			sRearSuspensionMaxLength = 0.5f;
	static inline float			sRearSuspensionFrequency = 1.5f;
	static inline float			sRearSuspensionDamping = 0.5f;

	Body *						mCarBody;									///< The vehicle
	Ref<VehicleConstraint>		mVehicleConstraint;							///< The vehicle constraint
	Ref<VehicleCollisionTester>	mTesters[3];								///< Collision testers for the wheel
	RMat44						mCameraPivot = RMat44::sIdentity();			///< The camera pivot, recorded before the physics update to align with the drawn world

	// Player input
	float						mForward = 0.0f;
	float						mPreviousForward = 1.0f;					///< Keeps track of last car direction so we know when to brake and when to accelerate
	float						mRight = 0.0f;
	float						mBrake = 0.0f;
	float						mHandBrake = 0.0f;
};
