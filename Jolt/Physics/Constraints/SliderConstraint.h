// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Physics/Constraints/TwoBodyConstraint.h>
#include <Physics/Constraints/MotorSettings.h>
#include <Physics/Constraints/ConstraintPart/DualAxisConstraintPart.h>
#include <Physics/Constraints/ConstraintPart/RotationEulerConstraintPart.h>
#include <Physics/Constraints/ConstraintPart/AxisConstraintPart.h>

namespace JPH {

/// Slider constraint settings, used to create a slider constraint
class SliderConstraintSettings final : public TwoBodyConstraintSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(SliderConstraintSettings)

	// See: ConstraintSettings::SaveBinaryState
	virtual void				SaveBinaryState(StreamOut &inStream) const override;

	/// Create an an instance of this constraint
	virtual TwoBodyConstraint *	Create(Body &inBody1, Body &inBody2) const override;

	/// Axis along which movement is possible (world space direction).
	Vec3						mSliderAxis = Vec3::sAxisX();

	/// Bodies are assumed to be placed so that the slider position = 0, movement will be limited between [mLimitsMin, mLimitsMax] where mLimitsMin e [-inf, 0] and mLimitsMax e [0, inf]
	float						mLimitsMin = -FLT_MAX;
	float						mLimitsMax = FLT_MAX;

	/// Maximum amount of friction force to apply (N) when not driven by a motor.
	float						mMaxFrictionForce = 0.0f;

	/// In case the constraint is powered, this determines the motor settings around the sliding axis
	MotorSettings				mMotorSettings;

protected:
	// See: ConstraintSettings::RestoreBinaryState
	virtual void				RestoreBinaryState(StreamIn &inStream) override;
};

/// A slider constraint allows movement in only 1 axis (and no rotation). Also known as a prismatic constraint.
class SliderConstraint final : public TwoBodyConstraint
{
public:
	/// Construct slider constraint
								SliderConstraint(Body &inBody1, Body &inBody2, const SliderConstraintSettings &inSettings);

	// Generic interface of a constraint
	virtual EConstraintType		GetType() const override								{ return EConstraintType::Slider; }
	virtual void				SetupVelocityConstraint(float inDeltaTime) override;
	virtual void				WarmStartVelocityConstraint(float inWarmStartImpulseRatio) override;
	virtual bool				SolveVelocityConstraint(float inDeltaTime) override;
	virtual bool				SolvePositionConstraint(float inDeltaTime, float inBaumgarte) override;
#ifdef JPH_STAT_COLLECTOR
	virtual void				CollectStats() const override;
#endif // JPH_STAT_COLLECTOR
#ifdef JPH_DEBUG_RENDERER
	virtual void				DrawConstraint(DebugRenderer *inRenderer) const override;
	virtual void				DrawConstraintLimits(DebugRenderer *inRenderer) const override;
#endif // JPH_DEBUG_RENDERER
	virtual void				SaveState(StateRecorder &inStream) const override;
	virtual void				RestoreState(StateRecorder &inStream) override;

	// See: TwoBodyConstraint
	virtual Mat44				GetConstraintToBody1Matrix() const override;
	virtual Mat44				GetConstraintToBody2Matrix() const override;

	/// Friction control
	void						SetMaxFrictionForce(float inFrictionForce)				{ mMaxFrictionForce = inFrictionForce; }
	float						GetMaxFrictionForce() const								{ return mMaxFrictionForce; }

	/// Motor settings
	MotorSettings &				GetMotorSettings()										{ return mMotorSettings; }
	const MotorSettings &		GetMotorSettings() const								{ return mMotorSettings; }

	// Motor controls
	void						SetMotorState(EMotorState inState)						{ JPH_ASSERT(inState == EMotorState::Off || mMotorSettings.IsValid()); mMotorState = inState; }
	EMotorState					GetMotorState() const									{ return mMotorState; }
	void						SetTargetVelocity(float inVelocity)						{ mTargetVelocity = inVelocity; }
	float						GetTargetVelocity() const								{ return mTargetVelocity; }
	void						SetTargetPosition(float inPosition)						{ mTargetPosition = mHasLimits? Clamp(inPosition, mLimitsMin, mLimitsMax) : inPosition; }
	float						GetTargetPosition() const								{ return mTargetPosition; }

	/// Update the limits of the slider constraint (see SliderConstraintSettings)
	void						SetLimits(float inLimitsMin, float inLimitsMax);
	float						GetLimitsMin() const									{ return mLimitsMin; }
	float						GetLimitsMax() const									{ return mLimitsMax; }
	bool						HasLimits() const										{ return mHasLimits; }

	///@name Get Lagrange multiplier from last physics update (relates to how much force/torque was applied to satisfy the constraint)
	inline Vector<2> 			GetTotalLambdaPosition() const							{ return mPositionConstraintPart.GetTotalLambda(); }
	inline float				GetTotalLambdaPositionLimits() const					{ return mPositionLimitsConstraintPart.GetTotalLambda(); }
	inline Vec3					GetTotalLambdaRotation() const							{ return mRotationConstraintPart.GetTotalLambda(); }
	inline float				GetTotalLambdaMotor() const								{ return mMotorConstraintPart.GetTotalLambda(); }

private:
	// Internal helper function to calculate the values below
	void						CalculateR1R2U(Mat44Arg inRotation1, Mat44Arg inRotation2);
	void						CalculateSlidingAxisAndPosition(Mat44Arg inRotation1);
	void						CalculatePositionConstraintProperties(Mat44Arg inRotation1, Mat44Arg inRotation2);
	void						CalculatePositionLimitsConstraintProperties(float inDeltaTime, Mat44Arg inRotation1);
	void						CalculateMotorConstraintProperties(float inDeltaTime);

	// CONFIGURATION PROPERTIES FOLLOW

	// Local space constraint positions
	Vec3						mLocalSpacePosition1;
	Vec3						mLocalSpacePosition2;

	// Local space sliding direction
	Vec3						mLocalSpaceSliderAxis1;

	// Local space normals to the sliding direction
	Vec3						mLocalSpaceNormal1;
	Vec3						mLocalSpaceNormal2;

	// Inverse of initial rotation from body 1 to body 2 in body 1 space
	Quat						mInvInitialOrientation;
		
	// Slider limits
	bool						mHasLimits;
	float						mLimitsMin;
	float						mLimitsMax;

	// Friction
	float						mMaxFrictionForce;

	// Motor controls
	MotorSettings				mMotorSettings;
	EMotorState					mMotorState = EMotorState::Off;
	float						mTargetVelocity = 0.0f;
	float						mTargetPosition = 0.0f;

	// RUN TIME PROPERTIES FOLLOW

	// Positions where the point constraint acts on (middle point between center of masses)
	Vec3						mR1;
	Vec3						mR2;

	// X2 + R2 - X1 - R1
	Vec3						mU;

	// World space sliding direction
	Vec3						mWorldSpaceSliderAxis;

	// Normals to the slider axis
	Vec3						mN1;
	Vec3						mN2;

	// Distance along the slide axis
	float						mD = 0.0f;

	// The constraint parts
	DualAxisConstraintPart		mPositionConstraintPart;
	RotationEulerConstraintPart	mRotationConstraintPart;
	AxisConstraintPart			mPositionLimitsConstraintPart;
	AxisConstraintPart			mMotorConstraintPart;
};

} // JPH