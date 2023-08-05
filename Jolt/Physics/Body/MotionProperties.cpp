// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Body/MotionProperties.h>
#include <Jolt/Physics/StateRecorder.h>

JPH_NAMESPACE_BEGIN

void MotionProperties::SetMassProperties(EAllowedDOFs inAllowedDOFs, const MassProperties &inMassProperties)
{
	// Store allowed DOFs
	mAllowedDOFs = inAllowedDOFs;

	// Decompose DOFs
	uint allowed_translation_axis = uint(inAllowedDOFs) & 0b111;
	uint allowed_rotation_axis = (uint(inAllowedDOFs) >> 3) & 0b111;

	// Set inverse mass
	if (allowed_translation_axis == 0)
	{
		// No translation possible
		mInvMass = 0.0f;
	}
	else
	{
		JPH_ASSERT(inMassProperties.mMass > 0.0f);
		mInvMass = 1.0f / inMassProperties.mMass;
	}

	if (allowed_rotation_axis == 0b111)
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
	else if (allowed_rotation_axis == 0)
	{
		// No rotation possible
		mInvInertiaDiagonal = Vec3::sZero();
		mInertiaRotation = Quat::sIdentity();
	}
	else
	{
		uint num_allowed_rotation_axis = CountBits(allowed_rotation_axis);
		if (num_allowed_rotation_axis == 1)
		{
			// We can only rotate around one axis so the inverse inertia is trivial to calculate
			mInertiaRotation = Quat::sIdentity();
			mInvInertiaDiagonal = Vec3::sZero();
			for (int axis = 0; axis < 3; ++axis)
				if ((allowed_rotation_axis & (1 << axis)) != 0)
					mInvInertiaDiagonal.SetComponent(axis, 1.0f / inMassProperties.mInertia(axis, axis));
		}
		else
		{
			JPH_ASSERT(num_allowed_rotation_axis == 2);
			uint locked_axis = CountTrailingZeros(~allowed_rotation_axis);

			// Copy the mass properties so we can modify it
			MassProperties copy = inMassProperties;
			Mat44 &inertia = copy.mInertia;

			// Set the locked row and column to 0
			for (uint axis = 0; axis < 3; ++axis)
			{
				inertia(axis, locked_axis) = 0.0f;
				inertia(locked_axis, axis) = 0.0f;
			}

			// Set the diagonal entry to 1
			inertia(locked_axis, locked_axis) = 1.0f;

			// Decompose the inertia matrix, note that using a 2x2 matrix would have been more efficient
			// but we happen to have a 3x3 matrix version lying around so we use that.
			Mat44 rotation;
			Vec3 diagonal;
			if (copy.DecomposePrincipalMomentsOfInertia(rotation, diagonal))
			{
				mInvInertiaDiagonal = diagonal.Reciprocal();
				mInertiaRotation = rotation.GetQuaternion();

				// Now set the diagonal entry corresponding to the locked axis to 0
				for (uint axis = 0; axis < 3; ++axis)
					if (abs(inertia.GetColumn3(locked_axis).Dot(rotation.GetColumn3(axis))) > 0.999f)
					{
						mInvInertiaDiagonal.SetComponent(axis, 0.0f);
						break;
					}

				// Check that we placed a zero
				JPH_ASSERT(Vec3::sEquals(mInvInertiaDiagonal, Vec3::sZero()).TestAnyXYZTrue());
			}
			else
			{
				// Failed! Fall back to inaccurate version.
				mInertiaRotation = Quat::sIdentity();
				mInvInertiaDiagonal = Vec3::sZero();
				for (uint axis = 0; axis < 3; ++axis)
					if (axis != locked_axis)
						mInvInertiaDiagonal.SetComponent(axis, 1.0f / inertia.GetColumn3(axis).Length());
			}
		}
	}

	JPH_ASSERT(mInvMass != 0.0f || mInvInertiaDiagonal != Vec3::sZero(), "Can't lock all axes, use a static body for this. This will crash with a division by zero later!");
}

void MotionProperties::SaveState(StateRecorder &inStream) const
{
	// Only write properties that can change at runtime
	inStream.Write(mLinearVelocity);
	inStream.Write(mAngularVelocity);
	inStream.Write(mForce);
	inStream.Write(mTorque);
#ifdef JPH_DOUBLE_PRECISION
	inStream.Write(mSleepTestOffset);
#endif // JPH_DOUBLE_PRECISION
	inStream.Write(mSleepTestSpheres);
	inStream.Write(mSleepTestTimer);
	inStream.Write(mAllowSleeping);
}

void MotionProperties::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mLinearVelocity);
	inStream.Read(mAngularVelocity);
	inStream.Read(mForce);
	inStream.Read(mTorque);
#ifdef JPH_DOUBLE_PRECISION
	inStream.Read(mSleepTestOffset);
#endif // JPH_DOUBLE_PRECISION
	inStream.Read(mSleepTestSpheres);
	inStream.Read(mSleepTestTimer);
	inStream.Read(mAllowSleeping);
}

JPH_NAMESPACE_END
