// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Constraints/TwoBodyConstraint.h>
#include <Jolt/Physics/Constraints/ConstraintPart/AxisConstraintPart.h>

JPH_NAMESPACE_BEGIN

/// Distance constraint settings, used to create a distance constraint
class DistanceConstraintSettings final : public TwoBodyConstraintSettings
{
public:
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(DistanceConstraintSettings)

	// See: ConstraintSettings::SaveBinaryState
	virtual void				SaveBinaryState(StreamOut &inStream) const override;

	/// Create an an instance of this constraint
	virtual TwoBodyConstraint *	Create(Body &inBody1, Body &inBody2) const override;

	/// This determines in which space the constraint is setup, all properties below should be in the specified space
	EConstraintSpace			mSpace = EConstraintSpace::WorldSpace;

	/// Body 1 constraint reference frame (space determined by mSpace).
	/// Constraint will keep mPoint1 (a point on body 1) and mPoint2 (a point on body 2) at the same distance.
	/// Note that this constraint can be used as a cheap PointConstraint by setting mPoint1 = mPoint2 (but this removes only 1 degree of freedom instead of 3).
	Vec3						mPoint1 = Vec3::sZero();

	/// Body 2 constraint reference frame (space determined by mSpace)
	Vec3						mPoint2 = Vec3::sZero();

	/// Ability to override the distance range at which the two points are kept apart. If the value is negative, it will be replaced by the distance between mPoint1 and mPoint2 (works only if mSpace is world space).
	float						mMinDistance = -1.0f;
	float						mMaxDistance = -1.0f;

	/// If mFrequency > 0 the constraint will be soft and mFrequency specifies the oscillation frequency in Hz and mDamping the damping ratio (0 = no damping, 1 = critical damping).
	/// If mFrequency <= 0, mDamping is ignored and the distance constraint will have hard limits (as hard as the time step / the number of velocity / position solver steps allows).
	/// Note that if you set mDamping = 0, you will not get an infinite oscillation. Because we integrate physics using an explicit Euler scheme, there is always energy loss.
	/// This is done to keep the simulation from exploding, because with a damping of 0 and even the slightest rounding error, the oscillation could become bigger and bigger until the simluation explodes.
	float						mFrequency = 0.0f;
	float						mDamping = 0.0f;

protected:
	// See: ConstraintSettings::RestoreBinaryState
	virtual void				RestoreBinaryState(StreamIn &inStream) override;
};

/// This constraint is a stiff spring that holds 2 points at a fixed distance from each other
class DistanceConstraint final : public TwoBodyConstraint
{
public:
	/// Construct distance constraint
								DistanceConstraint(Body &inBody1, Body &inBody2, const DistanceConstraintSettings &inSettings);

	// Generic interface of a constraint
	virtual EConstraintSubType	GetSubType() const override									{ return EConstraintSubType::Distance; }
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

	/// Update the minimum and maximum distance for the constraint
	void						SetDistance(float inMinDistance, float inMaxDistance)		{ JPH_ASSERT(inMinDistance <= inMaxDistance); mMinDistance = inMinDistance; mMaxDistance = inMaxDistance; }
	float						GetMinDistance() const										{ return mMinDistance; }
	float						GetMaxDistance() const										{ return mMaxDistance; }

	/// Update the spring frequency for the constraint
	void						SetFrequency(float inFrequency)								{ JPH_ASSERT(inFrequency >= 0.0f); mFrequency = inFrequency; }
	float						GetFrequency() const										{ return mFrequency; }

	/// Update the spring damping for the constraint
	void						SetDamping(float inDamping)									{ JPH_ASSERT(inDamping >= 0.0f); mDamping = inDamping; }
	float						GetDamping() const											{ return mDamping; }

	///@name Get Lagrange multiplier from last physics update (relates to how much force/torque was applied to satisfy the constraint)
	inline float	 			GetTotalLambdaPosition() const								{ return mAxisConstraint.GetTotalLambda(); }

private:
	// Internal helper function to calculate the values below
	void						CalculateConstraintProperties(float inDeltaTime);

	// CONFIGURATION PROPERTIES FOLLOW

	// Local space constraint positions
	Vec3						mLocalSpacePosition1;
	Vec3						mLocalSpacePosition2;

	// Min/max distance that must be kept between the world space points
	float						mMinDistance;
	float						mMaxDistance;

	// Soft constraint properties (see DistanceConstraintSettings)
	float						mFrequency;
	float						mDamping;

	// RUN TIME PROPERTIES FOLLOW

	// World space positions and normal
	Vec3						mWorldSpacePosition1;
	Vec3						mWorldSpacePosition2;
	Vec3						mWorldSpaceNormal;

	// Depending on if the distance < min or distance > max we can apply forces to prevent further violations
	float						mMinLambda;
	float						mMaxLambda;

	// The constraint part
	AxisConstraintPart			mAxisConstraint;
};

JPH_NAMESPACE_END
