// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Body/MotionProperties.h>
#include <Physics/StateRecorder.h>

namespace JPH {

void MotionProperties::ResetSleepTestSpheres(const Vec3 *inPoints)
{
	for (int i = 0; i < 3; ++i)
		mSleepTestSpheres[i] = Sphere(inPoints[i], 0.0f);
	mSleepTestTimer = 0.0f;
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
	inStream.Read(mSleepTestSpheres);
	inStream.Read(mSleepTestTimer);
	inStream.Read(mMotionQuality);
	inStream.Read(mAllowSleeping);
}

} // JPH