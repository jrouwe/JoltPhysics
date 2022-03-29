// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Constraints/MotorSettings.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(MotorSettings)
{
	JPH_ADD_ATTRIBUTE(MotorSettings, mFrequency)
	JPH_ADD_ATTRIBUTE(MotorSettings, mDamping)
	JPH_ADD_ATTRIBUTE(MotorSettings, mMinForceLimit)
	JPH_ADD_ATTRIBUTE(MotorSettings, mMaxForceLimit)
	JPH_ADD_ATTRIBUTE(MotorSettings, mMinTorqueLimit)
	JPH_ADD_ATTRIBUTE(MotorSettings, mMaxTorqueLimit)
}

void MotorSettings::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(mFrequency);
	inStream.Write(mDamping);
	inStream.Write(mMinForceLimit);
	inStream.Write(mMaxForceLimit);
	inStream.Write(mMinTorqueLimit);
	inStream.Write(mMaxTorqueLimit);
}

void MotorSettings::RestoreBinaryState(StreamIn &inStream)
{
	inStream.Read(mFrequency);
	inStream.Read(mDamping);
	inStream.Read(mMinForceLimit);
	inStream.Read(mMaxForceLimit);
	inStream.Read(mMinTorqueLimit);
	inStream.Read(mMaxTorqueLimit);
}

JPH_NAMESPACE_END
