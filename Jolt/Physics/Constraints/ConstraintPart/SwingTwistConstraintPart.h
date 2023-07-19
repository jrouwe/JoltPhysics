// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Geometry/Ellipse.h>
#include <Jolt/Physics/Constraints/ConstraintPart/RotationEulerConstraintPart.h>
#include <Jolt/Physics/Constraints/ConstraintPart/AngleConstraintPart.h>

JPH_NAMESPACE_BEGIN

/// Quaternion based constraint that decomposes the rotation in constraint space in swing and twist: q = q_swing * q_twist
/// where q_swing.x = 0 and where q_twist.y = q_twist.z = 0
///
/// - Rotation around the twist (x-axis) is within [inTwistMinAngle, inTwistMaxAngle].
/// - Rotation around the swing axis (y and z axis) are limited to an ellipsoid in quaternion space formed by the equation:
///
/// (q_swing.y / sin(inSwingYHalfAngle / 2))^2 + (q_swing.z / sin(inSwingZHalfAngle / 2))^2 <= 1
///
/// Which roughly corresponds to an elliptic cone shape with major axis (inSwingYHalfAngle, inSwingZHalfAngle).
///
/// In case inSwingYHalfAngle = 0, the rotation around Y will be constrained to 0 and the rotation around Z
/// will be constrained between [-inSwingZHalfAngle, inSwingZHalfAngle]. Vice versa if inSwingZHalfAngle = 0.
class SwingTwistConstraintPart
{
public:
	/// Set limits for this constraint (see description above for parameters)
	void						SetLimits(float inTwistMinAngle, float inTwistMaxAngle, float inSwingYHalfAngle, float inSwingZHalfAngle)
	{
		constexpr float cLockedAngle = DegreesToRadians(0.5f);
		constexpr float cFreeAngle = DegreesToRadians(179.5f);

		// Assume sane input
		JPH_ASSERT(inTwistMinAngle <= 0.0f && inTwistMinAngle >= -JPH_PI);
		JPH_ASSERT(inTwistMaxAngle >= 0.0f && inTwistMaxAngle <= JPH_PI);
		JPH_ASSERT(inSwingYHalfAngle >= 0.0f && inSwingYHalfAngle <= JPH_PI);
		JPH_ASSERT(inSwingZHalfAngle >= 0.0f && inSwingZHalfAngle <= JPH_PI);

		// Calculate the sine and cosine of the half angles
		Vec4 s, c;
		(0.5f * Vec4(inTwistMinAngle, inTwistMaxAngle, inSwingYHalfAngle, inSwingZHalfAngle)).SinCos(s, c);

		// Store axis flags which are used at runtime to quickly decided which contraints to apply
		mRotationFlags = 0;
		if (inTwistMinAngle > -cLockedAngle && inTwistMaxAngle < cLockedAngle)
		{
			mRotationFlags |= TwistXLocked;
			mSinTwistHalfMinAngle = 0.0f;
			mSinTwistHalfMaxAngle = 0.0f;
			mCosTwistHalfMinAngle = 1.0f;
			mCosTwistHalfMaxAngle = 1.0f;
		}
		else if (inTwistMinAngle < -cFreeAngle && inTwistMaxAngle > cFreeAngle)
		{
			mRotationFlags |= TwistXFree;
			mSinTwistHalfMinAngle = -1.0f;
			mSinTwistHalfMaxAngle = 1.0f;
			mCosTwistHalfMinAngle = 0.0f;
			mCosTwistHalfMaxAngle = 0.0f;
		}
		else
		{
			mSinTwistHalfMinAngle = s.GetX();
			mSinTwistHalfMaxAngle = s.GetY();
			mCosTwistHalfMinAngle = c.GetX();
			mCosTwistHalfMaxAngle = c.GetY();
		}

		if (inSwingYHalfAngle < cLockedAngle)
		{
			mRotationFlags |= SwingYLocked;
			mSinSwingYQuarterAngle = 0.0f;
		}
		else if (inSwingYHalfAngle > cFreeAngle)
		{
			mRotationFlags |= SwingYFree;
			mSinSwingYQuarterAngle = 1.0f;
		}
		else
		{
			mSinSwingYQuarterAngle = s.GetZ();
		}

		if (inSwingZHalfAngle < cLockedAngle)
		{
			mRotationFlags |= SwingZLocked;
			mSinSwingZQuarterAngle = 0.0f;
		}
		else if (inSwingZHalfAngle > cFreeAngle)
		{
			mRotationFlags |= SwingZFree;
			mSinSwingZQuarterAngle = 1.0f;
		}
		else
		{
			mSinSwingZQuarterAngle = s.GetW();
		}
	}

	/// Clamp twist and swing against the constraint limits, returns which parts were clamped (everything assumed in constraint space)
	inline void					ClampSwingTwist(Quat &ioSwing, bool &outSwingYClamped, bool &outSwingZClamped, Quat &ioTwist, bool &outTwistClamped) const
	{
		// Start with not clamped
		outTwistClamped = false;
		outSwingYClamped = false;
		outSwingZClamped = false;

		// Check that swing and twist quaternions don't contain rotations around the wrong axis
		JPH_ASSERT(ioSwing.GetX() == 0.0f);
		JPH_ASSERT(ioTwist.GetY() == 0.0f);
		JPH_ASSERT(ioTwist.GetZ() == 0.0f);

		// Ensure quaternions have w > 0
		bool negate_swing = ioSwing.GetW() < 0.0f;
		if (negate_swing)
			ioSwing = -ioSwing;
		bool negate_twist = ioTwist.GetW() < 0.0f;
		if (negate_twist)
			ioTwist = -ioTwist;

		if (mRotationFlags & TwistXLocked)
		{
			// Twist axis is locked, clamp whenever twist is not identity
			if (ioTwist.GetX() != 0.0f)
			{
				ioTwist = Quat::sIdentity();
				outTwistClamped = true;
			}
		}
		else if ((mRotationFlags & TwistXFree) == 0)
		{
			// Twist axis has limit, clamp whenever out of range
			float delta_min = mSinTwistHalfMinAngle - ioTwist.GetX();
			float delta_max = ioTwist.GetX() - mSinTwistHalfMaxAngle;
			if (delta_min > 0.0f || delta_max > 0.0f)
			{
				// We're outside of the limits, get actual delta to min/max range
				// Note that a twist of -1 and 1 represent the same angle, so if the difference is bigger than 1, the shortest angle is the other way around (2 - difference)
				// We should actually be working with angles rather than sin(angle / 2). When the difference is small the approximation is accurate, but
				// when working with extreme values the calculation is off and e.g. when the limit is between 0 and 180 a value of approx -60 will clamp
				// to 180 rather than 0 (you'd expect anything > -90 to go to 0).
				delta_min = abs(delta_min);
				if (delta_min > 1.0f) delta_min = 2.0f - delta_min;
				delta_max = abs(delta_max);
				if (delta_max > 1.0f) delta_max = 2.0f - delta_max;

				// Pick the twist that corresponds to the smallest delta
				if (delta_min < delta_max)
					ioTwist = Quat(mSinTwistHalfMinAngle, 0, 0, mCosTwistHalfMinAngle);
				else
					ioTwist = Quat(mSinTwistHalfMaxAngle, 0, 0, mCosTwistHalfMaxAngle);
				outTwistClamped = true;
			}
		}

		// Clamp swing
		if (mRotationFlags & SwingYLocked)
		{
			if (mRotationFlags & SwingZLocked)
			{
				// Both swing Y and Z are disabled, no degrees of freedom in swing
				outSwingYClamped = ioSwing.GetY() != 0.0f;
				outSwingZClamped = ioSwing.GetZ() != 0.0f;
				if (outSwingYClamped || outSwingZClamped)
					ioSwing = Quat::sIdentity();
			}
			else
			{
				// Swing Y angle disabled, only 1 degree of freedom in swing
				float z = Clamp(ioSwing.GetZ(), -mSinSwingZQuarterAngle, mSinSwingZQuarterAngle);
				outSwingYClamped = ioSwing.GetY() != 0.0f;
				outSwingZClamped = z != ioSwing.GetZ();
				if (outSwingYClamped || outSwingZClamped)
					ioSwing = Quat(0, 0, z, sqrt(1.0f - Square(z)));
			}
		}
		else if (mRotationFlags & SwingZLocked)
		{
			// Swing Z angle disabled, only 1 degree of freedom in swing
			float y = Clamp(ioSwing.GetY(), -mSinSwingYQuarterAngle, mSinSwingYQuarterAngle);
			outSwingYClamped = y != ioSwing.GetY();
			outSwingZClamped = ioSwing.GetZ() != 0.0f;
			if (outSwingYClamped || outSwingZClamped)
				ioSwing = Quat(0, y, 0, sqrt(1.0f - Square(y)));
		}
		else
		{
			// Two degrees of freedom, use ellipse to solve limits
			Ellipse ellipse(mSinSwingYQuarterAngle, mSinSwingZQuarterAngle);
			Float2 point(ioSwing.GetY(), ioSwing.GetZ());
			if (!ellipse.IsInside(point))
			{
				Float2 closest = ellipse.GetClosestPoint(point);
				ioSwing = Quat(0, closest.x, closest.y, sqrt(max(0.0f, 1.0f - Square(closest.x) - Square(closest.y))));
				outSwingYClamped = true;
				outSwingZClamped = true;
			}
		}

		// Flip sign back
		if (negate_swing)
			ioSwing = -ioSwing;
		if (negate_twist)
			ioTwist = -ioTwist;

		JPH_ASSERT(ioSwing.IsNormalized());
		JPH_ASSERT(ioTwist.IsNormalized());
	}

	/// Calculate properties used during the functions below
	/// @param inBody1 The first body that this constraint is attached to
	/// @param inBody2 The second body that this constraint is attached to
	/// @param inConstraintRotation The current rotation of the constraint in constraint space
	/// @param inConstraintToWorld Rotates from constraint space into world space
	inline void					CalculateConstraintProperties(const Body &inBody1, const Body &inBody2, QuatArg inConstraintRotation, QuatArg inConstraintToWorld)
	{
		// Decompose into swing and twist
		Quat q_swing, q_twist;
		inConstraintRotation.GetSwingTwist(q_swing, q_twist);

		// Clamp against joint limits
		Quat q_clamped_swing = q_swing, q_clamped_twist = q_twist;
		bool swing_y_clamped, swing_z_clamped, twist_clamped;
		ClampSwingTwist(q_clamped_swing, swing_y_clamped, swing_z_clamped, q_clamped_twist, twist_clamped);

		if (mRotationFlags & SwingYLocked)
		{
			Quat twist_to_world = inConstraintToWorld * q_swing;
			mWorldSpaceSwingLimitYRotationAxis = twist_to_world.RotateAxisY();
			mWorldSpaceSwingLimitZRotationAxis = twist_to_world.RotateAxisZ();

			if (mRotationFlags & SwingZLocked)
			{
				// Swing fully locked
				mSwingLimitYConstraintPart.CalculateConstraintProperties(inBody1, inBody2, mWorldSpaceSwingLimitYRotationAxis);
				mSwingLimitZConstraintPart.CalculateConstraintProperties(inBody1, inBody2, mWorldSpaceSwingLimitZRotationAxis);
			}
			else
			{
				// Swing only locked around Y
				mSwingLimitYConstraintPart.CalculateConstraintProperties(inBody1, inBody2, mWorldSpaceSwingLimitYRotationAxis);
				if (swing_z_clamped)
				{
					if (Sign(q_swing.GetW()) * q_swing.GetZ() < 0.0f)
						mWorldSpaceSwingLimitZRotationAxis = -mWorldSpaceSwingLimitZRotationAxis; // Flip axis if angle is negative because the impulse limit is going to be between [-FLT_MAX, 0]
					mSwingLimitZConstraintPart.CalculateConstraintProperties(inBody1, inBody2, mWorldSpaceSwingLimitZRotationAxis);
				}
				else
					mSwingLimitZConstraintPart.Deactivate();
			}
		}
		else if (mRotationFlags & SwingZLocked)
		{
			// Swing only locked around Z
			Quat twist_to_world = inConstraintToWorld * q_swing;
			mWorldSpaceSwingLimitYRotationAxis = twist_to_world.RotateAxisY();
			mWorldSpaceSwingLimitZRotationAxis = twist_to_world.RotateAxisZ();

			if (swing_y_clamped)
			{
				if (Sign(q_swing.GetW()) * q_swing.GetY() < 0.0f)
					mWorldSpaceSwingLimitYRotationAxis = -mWorldSpaceSwingLimitYRotationAxis; // Flip axis if angle is negative because the impulse limit is going to be between [-FLT_MAX, 0]
				mSwingLimitYConstraintPart.CalculateConstraintProperties(inBody1, inBody2, mWorldSpaceSwingLimitYRotationAxis);
			}
			else
				mSwingLimitYConstraintPart.Deactivate();
			mSwingLimitZConstraintPart.CalculateConstraintProperties(inBody1, inBody2, mWorldSpaceSwingLimitZRotationAxis);
		}
		else if ((mRotationFlags & SwingYZFree) != SwingYZFree)
		{
			// Swing has limits around Y and Z
			if (swing_y_clamped || swing_z_clamped)
			{
				// Calculate axis of rotation from clamped swing to swing
				Vec3 current = (inConstraintToWorld * q_swing).RotateAxisX();
				Vec3 desired = (inConstraintToWorld * q_clamped_swing).RotateAxisX();
				mWorldSpaceSwingLimitYRotationAxis = desired.Cross(current);
				float len = mWorldSpaceSwingLimitYRotationAxis.Length();
				if (len != 0.0f)
				{
					mWorldSpaceSwingLimitYRotationAxis /= len;
					mSwingLimitYConstraintPart.CalculateConstraintProperties(inBody1, inBody2, mWorldSpaceSwingLimitYRotationAxis);
				}
				else
					mSwingLimitYConstraintPart.Deactivate();
			}
			else
				mSwingLimitYConstraintPart.Deactivate();
			mSwingLimitZConstraintPart.Deactivate();
		}
		else
		{
			// No swing limits
			mSwingLimitYConstraintPart.Deactivate();
			mSwingLimitZConstraintPart.Deactivate();
		}

		if (mRotationFlags & TwistXLocked)
		{
			// Twist locked, always activate constraint
			mWorldSpaceTwistLimitRotationAxis = (inConstraintToWorld * q_swing).RotateAxisX();
			mTwistLimitConstraintPart.CalculateConstraintProperties(inBody1, inBody2, mWorldSpaceTwistLimitRotationAxis);
		}
		else if ((mRotationFlags & TwistXFree) == 0)
		{
			// Twist has limits
			if (twist_clamped)
			{
				mWorldSpaceTwistLimitRotationAxis = (inConstraintToWorld * q_swing).RotateAxisX();
				if (Sign(q_twist.GetW()) * q_twist.GetX() < 0.0f)
					mWorldSpaceTwistLimitRotationAxis = -mWorldSpaceTwistLimitRotationAxis; // Flip axis if angle is negative because the impulse limit is going to be between [-FLT_MAX, 0]
				mTwistLimitConstraintPart.CalculateConstraintProperties(inBody1, inBody2, mWorldSpaceTwistLimitRotationAxis);
			}
			else
				mTwistLimitConstraintPart.Deactivate();
		}
		else
		{
			// No twist limits
			mTwistLimitConstraintPart.Deactivate();
		}
	}

	/// Deactivate this constraint
	void						Deactivate()
	{
		mSwingLimitYConstraintPart.Deactivate();
		mSwingLimitZConstraintPart.Deactivate();
		mTwistLimitConstraintPart.Deactivate();
	}

	/// Check if constraint is active
	inline bool					IsActive() const
	{
		return mSwingLimitYConstraintPart.IsActive() || mSwingLimitZConstraintPart.IsActive() || mTwistLimitConstraintPart.IsActive();
	}

	/// Must be called from the WarmStartVelocityConstraint call to apply the previous frame's impulses
	inline void					WarmStart(Body &ioBody1, Body &ioBody2, float inWarmStartImpulseRatio)
	{
		mSwingLimitYConstraintPart.WarmStart(ioBody1, ioBody2, inWarmStartImpulseRatio);
		mSwingLimitZConstraintPart.WarmStart(ioBody1, ioBody2, inWarmStartImpulseRatio);
		mTwistLimitConstraintPart.WarmStart(ioBody1, ioBody2, inWarmStartImpulseRatio);
	}

	/// Iteratively update the velocity constraint. Makes sure d/dt C(...) = 0, where C is the constraint equation.
	inline bool					SolveVelocityConstraint(Body &ioBody1, Body &ioBody2)
	{
		bool impulse = false;

		// Solve swing constraint
		if (mSwingLimitYConstraintPart.IsActive())
			impulse |= mSwingLimitYConstraintPart.SolveVelocityConstraint(ioBody1, ioBody2, mWorldSpaceSwingLimitYRotationAxis, -FLT_MAX, (mRotationFlags & SwingYLocked)? FLT_MAX : 0.0f);

		if (mSwingLimitZConstraintPart.IsActive())
			impulse |= mSwingLimitZConstraintPart.SolveVelocityConstraint(ioBody1, ioBody2, mWorldSpaceSwingLimitZRotationAxis, -FLT_MAX, (mRotationFlags & SwingZLocked)? FLT_MAX : 0.0f);

		// Solve twist constraint
		if (mTwistLimitConstraintPart.IsActive())
			impulse |= mTwistLimitConstraintPart.SolveVelocityConstraint(ioBody1, ioBody2, mWorldSpaceTwistLimitRotationAxis, -FLT_MAX, (mRotationFlags & TwistXLocked)? FLT_MAX : 0.0f);

		return impulse;
	}

	/// Iteratively update the position constraint. Makes sure C(...) = 0.
	/// @param ioBody1 The first body that this constraint is attached to
	/// @param ioBody2 The second body that this constraint is attached to
	/// @param inConstraintRotation The current rotation of the constraint in constraint space
	/// @param inConstraintToBody1 , inConstraintToBody2 Rotates from constraint space to body 1/2 space
	/// @param inBaumgarte Baumgarte constant (fraction of the error to correct)
	inline bool					SolvePositionConstraint(Body &ioBody1, Body &ioBody2, QuatArg inConstraintRotation, QuatArg inConstraintToBody1, QuatArg inConstraintToBody2, float inBaumgarte) const
	{
		Quat q_swing, q_twist;
		inConstraintRotation.GetSwingTwist(q_swing, q_twist);

		bool swing_y_clamped, swing_z_clamped, twist_clamped;
		ClampSwingTwist(q_swing, swing_y_clamped, swing_z_clamped, q_twist, twist_clamped);

		// Solve rotation violations
		if (swing_y_clamped || swing_z_clamped || twist_clamped)
		{
			RotationEulerConstraintPart part;
			Quat inv_initial_orientation = inConstraintToBody2 * (inConstraintToBody1 * q_swing * q_twist).Conjugated();
			part.CalculateConstraintProperties(ioBody1, Mat44::sRotation(ioBody1.GetRotation()), ioBody2, Mat44::sRotation(ioBody2.GetRotation()));
			return part.SolvePositionConstraint(ioBody1, ioBody2, inv_initial_orientation, inBaumgarte);
		}

		return false;
	}

	/// Return lagrange multiplier for swing
	inline float				GetTotalSwingYLambda() const
	{
		return mSwingLimitYConstraintPart.GetTotalLambda();
	}

	inline float				GetTotalSwingZLambda() const
	{
		return mSwingLimitZConstraintPart.GetTotalLambda();
	}

	/// Return lagrange multiplier for twist
	inline float				GetTotalTwistLambda() const
	{
		return mTwistLimitConstraintPart.GetTotalLambda();
	}

	/// Save state of this constraint part
	void						SaveState(StateRecorder &inStream) const
	{
		mSwingLimitYConstraintPart.SaveState(inStream);
		mSwingLimitZConstraintPart.SaveState(inStream);
		mTwistLimitConstraintPart.SaveState(inStream);
	}

	/// Restore state of this constraint part
	void						RestoreState(StateRecorder &inStream)
	{
		mSwingLimitYConstraintPart.RestoreState(inStream);
		mSwingLimitZConstraintPart.RestoreState(inStream);
		mTwistLimitConstraintPart.RestoreState(inStream);
	}

private:
	// CONFIGURATION PROPERTIES FOLLOW

	enum ERotationFlags
	{
		/// Indicates that axis is completely locked (cannot rotate around this axis)
		TwistXLocked			= 1 << 0,
		SwingYLocked			= 1 << 1,
		SwingZLocked			= 1 << 2,

		/// Indicates that axis is completely free (can rotate around without limits)
		TwistXFree				= 1 << 3,
		SwingYFree				= 1 << 4,
		SwingZFree				= 1 << 5,
		SwingYZFree				= SwingYFree | SwingZFree
	};

	uint8						mRotationFlags;

	// Constants
	float						mSinTwistHalfMinAngle;
	float						mSinTwistHalfMaxAngle;
	float						mCosTwistHalfMinAngle;
	float						mCosTwistHalfMaxAngle;
	float						mSinSwingYQuarterAngle;
	float						mSinSwingZQuarterAngle;

	// RUN TIME PROPERTIES FOLLOW

	/// Rotation axis for the angle constraint parts
	Vec3						mWorldSpaceSwingLimitYRotationAxis;
	Vec3						mWorldSpaceSwingLimitZRotationAxis;
	Vec3						mWorldSpaceTwistLimitRotationAxis;

	/// The constraint parts
	AngleConstraintPart			mSwingLimitYConstraintPart;
	AngleConstraintPart			mSwingLimitZConstraintPart;
	AngleConstraintPart			mTwistLimitConstraintPart;
};

JPH_NAMESPACE_END
