// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Constraints/SwingTwistConstraint.h>

class PoweredSwingTwistConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(PoweredSwingTwistConstraintTest)

	virtual void				Initialize() override;
	virtual void				PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	virtual void				GetInitialCamera(CameraState &ioState) const override;

	virtual bool				HasSettingsMenu() const override							{ return true; }
	virtual void				CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	static Vec3					sBodyRotation[2];
	inline static EMotorState	sSwingMotorState = EMotorState::Velocity;
	inline static EMotorState	sTwistMotorState = EMotorState::Velocity;
	inline static Vec3			sTargetVelocityCS = Vec3(DegreesToRadians(90), 0, 0);
	inline static Vec3			sTargetOrientationCS = Vec3::sZero();
	inline static float			sMaxAngularAcceleration = DegreesToRadians(36000.0f);
	inline static float			sMaxFrictionAngularAcceleration = 0.0f;
	inline static float			sNormalHalfConeAngle = DegreesToRadians(60);
	inline static float			sPlaneHalfConeAngle = DegreesToRadians(45);
	inline static float			sTwistMinAngle = DegreesToRadians(-180);
	inline static float			sTwistMaxAngle = DegreesToRadians(180);
	inline static float			sFrequency = 10.0f;
	inline static float			sDamping = 2.0f;

	SwingTwistConstraint *		mConstraint = nullptr;
	float						mInertiaBody2AsSeenFromConstraint;
};
