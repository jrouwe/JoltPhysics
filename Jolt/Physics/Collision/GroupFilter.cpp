// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
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
	if (group_filter == nullptr)
	{
		result.SetError("Failed to create instance of group filter");
		return result;
	}
	group_filter->RestoreBinaryState(inStream);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to restore group filter");
		return result;
	}

	result.Set(group_filter);
	return result;
}


void GroupFilter::sSaveGroupFilter(StreamOut &inStream, const GroupFilter *inGroupFilter, GroupFilterToIDMap *ioGroupFilterMap)
{
	// Save group filter
	if (ioGroupFilterMap == nullptr || inGroupFilter == nullptr)
	{
		// Write null ID
		inStream.Write(~uint32(0));
	}
	else
	{
		GroupFilterToIDMap::const_iterator group_filter_id = ioGroupFilterMap->find(inGroupFilter);
		if (group_filter_id != ioGroupFilterMap->end())
		{
			// Existing group filter, write ID
			inStream.Write(group_filter_id->second);
		}
		else
		{
			// New group filter, write the ID
			uint32 new_group_filter_id = (uint32)ioGroupFilterMap->size();
			(*ioGroupFilterMap)[inGroupFilter] = new_group_filter_id;
			inStream.Write(new_group_filter_id);

			// Write the group filter
			inGroupFilter->SaveBinaryState(inStream);
		}
	}
}

GroupFilter::GroupFilterResult GroupFilter::sRestoreGroupFilter(StreamIn &inStream, IDToGroupFilterMap &ioGroupFilterMap)
{
	GroupFilterResult result;

	// Read group filter
	uint32 group_filter_id = ~uint32(0);
	inStream.Read(group_filter_id);
	if (group_filter_id != ~uint32(0))
	{
		// Check if it already exists
		if (group_filter_id >= ioGroupFilterMap.size())
		{
			// New group filter, restore it
			result = GroupFilter::sRestoreFromBinaryState(inStream);
			if (result.HasError())
				return result;
			JPH_ASSERT(group_filter_id == ioGroupFilterMap.size());
			ioGroupFilterMap.push_back(result.Get());
		}
		else
		{
			// Existing group filter
			result.Set(ioGroupFilterMap[group_filter_id].GetPtr());
		}
	}
	else
	{
		// Nullptr
		result.Set(nullptr);
	}

	return result;
}

JPH_NAMESPACE_END
