// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/NonCopyable.h>
#include <Jolt/Core/Atomics.h>

JPH_NAMESPACE_BEGIN

class Body;
class BodyID;
class IslandBuilder;
class TempAllocator;
class Constraint;
class BodyManager;
class ContactConstraintManager;

/// Assigns bodies in large islands to a group that can run in parallel
class IslandGroupBuilder : public NonCopyable
{
private:
	using					GroupMask = uint16;

	/// Describes a group of constraints and contacts
	struct Group
	{
		uint32 *			mContactBufferBegin;								///< Begin of the contact buffer
		uint32 *			mContactBufferEnd;									///< End of the contact buffer

		uint32 *			mConstraintBufferBegin;								///< Begin of the constraint buffer
		uint32 *			mConstraintBufferEnd;								///< End of the constraint buffer
	};

public:
	static constexpr uint	cNumGroups = sizeof(GroupMask) * 8;
	static constexpr uint	cNonParallelGroupIdx = cNumGroups - 1;

	/// Structure that describes the resulting groups from the group builder
	class Groups
	{
	public:
		inline uint			GetNumGroups() const
		{
			return mNumGroups;
		}

		inline void			GetConstraintsInGroup(uint inGroupIndex, uint32 *&outConstraintsBegin, uint32 *&outConstraintsEnd) const
		{
			const Group &group = mGroups[inGroupIndex];
			outConstraintsBegin = group.mConstraintBufferBegin;
			outConstraintsEnd = group.mConstraintBufferEnd;
		}

		inline void			GetContactsInGroup(uint inGroupIndex, uint32 *&outContactsBegin, uint32 *&outContactsEnd) const
		{
			const Group &group = mGroups[inGroupIndex];
			outContactsBegin = group.mContactBufferBegin;
			outContactsEnd = group.mContactBufferEnd;
		}

	private:
		friend class IslandGroupBuilder;

		Group				mGroups[cNumGroups];								///< Data per group
		uint				mNumGroups = 0;										///< Number of groups that were detected
	};

	/// Destructor
							~IslandGroupBuilder();

	/// Prepare the island group builder by allocating memory
	void					Prepare(const IslandBuilder &inIslandBuilder, uint32 inNumActiveBodies, TempAllocator *inTempAllocator);

	/// Build groups for a single island
	void					BuildGroupsForIsland(uint32 inIslandIndex, const IslandBuilder &inIslandBuilder, const BodyManager &inBodyManager, const ContactConstraintManager &inContactManager, Constraint **inActiveConstraints, Groups &outGroups);

	/// Assign two bodies to a group. Returns the group index.
	uint					AssignGroup(const Body *inBody1, const Body *inBody2);

	/// Force a body to be in a non parallel group. Returns the group index.
	uint					AssignToNonParallelGroup(const Body *inBody);

	/// Reset the group builder
	void					Reset(TempAllocator *inTempAllocator);

private:
	// Reset group ids for all bodies in the island
	void					ResetGroups(const Body *inBodies, const BodyID *inBodiesStart, const BodyID *inBodiesEnd);

	static constexpr uint	cGroupBuilderTreshold = 100;						///< If the number of constraints + contacts in an island is larger than this, we will try to build groups
	static constexpr uint	cGroupCombineTreshold = 50;							///< If the number of constraints + contacts in a group is lower than this, we will merge this group into the 'non-parallel group'

	uint32					mNumActiveBodies = 0;								///< Cached number of active bodies

	GroupMask *				mGroupMasks = nullptr;								///< Group bits for each body in the BodyManager::mActiveBodies list

	uint32 *				mContactAndConstaintsGroupIdx = nullptr;			///< Buffer to store the group index per constraint or contact
	uint32 *				mContactAndConstraintIndices = nullptr;				///< Buffer to store the ordered constraint indices per group
	uint					mContactAndConstraintsSize = 0;						///< Total size of mContactAndConstraintsGroupIdx and mContactAndConstraintIndices
	atomic<uint>			mContactAndConstraintsNextFree { 0 };				///< Next element that is free in both buffers
};

JPH_NAMESPACE_END
