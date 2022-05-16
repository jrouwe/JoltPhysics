// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Constraints/TwoBodyConstraint.h>
#include <Jolt/Physics/Constraints/ConstraintPart/PointConstraintPart.h>

JPH_NAMESPACE_BEGIN

/// Point constraint settings, used to create a point constraint
class PointConstraintSettings final : public TwoBodyConstraintSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(PointConstraintSettings)

	// See: ConstraintSettings::SaveBinaryState
	virtual void				SaveBinaryState(StreamOut &inStream) const override;

	/// Create an an instance of this constraint
	virtual TwoBodyConstraint *	Create(Body &inBody1, Body &inBody2) const override;

	/// This determines in which space the constraint is setup, all properties below should be in the specified space
	EConstraintSpace			mSpace = EConstraintSpace::WorldSpace;

	/// Body 1 constraint position (space determined by mSpace).
	Vec3						mPoint1 = Vec3::sZero();

	/// Body 2 constraint position (space determined by mSpace).
	/// Note: Normally you would set mPoint1 = mPoint2 if the bodies are already placed how you want to constrain them (if mSpace = world space).
	Vec3						mPoint2 = Vec3::sZero();

protected:
	// See: ConstraintSettings::RestoreBinaryState
	virtual void				RestoreBinaryState(StreamIn &inStream) override;
};

/// A point constraint constrains 2 bodies on a single point (removing 3 degrees of freedom)
class PointConstraint final : public TwoBodyConstraint
{
public:
	/// Construct point constraint
								PointConstraint(Body &inBody1, Body &inBody2, const PointConstraintSettings &inSettings);

	// Generic interface of a constraint
	virtual EConstraintSubType	GetSubType() const override									{ return EConstraintSubType::Point; }
	virtual void				SetupVelocityConstraint(float inDeltaTime) override;
	virtual void				WarmStartVelocityConstraint(float inWarmStartImpulseRatio) override;
	virtual bool				SolveVelocityConstraint(float inDeltaTime) override;
	virtual bool				SolvePositionConstraint(float inDeltaTime, float inBaumgarte) override;
#ifdef JPH_DEBUG_RENDERER
	virtual void				DrawConstraint(DebugRenderer *inRenderer) const override;
#endif // JPH_DEBUG_RENDERER
	virtual void				SaveState(StateRecorder &inStream) const override;
	virtual void				RestoreState(StateRecorder &inStream) override;
	virtual Ref<ConstraintSettings> GetConstraintSettings() const override;

	// See: TwoBodyConstraint
	virtual Mat44				GetConstraintToBody1Matrix() const override					{ return Mat44::sTranslation(mLocalSpacePosition1); }
	virtual Mat44				GetConstraintToBody2Matrix() const override					{ return Mat44::sTranslation(mLocalSpacePosition2); } // Note: Incorrect rotation as we don't track the original rotation difference, should not matter though as the constraint is not limiting rotation.

	///@name Get Lagrange multiplier from last physics update (relates to how much force/torque was applied to satisfy the constraint)
	inline Vec3		 			GetTotalLambdaPosition() const								{ return mPointConstraintPart.GetTotalLambda(); }

private:
	// Internal helper function to calculate the values below
	void						CalculateConstraintProperties();

	// Local space constraint positions
	Vec3						mLocalSpacePosition1;
	Vec3						mLocalSpacePosition2;

	// The constraint part
	PointConstraintPart			mPointConstraintPart;
};

JPH_NAMESPACE_END
