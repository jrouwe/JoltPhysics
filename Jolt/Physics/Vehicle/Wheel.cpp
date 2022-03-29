// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Vehicle/Wheel.h>
#include <Jolt/Physics/Vehicle/VehicleConstraint.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(WheelSettings)
{
	JPH_ADD_ATTRIBUTE(WheelSettings, mPosition)
	JPH_ADD_ATTRIBUTE(WheelSettings, mDirection)
	JPH_ADD_ATTRIBUTE(WheelSettings, mSuspensionMinLength)
	JPH_ADD_ATTRIBUTE(WheelSettings, mSuspensionMaxLength)
	JPH_ADD_ATTRIBUTE(WheelSettings, mSuspensionPreloadLength)
	JPH_ADD_ATTRIBUTE(WheelSettings, mSuspensionFrequency)
	JPH_ADD_ATTRIBUTE(WheelSettings, mSuspensionDamping)
	JPH_ADD_ATTRIBUTE(WheelSettings, mRadius)
	JPH_ADD_ATTRIBUTE(WheelSettings, mWidth)
}

void WheelSettings::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(mPosition);
	inStream.Write(mDirection);
	inStream.Write(mSuspensionMinLength);
	inStream.Write(mSuspensionMaxLength);
	inStream.Write(mSuspensionPreloadLength);
	inStream.Write(mSuspensionFrequency);
	inStream.Write(mSuspensionDamping);
	inStream.Write(mRadius);
	inStream.Write(mWidth);
}

void WheelSettings::RestoreBinaryState(StreamIn &inStream)
{
	inStream.Read(mPosition);
	inStream.Read(mDirection);
	inStream.Read(mSuspensionMinLength);
	inStream.Read(mSuspensionMaxLength);
	inStream.Read(mSuspensionPreloadLength);
	inStream.Read(mSuspensionFrequency);
	inStream.Read(mSuspensionDamping);
	inStream.Read(mRadius);
	inStream.Read(mWidth);
}

Wheel::Wheel(const WheelSettings &inSettings) :
	mSettings(&inSettings),
	mContactLength(inSettings.mSuspensionMaxLength + inSettings.mRadius)
{
	JPH_ASSERT(inSettings.mDirection.IsNormalized());
	JPH_ASSERT(inSettings.mSuspensionMinLength >= 0.0f);
	JPH_ASSERT(inSettings.mSuspensionMaxLength >= inSettings.mSuspensionMinLength);
	JPH_ASSERT(inSettings.mSuspensionPreloadLength >= 0.0f);
	JPH_ASSERT(inSettings.mSuspensionFrequency > 0.0f);
	JPH_ASSERT(inSettings.mSuspensionDamping >= 0.0f);
	JPH_ASSERT(inSettings.mRadius > 0.0f);
	JPH_ASSERT(inSettings.mWidth >= 0.0f);
}

bool Wheel::SolveLongitudinalConstraintPart(const VehicleConstraint &inConstraint, float inMinImpulse, float inMaxImpulse) 
{ 
	return mLongitudinalPart.SolveVelocityConstraint(*inConstraint.GetVehicleBody(), *mContactBody, -mContactLongitudinal, inMinImpulse, inMaxImpulse); 
}

bool Wheel::SolveLateralConstraintPart(const VehicleConstraint &inConstraint, float inMinImpulse, float inMaxImpulse) 
{ 
	return mLateralPart.SolveVelocityConstraint(*inConstraint.GetVehicleBody(), *mContactBody, -mContactLateral, inMinImpulse, inMaxImpulse); 
}

JPH_NAMESPACE_END
