// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Physics/Constraints/TwoBodyConstraint.h>
#include <Physics/Constraints/ConstraintPart/PointConstraintPart.h>

namespace JPH {

/// Point constraint settings, used to create a point constraint
class PointConstraintSettings final : public TwoBodyConstraintSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(PointConstraintSettings)

	// See: ConstraintSettings::SaveBinaryState
	virtual void				SaveBinaryState(StreamOut &inStream) const override;

	/// Create an an instance of this constraint
	virtual TwoBodyConstraint *	Create(Body &inBody1, Body &inBody2) const override;

	/// Constraint is placed at mCommonPoint (world space position).
	Vec3						mCommonPoint = Vec3::sZero();

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
	virtual EConstraintType		GetType() const override									{ return EConstraintType::Point; }
	virtual void				SetupVelocityConstraint(float inDeltaTime) override;
	virtual void				WarmStartVelocityConstraint(float inWarmStartImpulseRatio) override;
	virtual bool				SolveVelocityConstraint(float inDeltaTime) override;
	virtual bool				SolvePositionConstraint(float inDeltaTime, float inBaumgarte) override;
#ifdef JPH_DEBUG_RENDERER
	virtual void				DrawConstraint(DebugRenderer *inRenderer) const override;
#endif // JPH_DEBUG_RENDERER
	virtual void				SaveState(StateRecorder &inStream) const override;
	virtual void				RestoreState(StateRecorder &inStream) override;

	// See: TwoBodyConstraint
	virtual Mat44				GetConstraintToBody1Matrix() const override					{ return Mat44::sTranslation(mLocalSpacePosition1); }
	virtual Mat44				GetConstraintToBody2Matrix() const override					{ return Mat44::sTranslation(mLocalSpacePosition2); } // Note: Incorrect rotation as we don't track the original rotation difference, should not matter though as the constraint is not limiting rotation.

private:
	// Internal helper function to calculate the values below
	void						CalculateConstraintProperties();

	// Local space constraint positions
	Vec3						mLocalSpacePosition1;
	Vec3						mLocalSpacePosition2;

	// The constraint part
	PointConstraintPart			mPointConstraintPart;
};

} // JPH