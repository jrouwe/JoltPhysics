// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/SoftBody/SoftBodyCreationSettings.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(SoftBodyCreationSettings)
{
	JPH_ADD_ATTRIBUTE(SoftBodyCreationSettings, mSettings)
	JPH_ADD_ATTRIBUTE(SoftBodyCreationSettings, mPosition)
	JPH_ADD_ATTRIBUTE(SoftBodyCreationSettings, mRotation)
	JPH_ADD_ATTRIBUTE(SoftBodyCreationSettings, mUserData)
	JPH_ADD_ENUM_ATTRIBUTE(SoftBodyCreationSettings, mObjectLayer)
	JPH_ADD_ATTRIBUTE(SoftBodyCreationSettings, mCollisionGroup)
	JPH_ADD_ATTRIBUTE(SoftBodyCreationSettings, mNumIterations)
	JPH_ADD_ATTRIBUTE(SoftBodyCreationSettings, mRestitution)
	JPH_ADD_ATTRIBUTE(SoftBodyCreationSettings, mFriction)
	JPH_ADD_ATTRIBUTE(SoftBodyCreationSettings, mPressure)
	JPH_ADD_ATTRIBUTE(SoftBodyCreationSettings, mGravityFactor)
	JPH_ADD_ATTRIBUTE(SoftBodyCreationSettings, mUpdatePosition)
	JPH_ADD_ATTRIBUTE(SoftBodyCreationSettings, mMakeRotationIdentity)
}

void SoftBodyCreationSettings::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(mPosition);
	inStream.Write(mRotation);
	inStream.Write(mUserData);
	inStream.Write(mObjectLayer);
	mCollisionGroup.SaveBinaryState(inStream);
	inStream.Write(mNumIterations);
	inStream.Write(mRestitution);
	inStream.Write(mFriction);
	inStream.Write(mPressure);
	inStream.Write(mGravityFactor);
	inStream.Write(mUpdatePosition);
	inStream.Write(mMakeRotationIdentity);
}

void SoftBodyCreationSettings::RestoreBinaryState(StreamIn &inStream)
{
	inStream.Read(mPosition);
	inStream.Read(mRotation);
	inStream.Read(mUserData);
	inStream.Read(mObjectLayer);
	mCollisionGroup.RestoreBinaryState(inStream);
	inStream.Read(mNumIterations);
	inStream.Read(mRestitution);
	inStream.Read(mFriction);
	inStream.Read(mPressure);
	inStream.Read(mGravityFactor);
	inStream.Read(mUpdatePosition);
	inStream.Read(mMakeRotationIdentity);
}

JPH_NAMESPACE_END
