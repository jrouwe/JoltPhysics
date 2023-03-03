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
		inline uint			GetNumContacts() const								{ return uint(mContactBufferEnd - mContactBufferBegin); }
		inline uint 		GetNumConstraints() const							{ return uint(mConstraintBufferEnd - mConstraintBufferBegin); }
		inline uint			GetNumItems() const									{ return GetNumContacts() + GetNumConstraints(); }

		uint32 *			mContactBufferBegin;								///< Begin of the contact buffer
		uint32 *			mContactBufferEnd;									///< End of the contact buffer

		uint32 *			mConstraintBufferBegin;								///< Begin of the constraint buffer
		uint32 *			mConstraintBufferEnd;								///< End of the constraint buffer
	};

public:
	static constexpr uint	cNumGroups = sizeof(GroupMask) * 8;
	static constexpr uint	cNonParallelGroupIdx = cNumGroups - 1;

	/// Status code for retrieving a batch
	enum class EStatus
	{
		WaitingForBatch,														///< Work is expected to be available later
		BatchRetrieved,															///< Work is being returned
		AllBatchesDone,															///< No further work is expected from this group
	};

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

		/// Reset current status so that no work can be picked up from this group
		inline void			ResetStatus()
		{
			mStatus.store((uint64(cNonParallelGroupIdx) << StatusGroupShift) | StatusItemMask, memory_order_relaxed);
		}

		/// Make the first batch available to other threads
		inline void			StartFirstBatch()
		{
			uint group_index = mNumGroups > 0? 0 : cNonParallelGroupIdx;
			mStatus.store(uint64(group_index) << StatusGroupShift, memory_order_release);
		}

		/// Fetch the next batch to process
		EStatus				FetchNextBatch(uint32 *&outConstraintsBegin, uint32 *&outConstraintsEnd, uint32 *&outContactsBegin, uint32 *&outContactsEnd);

		/// Mark a batch as processed, returns true if this is the final iteration for the batch
		bool				MarkBatchProcessed(const uint32 *inConstraintsBegin, const uint32 *inConstraintsEnd, const uint32 *inContactsBegin, const uint32 *inContactsEnd);

	private:
		friend class IslandGroupBuilder;

		enum EIterationStatus : uint64
		{
			StatusIterationMask		= 0xffff000000000000,
			StatusIterationShift	= 48,
			StatusGroupMask			= 0x0000ffff00000000,
			StatusGroupShift		= 32,
			StatusItemMask			= 0x00000000ffffffff,
		};

		static inline int	sGetIteration(uint64 inStatus)
		{
			return int((inStatus & StatusIterationMask) >> StatusIterationShift);
		}

		static inline uint	sGetGroup(uint64 inStatus)
		{
			return uint((inStatus & StatusGroupMask) >> StatusGroupShift);
		}

		static inline uint	sGetItem(uint64 inStatus)
		{
			return uint(inStatus & StatusItemMask);
		}

		Group				mGroups[cNumGroups];								///< Data per group
		uint				mNumGroups;											///< Number of groups that were created (excluding the non-parallel group)
		int					mNumIterations;										///< Number of iterations to do
		atomic<uint64>		mStatus;											///< Status of the group, see EIterationStatus
		atomic<uint>		mItemsProcessed;									///< Number of items that have been marked as processed
	};

	/// Destructor
							~IslandGroupBuilder();

	/// Prepare the island group builder by allocating memory
	void					Prepare(const IslandBuilder &inIslandBuilder, uint32 inNumActiveBodies, TempAllocator *inTempAllocator);

	/// Assign two bodies to a group. Returns the group index.
	uint					AssignGroup(const Body *inBody1, const Body *inBody2);

	/// Force a body to be in a non parallel group. Returns the group index.
	uint					AssignToNonParallelGroup(const Body *inBody);

	/// Build groups for a single island, the created groups willb e added to the list of batches and can be fetched with FetchNextBatch
	bool					BuildGroupsForIsland(uint32 inIslandIndex, const IslandBuilder &inIslandBuilder, const BodyManager &inBodyManager, const ContactConstraintManager &inContactManager, Constraint **inActiveConstraints, int inNumIterations);

	/// Fetch the next batch to process, returns a handle in outGroupedIslandIndex that must be provided to MarkBatchProcessed when complete
	EStatus					FetchNextBatch(uint &outGroupedIslandIndex, uint32 *&outConstraintsBegin, uint32 *&outConstraintsEnd, uint32 *&outContactsBegin, uint32 *&outContactsEnd);

	/// Mark a batch as processed, returns true if this is the final iteration for the batch
	bool					MarkBatchProcessed(uint inGroupedIslandIndex, const uint32 *inConstraintsBegin, const uint32 *inConstraintsEnd, const uint32 *inContactsBegin, const uint32 *inContactsEnd);

	/// Reset the group builder
	void					Reset(TempAllocator *inTempAllocator);

private:
	// Reset group ids for all bodies in the island
	void					ResetGroups(const Body *inBodies, const BodyID *inBodiesStart, const BodyID *inBodiesEnd);

	static constexpr uint	cGroupBuilderTreshold = 256;						///< If the number of constraints + contacts in an island is larger than this, we will try to build groups
	static constexpr uint	cGroupCombineTreshold = 128;						///< If the number of constraints + contacts in a group is lower than this, we will merge this group into the 'non-parallel group'
	static constexpr uint	cBatchSize = 64;									///< Number of items to process in a constraint batch

	uint32					mNumActiveBodies = 0;								///< Cached number of active bodies

	GroupMask *				mGroupMasks = nullptr;								///< Group bits for each body in the BodyManager::mActiveBodies list

	uint32 *				mContactAndConstaintsGroupIdx = nullptr;			///< Buffer to store the group index per constraint or contact
	uint32 *				mContactAndConstraintIndices = nullptr;				///< Buffer to store the ordered constraint indices per group
	uint					mContactAndConstraintsSize = 0;						///< Total size of mContactAndConstraintsGroupIdx and mContactAndConstraintIndices
	atomic<uint>			mContactAndConstraintsNextFree { 0 };				///< Next element that is free in both buffers

	uint					mNumGroupedIslands = 0;								///< Total number of islands that required grouping
	Groups *				mGroupedIslands = nullptr;							///< List of islands that required grouping
	atomic<uint>			mNextGroupedIsland = 0;								///< Next grouped island to pick from mGroupedIslands
	atomic<uint>			mNumGroupedIslandsCreated = 0;						///< Number of grouped islands that have been fully created and are available for other threads to read
};

JPH_NAMESPACE_END
