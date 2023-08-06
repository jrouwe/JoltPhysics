// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/CollisionGroup.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(CollisionGroup)
{
	JPH_ADD_ATTRIBUTE(CollisionGroup, mGroupFilter)
	JPH_ADD_ATTRIBUTE(CollisionGroup, mGroupID)
	JPH_ADD_ATTRIBUTE(CollisionGroup, mSubGroupID)
}

void CollisionGroup::SaveBinaryState(StreamOut &inStream) const
{
	inStream.Write(mGroupID);
	inStream.Write(mSubGroupID);
}

void CollisionGroup::RestoreBinaryState(StreamIn &inStream)
{
	inStream.Read(mGroupID);
	inStream.Read(mSubGroupID);
}

void CollisionGroup::SaveWithGroupFilter(StreamOut &inStream, GroupFilterToIDMap *ioGroupFilterMap) const
{
	// Save creation settings
	SaveBinaryState(inStream);

	// Save group filter
	const GroupFilter *group_filter = GetGroupFilter();
	if (ioGroupFilterMap == nullptr || group_filter == nullptr)
	{
		// Write null ID
		inStream.Write(~uint32(0));
	}
	else
	{
		GroupFilterToIDMap::const_iterator group_filter_id = ioGroupFilterMap->find(group_filter);
		if (group_filter_id != ioGroupFilterMap->end())
		{
			// Existing group filter, write ID
			inStream.Write(group_filter_id->second);
		}
		else
		{
			// New group filter, write the ID
			uint32 new_group_filter_id = (uint32)ioGroupFilterMap->size();
			(*ioGroupFilterMap)[group_filter] = new_group_filter_id;
			inStream.Write(new_group_filter_id);

			// Write the group filter
			group_filter->SaveBinaryState(inStream);
		}
	}
}

CollisionGroup::CGResult CollisionGroup::sRestoreWithGroupFilter(StreamIn &inStream, IDToGroupFilterMap &ioGroupFilterMap)
{
	CGResult result;

	// Restore collision group
	CollisionGroup cg;
	cg.RestoreBinaryState(inStream);

	// Read group filter
	const GroupFilter *group_filter = nullptr;
	uint32 group_filter_id = ~uint32(0);
	inStream.Read(group_filter_id);
	if (group_filter_id != ~uint32(0))
	{
		// Check if it already exists
		if (group_filter_id >= ioGroupFilterMap.size())
		{
			// New group filter, restore it
			GroupFilter::GroupFilterResult group_filter_result = GroupFilter::sRestoreFromBinaryState(inStream);
			if (group_filter_result.HasError())
			{
				result.SetError(group_filter_result.GetError());
				return result;
			}
			group_filter = group_filter_result.Get();
			JPH_ASSERT(group_filter_id == ioGroupFilterMap.size());
			ioGroupFilterMap.push_back(group_filter);
		}
		else
		{
			// Existing group filter
			group_filter = ioGroupFilterMap[group_filter_id];
		}
	}

	// Set the group filter
	cg.SetGroupFilter(group_filter);
	result.Set(cg);
	return result;
}

JPH_NAMESPACE_END
