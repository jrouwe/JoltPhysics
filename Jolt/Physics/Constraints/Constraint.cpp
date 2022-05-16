// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Constraints/Constraint.h>
#include <Jolt/Physics/StateRecorder.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#include <Jolt/Core/Factory.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(ConstraintSettings)
{
	JPH_ADD_BASE_CLASS(ConstraintSettings, SerializableObject)

	JPH_ADD_ATTRIBUTE(ConstraintSettings, mEnabled)
	JPH_ADD_ATTRIBUTE(ConstraintSettings, mDrawConstraintSize)
}

void ConstraintSettings::SaveBinaryState(StreamOut &inStream) const
{ 
	inStream.Write(GetRTTI()->GetHash());
	inStream.Write(mEnabled);
	inStream.Write(mDrawConstraintSize);
}

void ConstraintSettings::RestoreBinaryState(StreamIn &inStream)
{
	// Type hash read by sRestoreFromBinaryState
	inStream.Read(mEnabled);
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
	const RTTI *rtti = Factory::sInstance->Find(hash);
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

void Constraint::ToConstraintSettings(ConstraintSettings &outSettings) const
{
	outSettings.mEnabled = mEnabled;

#ifdef JPH_DEBUG_RENDERER
	outSettings.mDrawConstraintSize = mDrawConstraintSize;
#endif // JPH_DEBUG_RENDERER
}

JPH_NAMESPACE_END
