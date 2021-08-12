// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Constraints/Constraint.h>
#include <Physics/StateRecorder.h>
#include <ObjectStream/TypeDeclarations.h>
#include <Core/StreamIn.h>
#include <Core/StreamOut.h>
#include <Core/Factory.h>

namespace JPH {

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(ConstraintSettings)
{
	JPH_ADD_BASE_CLASS(ConstraintSettings, SerializableObject)

	JPH_ADD_ATTRIBUTE(ConstraintSettings, mDrawConstraintSize)
}

void ConstraintSettings::SaveBinaryState(StreamOut &inStream) const
{ 
	inStream.Write(GetRTTI()->GetHash());
	inStream.Write(mDrawConstraintSize);
}

void ConstraintSettings::RestoreBinaryState(StreamIn &inStream)
{
	// Type hash read by sRestoreFromBinaryState
	inStream.Read(mDrawConstraintSize);
}

ConstraintSettings::ConstraintResult ConstraintSettings::sRestoreFromBinaryState(StreamIn &inStream)
{
	ConstraintResult result;

	// Read the type of the constraint
	uint32 hash;
	inStream.Read(hash);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to read type id");
		return result;
	}

	// Get the RTTI for the shape
	const RTTI *rtti = Factory::sInstance.Find(hash);
	if (rtti == nullptr)
	{
		result.SetError("Failed to resolve type. Type not registered in factory?");
		return result;
	}

	// Construct and read the data of the shape
	Ref<ConstraintSettings> constraint = reinterpret_cast<ConstraintSettings *>(rtti->CreateObject());
	constraint->RestoreBinaryState(inStream);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to restore constraint");
		return result;
	}

	result.Set(constraint);
	return result;
}

void Constraint::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mEnabled);
}

void Constraint::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mEnabled);
}

} // JPH