// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

void MotionProperties::MoveKinematic(Vec3Arg inDeltaPosition, QuatArg inDeltaRotation, float inDeltaTime)
{
	JPH_ASSERT(BodyAccess::sCheckRights(BodyAccess::sVelocityAccess, BodyAccess::EAccess::ReadWrite));
	JPH_ASSERT(BodyAccess::sCheckRights(BodyAccess::sPositionAccess, BodyAccess::EAccess::Read));
	JPH_ASSERT(mCachedBodyType == EBodyType::RigidBody);
	JPH_ASSERT(mCachedMotionType != EMotionType::Static);

	// Calculate required linear velocity
	mLinearVelocity = inDeltaPosition / inDeltaTime;

	// Calculate required angular velocity
	Vec3 axis;
	float angle;
	inDeltaRotation.GetAxisAngle(axis, angle);
	mAngularVelocity = axis * (angle / inDeltaTime);
}

void MotionProperties::ClampLinearVelocity()
{
	JPH_ASSERT(BodyAccess::sCheckRights(BodyAccess::sVelocityAccess, BodyAccess::EAccess::ReadWrite));

	float len_sq = mLinearVelocity.LengthSq();
	JPH_ASSERT(isfinite(len_sq));
	if (len_sq > Square(mMaxLinearVelocity))
		mLinearVelocity *= mMaxLinearVelocity / sqrt(len_sq);
}

void MotionProperties::ClampAngularVelocity()
{
	JPH_ASSERT(BodyAccess::sCheckRights(BodyAccess::sVelocityAccess, BodyAccess::EAccess::ReadWrite));

	float len_sq = mAngularVelocity.LengthSq();
	JPH_ASSERT(isfinite(len_sq));
	if (len_sq > Square(mMaxAngularVelocity))
		mAngularVelocity *= mMaxAngularVelocity / sqrt(len_sq);
}

inline Mat44 MotionProperties::GetLocalSpaceInverseInertiaUnchecked() const
{
	Mat44 rotation = Mat44::sRotation(mInertiaRotation);
	Mat44 rotation_mul_scale_transposed(mInvInertiaDiagonal.SplatX() * rotation.GetColumn4(0), mInvInertiaDiagonal.SplatY() * rotation.GetColumn4(1), mInvInertiaDiagonal.SplatZ() * rotation.GetColumn4(2), Vec4(0, 0, 0, 1));
	return rotation.Multiply3x3RightTransposed(rotation_mul_scale_transposed);
}

inline Mat44 MotionProperties::GetLocalSpaceInverseInertia() const
{
	JPH_ASSERT(mCachedMotionType == EMotionType::Dynamic);
	return GetLocalSpaceInverseInertiaUnchecked();
}

Mat44 MotionProperties::GetInverseInertiaForRotation(Mat44Arg inRotation) const
{
	JPH_ASSERT(mCachedMotionType == EMotionType::Dynamic);

	Mat44 rotation = inRotation * Mat44::sRotation(mInertiaRotation);
	Mat44 rotation_mul_scale_transposed(mInvInertiaDiagonal.SplatX() * rotation.GetColumn4(0), mInvInertiaDiagonal.SplatY() * rotation.GetColumn4(1), mInvInertiaDiagonal.SplatZ() * rotation.GetColumn4(2), Vec4(0, 0, 0, 1));
	return rotation.Multiply3x3RightTransposed(rotation_mul_scale_transposed);
}

Vec3 MotionProperties::MultiplyWorldSpaceInverseInertiaByVector(QuatArg inBodyRotation, Vec3Arg inV) const
{
	JPH_ASSERT(mCachedMotionType == EMotionType::Dynamic);

	Mat44 rotation = Mat44::sRotation(inBodyRotation * mInertiaRotation);
	return rotation.Multiply3x3(mInvInertiaDiagonal * rotation.Multiply3x3Transposed(inV));
}

void MotionProperties::ApplyForceTorqueAndDragInternal(QuatArg inBodyRotation, Vec3Arg inGravity, float inDeltaTime)
{
	JPH_ASSERT(BodyAccess::sCheckRights(BodyAccess::sVelocityAccess, BodyAccess::EAccess::ReadWrite));
	JPH_ASSERT(mCachedBodyType == EBodyType::RigidBody);
	JPH_ASSERT(mCachedMotionType == EMotionType::Dynamic);

	// Update linear velocity
	mLinearVelocity = LockTranslation(mLinearVelocity + inDeltaTime * (mGravityFactor * inGravity + mInvMass * GetAccumulatedForce()));

	// Update angular velocity
	mAngularVelocity += inDeltaTime * MultiplyWorldSpaceInverseInertiaByVector(inBodyRotation, GetAccumulatedTorque());

	// Linear damping: dv/dt = -c * v
	// Solution: v(t) = v(0) * e^(-c * t) or v2 = v1 * e^(-c * dt)
	// Taylor expansion of e^(-c * dt) = 1 - c * dt + ...
	// Since dt is usually in the order of 1/60 and c is a low number too this approximation is good enough
	mLinearVelocity *= max(0.0f, 1.0f - mLinearDamping * inDeltaTime);
	mAngularVelocity *= max(0.0f, 1.0f - mAngularDamping * inDeltaTime);

	// Clamp velocities
	ClampLinearVelocity();
	ClampAngularVelocity();
}

void MotionProperties::ResetSleepTestSpheres(const RVec3 *inPoints)
{
#ifdef JPH_DOUBLE_PRECISION
	// Make spheres relative to the first point and initialize them to zero radius
	DVec3 offset = inPoints[0];
	offset.StoreDouble3(&mSleepTestOffset);
	mSleepTestSpheres[0] = Sphere(Vec3::sZero(), 0.0f);
	for (int i = 1; i < 3; ++i)
		mSleepTestSpheres[i] = Sphere(Vec3(inPoints[i] - offset), 0.0f);
#else
	// Initialize the spheres to zero radius around the supplied points
	for (int i = 0; i < 3; ++i)
		mSleepTestSpheres[i] = Sphere(inPoints[i], 0.0f);
#endif

	mSleepTestTimer = 0.0f;
}

ECanSleep MotionProperties::AccumulateSleepTime(float inDeltaTime, float inTimeBeforeSleep)
{
	mSleepTestTimer += inDeltaTime;
	return mSleepTestTimer >= inTimeBeforeSleep? ECanSleep::CanSleep : ECanSleep::CannotSleep;
}

JPH_NAMESPACE_END
