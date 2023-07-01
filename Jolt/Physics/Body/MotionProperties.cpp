// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Body/MotionProperties.h>
#include <Jolt/Physics/StateRecorder.h>

JPH_NAMESPACE_BEGIN

void MotionProperties::SetMassProperties(ELockedAxis inLockedAxis, const MassProperties &inMassProperties)
{
	// Store locked axis
	mLockedAxis = inLockedAxis;

	// Decompose locked axis
	uint locked_translation_axis = uint(inLockedAxis) & 0b111;
	uint locked_rotation_axis = (uint(inLockedAxis) >> 3) & 0b111;

	// Set inverse mass
	if (locked_translation_axis == 0b111)
	{
		// No translation possible
		mInvMass = 0.0f;
	}
	else
	{
		JPH_ASSERT(inMassProperties.mMass > 0.0f);
		mInvMass = 1.0f / inMassProperties.mMass;
	}

	if (locked_rotation_axis == 0)
	{
		// Set inverse inertia
		Mat44 rotation;
		Vec3 diagonal;
		if (inMassProperties.DecomposePrincipalMomentsOfInertia(rotation, diagonal) 
			&& !diagonal.IsNearZero())
		{	
			mInvInertiaDiagonal = diagonal.Reciprocal();
			mInertiaRotation = rotation.GetQuaternion();
		}
		else
		{
			// Failed! Fall back to inertia tensor of sphere with radius 1.
			mInvInertiaDiagonal = Vec3::sReplicate(2.5f * mInvMass);
			mInertiaRotation = Quat::sIdentity();
		}
	}
	else if (locked_rotation_axis == 0b111)
	{
		// No rotation possible
		mInvInertiaDiagonal = Vec3::sZero();
		mInertiaRotation = Quat::sIdentity();
	}
	else
	{
		uint num_locked_rotation_axis = CountBits(locked_rotation_axis);
		if (num_locked_rotation_axis == 2)
		{
			// We can only rotate around one axis so the inverse inertia is trivial to calculate
			mInvInertiaDiagonal = Vec3::sZero();
			mInertiaRotation = Quat::sIdentity();
			for (int axis = 0; axis < 3; ++axis)
				if ((locked_rotation_axis & (1 << axis)) == 0)
					mInvInertiaDiagonal.SetComponent(axis, 1.0f / inMassProperties.mInertia(axis, axis));
		}
		else
		{
			JPH_ASSERT(num_locked_rotation_axis == 1);
			uint locked_axis = CountTrailingZeros(locked_rotation_axis);

			Mat44 inverse_inertia = inMassProperties.mInertia.Inversed3x3();

			// Get the 2 non-locked axis
			Vec3 normalized_axis[2];
			float axis_len[2];
			uint next_axis = 0;
			for (uint axis = 0; axis < 3; ++axis)
				if (axis != locked_axis)
				{
					Vec3 &dest = normalized_axis[next_axis];
					dest = inverse_inertia.GetColumn3(axis);
					dest.SetComponent(locked_axis, 0.0f);
					float dest_len = dest.Length();
					dest /= dest_len;
					axis_len[next_axis] = dest_len;
					++next_axis;
				}
			JPH_ASSERT(next_axis == 2);

			// Make sure that axis 0 is the smallest (largest inertia)
			if (axis_len[0] > axis_len[1])
			{
				swap(normalized_axis[0], normalized_axis[1]);
				swap(axis_len[0], axis_len[1]);
			}

			// Build a new inverse inertia matrix and decompose it
			Mat44 reordered_inverse_inertia(Vec4(normalized_axis[0]), Vec4(normalized_axis[1]), Vec4(normalized_axis[0].Cross(normalized_axis[1])), Vec4(0, 0, 0, 1));
			mInertiaRotation = reordered_inverse_inertia.Decompose(mInvInertiaDiagonal).GetQuaternion();
		}
	}

	JPH_ASSERT(mInvMass != 0.0f || mInvInertiaDiagonal != Vec3::sZero(), "Can't lock all axes, use a Kinematic body for this!");
}

void MotionProperties::SaveState(StateRecorder &inStream) const
{
	// Only write properties that can change at runtime
	inStream.Write(mLinearVelocity);
	inStream.Write(mAngularVelocity);
	inStream.Write(mForce);
	inStream.Write(mTorque);
	inStream.Write(mLinearDamping);
	inStream.Write(mAngularDamping);
	inStream.Write(mMaxLinearVelocity);
	inStream.Write(mMaxAngularVelocity);
	inStream.Write(mGravityFactor);
#ifdef JPH_DOUBLE_PRECISION
	inStream.Write(mSleepTestOffset);
#endif // JPH_DOUBLE_PRECISION
	inStream.Write(mSleepTestSpheres);
	inStream.Write(mSleepTestTimer);
	inStream.Write(mMotionQuality);
	inStream.Write(mAllowSleeping);
}

void MotionProperties::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mLinearVelocity);
	inStream.Read(mAngularVelocity);
	inStream.Read(mForce);
	inStream.Read(mTorque);
	inStream.Read(mLinearDamping);
	inStream.Read(mAngularDamping);
	inStream.Read(mMaxLinearVelocity);
	inStream.Read(mMaxAngularVelocity);
	inStream.Read(mGravityFactor);
#ifdef JPH_DOUBLE_PRECISION
	inStream.Read(mSleepTestOffset);
#endif // JPH_DOUBLE_PRECISION
	inStream.Read(mSleepTestSpheres);
	inStream.Read(mSleepTestTimer);
	inStream.Read(mMotionQuality);
	inStream.Read(mAllowSleeping);
}

JPH_NAMESPACE_END
