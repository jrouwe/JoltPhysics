// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/GroupFilter.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#include <Jolt/Core/Factory.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_ABSTRACT_BASE(GroupFilter)
{
	JPH_ADD_BASE_CLASS(GroupFilter, SerializableObject)
}

void GroupFilter::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(GetRTTI()->GetHash());
}

void GroupFilter::RestoreBinaryState(StreamIn &inStream)
{
	// RTTI hash is read in sRestoreFromBinaryState
}

GroupFilter::GroupFilterResult GroupFilter::sRestoreFromBinaryState(StreamIn &inStream)
{
	GroupFilterResult result;

	// Read the type of the group filter
	uint32 hash;
	inStream.Read(hash);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to read type hash");
		return result;
	}

	// Get the RTTI for the group filter
	const RTTI *rtti = Factory::sInstance->Find(hash);
	if (rtti == nullptr)
	{
		result.SetError("Failed to create instance of group filter");
		return result;
	}

	// Construct and read the data of the group filter
	Ref<GroupFilter> group_filter = reinterpret_cast<GroupFilter *>(rtti->CreateObject());
	group_filter->RestoreBinaryState(inStream);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to restore group filter");
		return result;
	}

	result.Set(group_filter);
	return result;
}

JPH_NAMESPACE_END
