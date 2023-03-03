// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/IslandGroupBuilder.h>
#include <Jolt/Physics/IslandBuilder.h>
#include <Jolt/Physics/Constraints/Constraint.h>
#include <Jolt/Physics/Constraints/ContactConstraintManager.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/Core/TempAllocator.h>

JPH_NAMESPACE_BEGIN

IslandGroupBuilder::EStatus IslandGroupBuilder::Groups::FetchNextBatch(uint32 *&outConstraintsBegin, uint32 *&outConstraintsEnd, uint32 *&outContactsBegin, uint32 *&outContactsEnd)
{
	{
		// First check if we can get a new batch (doing a relaxed read to avoid hammering an atomic)
		uint64 status = mStatus.load(memory_order_relaxed);
		if (sGetIteration(status) >= mNumIterations)
			return EStatus::AllBatchesDone;

		uint group_index = sGetGroup(status);
		uint item = sGetItem(status);
		if (group_index == cNonParallelGroupIdx)
		{
			// Non parallel group needs to be taken as a single batch, only the thread that takes element 0 will do it
			if (item != 0)
				return EStatus::WaitingForBatch;
		}
		else
		{
			// Parallel group is split into batches
			JPH_ASSERT(group_index < mNumGroups);
			const Group &group = mGroups[group_index];
			if (item >= group.GetNumItems())
				return EStatus::WaitingForBatch;
		}
	}

	// Then try to actually get the batch
	uint64 status = mStatus.fetch_add(cBatchSize, memory_order_acquire);
	if (sGetIteration(status) >= mNumIterations)
		return EStatus::AllBatchesDone;

	uint group_index = sGetGroup(status);
	JPH_ASSERT(group_index < mNumGroups || group_index == cNonParallelGroupIdx);
	const Group &group = mGroups[group_index];
	uint item_begin = sGetItem(status);
	if (group_index == cNonParallelGroupIdx)
	{
		if (item_begin == 0)
		{
			// Non-parallel group always goes as a single batch
			outConstraintsBegin = group.mConstraintBufferBegin;
			outConstraintsEnd = group.mConstraintBufferEnd;
			outContactsBegin = group.mContactBufferBegin;
			outContactsEnd = group.mContactBufferEnd;
			return EStatus::BatchRetrieved;
		}
		else
		{
			// Otherwise we're done with this group
			return EStatus::WaitingForBatch;
		}
	}

	// Parallel group is split into batches
	uint num_constraints = group.GetNumConstraints();
	uint num_contacts = group.GetNumContacts();
	uint num_items = num_constraints + num_contacts;
	if (item_begin >= num_items)
		return EStatus::WaitingForBatch;

	uint item_end = min(item_begin + cBatchSize, num_items);
	if (item_end >= num_constraints)
	{
		if (item_begin < num_constraints)
		{
			// Partially from constraints and partially from contacts
			outConstraintsBegin = group.mConstraintBufferBegin + item_begin;
			outConstraintsEnd = group.mConstraintBufferEnd;
		}
		else
		{
			// Only contacts
			outConstraintsBegin = nullptr;
			outConstraintsEnd = nullptr;
		}

		outContactsBegin = group.mContactBufferBegin + (max(item_begin, num_constraints) - num_constraints);
		outContactsEnd = group.mContactBufferBegin + (item_end - num_constraints);
	}
	else
	{
		// Only constraints
		outConstraintsBegin = group.mConstraintBufferBegin + item_begin;
		outConstraintsEnd = group.mConstraintBufferBegin + item_end;

		outContactsBegin = nullptr;
		outContactsEnd = nullptr;
	}
	return EStatus::BatchRetrieved;
}

bool IslandGroupBuilder::Groups::MarkBatchProcessed(const uint32 *inConstraintsBegin, const uint32 *inConstraintsEnd, const uint32 *inContactsBegin, const uint32 *inContactsEnd)
{
	// Add the number of items we processed to the total number of items processed
	uint num_items_processed = uint(inConstraintsEnd - inConstraintsBegin) + uint(inContactsEnd - inContactsBegin);
	JPH_ASSERT(num_items_processed > 0); // Logic will break if we mark a block of 0 items as processed
	uint total_items_processed = mItemsProcessed.fetch_add(num_items_processed, memory_order_release) + num_items_processed;

	// We fetched this batch, nobody should change the group and or iteration until we mark the last batch as processed so we can safely get the current status
	uint64 status = mStatus.load(memory_order_relaxed);
	uint group_index = sGetGroup(status);
	JPH_ASSERT(group_index < mNumGroups || group_index == cNonParallelGroupIdx);
	const Group &group = mGroups[group_index];
	uint num_items_in_group = group.GetNumItems();

	// Determine if this is the last iteration before possibly incrementing it
	int iteration = sGetIteration(status);
	bool is_last_iteration = iteration == mNumIterations - 1;

	// Check if we're at the end of the group
	if (total_items_processed >= num_items_in_group)
	{
		JPH_ASSERT(total_items_processed == num_items_in_group); // Should not overflow, that means we're retiring more items than we should process

		// Set items processed back to 0 for the next group/iteration
		mItemsProcessed.store(0, memory_order_relaxed);

		// Determine next group
		do
		{
			if (group_index == cNonParallelGroupIdx)
			{
				// At start of next iteration
				group_index = 0;
				++iteration;
			}
			else
			{
				// At start of next group
				++group_index;
			}
		
			// If we're beyond the end of groups, go to the non-parallel group
			if (group_index >= mNumGroups)
				group_index = cNonParallelGroupIdx;
		}
		while (iteration < mNumIterations
			&& mGroups[group_index].GetNumItems() == 0); // We don't support processing empty groups, skip to the next group in this case

		mStatus.store((uint64(iteration) << StatusIterationShift) | (uint64(group_index) << StatusGroupShift), memory_order_release);
	}

	return is_last_iteration;
}

IslandGroupBuilder::~IslandGroupBuilder()
{
	JPH_ASSERT(mGroupMasks == nullptr);
	JPH_ASSERT(mContactAndConstaintsGroupIdx == nullptr);
	JPH_ASSERT(mContactAndConstraintIndices == nullptr);
}

void IslandGroupBuilder::Prepare(const IslandBuilder &inIslandBuilder, uint32 inNumActiveBodies, TempAllocator *inTempAllocator)
{
	JPH_PROFILE_FUNCTION();

	// Count the total number of constraints and contacts that we will be putting in groups
	mContactAndConstraintsSize = 0;
	for (uint32 island = 0; island < inIslandBuilder.GetNumIslands(); ++island)
	{
		// Get the contacts in this island
		uint32 *contacts_start, *contacts_end;
		inIslandBuilder.GetContactsInIsland(island, contacts_start, contacts_end);
		uint num_contacts_in_island = uint(contacts_end - contacts_start);

		// Get the constraints in this island
		uint32 *constraints_start, *constraints_end;
		inIslandBuilder.GetConstraintsInIsland(island, constraints_start, constraints_end);
		uint num_constraints_in_island = uint(constraints_end - constraints_start);

		uint island_size = num_contacts_in_island + num_constraints_in_island;
		if (island_size >= cGroupBuilderTreshold)
		{
			mNumGroupedIslands++;
			mContactAndConstraintsSize += island_size;
		}
		else
			break; // If this island doesn't have enough constraints, the next islands won't either
	}

	if (mContactAndConstraintsSize > 0)
	{
		mNumActiveBodies = inNumActiveBodies;

		// Allocate group mask buffer
		mGroupMasks = (GroupMask *)inTempAllocator->Allocate(mNumActiveBodies * sizeof(GroupMask));

		// Allocate contact and constraint buffer
		mContactAndConstaintsGroupIdx = (uint32 *)inTempAllocator->Allocate(mContactAndConstraintsSize * sizeof(uint32));
		mContactAndConstraintIndices = (uint32 *)inTempAllocator->Allocate(mContactAndConstraintsSize * sizeof(uint32));

		// Allocate island group buffer
		mGroupedIslands = (Groups *)inTempAllocator->Allocate(mNumGroupedIslands * sizeof(Groups));

		// Prevent any of the groups from being picked up as work
		for (uint i = 0; i < mNumGroupedIslands; ++i)
			mGroupedIslands[i].ResetStatus();
	}
}

uint IslandGroupBuilder::AssignGroup(const Body *inBody1, const Body *inBody2)
{
	uint32 idx1 = inBody1->GetIndexInActiveBodiesInternal();
	uint32 idx2 = inBody2->GetIndexInActiveBodiesInternal();

	// Test if either index is negative
	if (idx1 == Body::cInactiveIndex || !inBody1->IsDynamic())
	{
		// Body 1 is not active or a kinematic body, so we only need to set 1 group
		JPH_ASSERT(idx2 < mNumActiveBodies);
		GroupMask &mask = mGroupMasks[idx2];
		uint group = min(CountTrailingZeros(~mask), cNonParallelGroupIdx);
		mask |= GroupMask(1 << group);
		return group;
	}
	else if (idx2 == Body::cInactiveIndex || !inBody2->IsDynamic())
	{
		// Body 2 is not active or a kinematic body, so we only need to set 1 group
		JPH_ASSERT(idx1 < mNumActiveBodies);
		GroupMask &mask = mGroupMasks[idx1];
		uint group = min(CountTrailingZeros(~mask), cNonParallelGroupIdx);
		mask |= GroupMask(1 << group);
		return group;
	}
	else
	{
		// If both bodies are active, we need to set 2 groups
		JPH_ASSERT(idx1 < mNumActiveBodies);
		JPH_ASSERT(idx2 < mNumActiveBodies);
		GroupMask &mask1 = mGroupMasks[idx1];
		GroupMask &mask2 = mGroupMasks[idx2];
		uint group = min(CountTrailingZeros((~mask1) & (~mask2)), cNonParallelGroupIdx);
		GroupMask mask = GroupMask(1 << group);
		mask1 |= mask;
		mask2 |= mask;
		return group;
	}
}

uint IslandGroupBuilder::AssignToNonParallelGroup(const Body *inBody)
{
	uint32 idx = inBody->GetIndexInActiveBodiesInternal();
	if (idx != Body::cInactiveIndex)
	{
		JPH_ASSERT(idx < mNumActiveBodies);
		mGroupMasks[idx] |= 1 << cNonParallelGroupIdx;
	}

	return cNonParallelGroupIdx;
}

bool IslandGroupBuilder::BuildGroupsForIsland(uint32 inIslandIndex, const IslandBuilder &inIslandBuilder, const BodyManager &inBodyManager, const ContactConstraintManager &inContactManager, Constraint **inActiveConstraints, int inNumIterations)
{
	JPH_PROFILE_FUNCTION();

	// Get the contacts in this island
	uint32 *contacts_start, *contacts_end;
	inIslandBuilder.GetContactsInIsland(inIslandIndex, contacts_start, contacts_end);
	uint num_contacts_in_island = uint(contacts_end - contacts_start);

	// Get the constraints in this island
	uint32 *constraints_start, *constraints_end;
	inIslandBuilder.GetConstraintsInIsland(inIslandIndex, constraints_start, constraints_end);
	uint num_constraints_in_island = uint(constraints_end - constraints_start);

	// Check if it exceeds the treshold
	uint island_size = num_contacts_in_island + num_constraints_in_island;
	if (island_size < cGroupBuilderTreshold)
		return false;

	// Get bodies in this island
	BodyID *bodies_start, *bodies_end;
	inIslandBuilder.GetBodiesInIsland(inIslandIndex, bodies_start, bodies_end);

	// Reset the group for all bodies in this island
	Body const * const *bodies = inBodyManager.GetBodies().data();
	for (const BodyID *b = bodies_start; b < bodies_end; ++b)
		mGroupMasks[bodies[b->GetIndex()]->GetIndexInActiveBodiesInternal()] = 0;

	// Count the number of contacts and constraints per group
	uint num_contacts_in_group[cNumGroups] = { };
	uint num_constraints_in_group[cNumGroups] = { };

	// Get space to store group indices
	uint offset = mContactAndConstraintsNextFree.fetch_add(island_size);
	uint32 *contact_group_idx = mContactAndConstaintsGroupIdx + offset;
	uint32 *constraint_group_idx = contact_group_idx + num_contacts_in_island;

	// Assign the contact to a group
	uint32 *cur_contact_group_idx = contact_group_idx;
	for (uint32 *c = contacts_start; c < contacts_end; ++c)
	{
		const Body *body1, *body2;
		inContactManager.GetAffectedBodies(*c, body1, body2);
		uint group = AssignGroup(body1, body2);
		num_contacts_in_group[group]++;
		*cur_contact_group_idx++ = group;
	}

	// Assign the constraint to a group
	uint32 *cur_constraint_group_idx = constraint_group_idx;
	for (uint32 *c = constraints_start; c < constraints_end; ++c)
	{
		uint group = inActiveConstraints[*c]->BuildIslandGroups(*this);
		num_constraints_in_group[group]++;
		*cur_constraint_group_idx++ = group;
	}

	// Start with 0 groups
	uint group_remap_table[cNumGroups];
	uint new_group_idx = mNextGroupedIsland.fetch_add(1, memory_order_relaxed);
	JPH_ASSERT(new_group_idx < mNumGroupedIslands);
	Groups &groups = mGroupedIslands[new_group_idx];
	groups.mNumGroups = 0;
	groups.mNumIterations = inNumIterations;
	groups.mItemsProcessed.store(0, memory_order_relaxed);

	// Allocate space to store the sorted constraint and contact indices per group
	uint32 *buffer = mContactAndConstraintIndices + offset;
	uint32 *constraint_buffer_cur[cNumGroups], *contact_buffer_cur[cNumGroups];
	for (uint g = 0; g < cNumGroups; ++g)
	{
		// If this group doesn't contain enough constraints and contacts, we will combine it with the non parallel group
		if (num_constraints_in_group[g] + num_contacts_in_group[g] < cGroupCombineTreshold
			&& g < cNonParallelGroupIdx) // The non-parallel group cannot merge into itself
		{
			// Remap it
			group_remap_table[g] = cNonParallelGroupIdx;

			// Add the counts to the non parallel group
			num_contacts_in_group[cNonParallelGroupIdx] += num_contacts_in_group[g];
			num_constraints_in_group[cNonParallelGroupIdx] += num_constraints_in_group[g];
		}
		else
		{
			// This group is valid, map it to the next empty slot
			uint target_group;
			if (g < cNonParallelGroupIdx)
				target_group = groups.mNumGroups++;
			else
				target_group = cNonParallelGroupIdx;
			Group &group = groups.mGroups[target_group];
			group_remap_table[g] = target_group;

			// Allocate space for contacts
			group.mContactBufferBegin = buffer;
			group.mContactBufferEnd = group.mContactBufferBegin + num_contacts_in_group[g];

			// Allocate space for constraints
			group.mConstraintBufferBegin = group.mContactBufferEnd;
			group.mConstraintBufferEnd = group.mConstraintBufferBegin + num_constraints_in_group[g];

			// Store start for each group
			contact_buffer_cur[target_group] = group.mContactBufferBegin;
			constraint_buffer_cur[target_group] = group.mConstraintBufferBegin;

			// Update buffer pointer
			buffer = group.mConstraintBufferEnd;
		}
	}

	// Group the contacts
	for (uint c = 0; c < num_contacts_in_island; ++c)
	{
		uint group = group_remap_table[contact_group_idx[c]];
		*contact_buffer_cur[group]++ = contacts_start[c];
	}

	// Group the constraints
	for (uint c = 0; c < num_constraints_in_island; ++c)
	{
		uint group = group_remap_table[constraint_group_idx[c]];
		*constraint_buffer_cur[group]++ = constraints_start[c];
	}

#ifdef JPH_ENABLE_ASSERTS
	for (uint g = 0; g < cNumGroups; ++g)
	{
		// If there are no more groups, process the non-parallel group
		if (g >= groups.mNumGroups)
			g = cNonParallelGroupIdx;

		// Check that we wrote all elements
		Group &group = groups.mGroups[g];
		JPH_ASSERT(contact_buffer_cur[g] == group.mContactBufferEnd);
		JPH_ASSERT(constraint_buffer_cur[g] == group.mConstraintBufferEnd);
	}

#ifdef _DEBUG
	// Validate that the groups are indeed not touching the same body
	for (uint g = 0; g < groups.mNumGroups; ++g)
	{
		Array<bool> body_used(mNumActiveBodies, false);

		// Validate contacts
		uint32 *group_contacts_begin, *group_contacts_end;
		groups.GetContactsInGroup(g, group_contacts_begin, group_contacts_end);
		for (uint32 *c = group_contacts_begin; c < group_contacts_end; ++c)
		{
			const Body *body1, *body2;
			inContactManager.GetAffectedBodies(*c, body1, body2);

			uint32 idx1 = body1->GetIndexInActiveBodiesInternal();
			if (idx1 != Body::cInactiveIndex && !body1->IsDynamic())
			{
				JPH_ASSERT(!body_used[idx1]);
				body_used[idx1] = true;
			}

			uint32 idx2 = body2->GetIndexInActiveBodiesInternal();
			if (idx2 != Body::cInactiveIndex && !body2->IsDynamic())
			{
				JPH_ASSERT(!body_used[idx2]);
				body_used[idx2] = true;
			}
		}
	}
#endif // _DEBUG
#endif // JPH_ENABLE_ASSERTS

	// Allow other threads to pick up this group now
	groups.StartFirstBatch();
	mNumGroupedIslandsCreated.fetch_add(1, memory_order_release);
	return true;
}

IslandGroupBuilder::EStatus IslandGroupBuilder::FetchNextBatch(uint &outGroupedIslandIndex, uint32 *&outConstraintsBegin, uint32 *&outConstraintsEnd, uint32 *&outContactsBegin, uint32 *&outContactsEnd)
{
	// We can't be done when all islands haven't been submitted yet
	uint num_grouped_islands_created = mNumGroupedIslandsCreated.load(memory_order_acquire);
	bool all_done = num_grouped_islands_created == mNumGroupedIslands;

	for (Groups *g = mGroupedIslands; g < mGroupedIslands + num_grouped_islands_created; ++g)
		switch (g->FetchNextBatch(outConstraintsBegin, outConstraintsEnd, outContactsBegin, outContactsEnd))
		{
		case EStatus::AllBatchesDone:
			break;

		case EStatus::WaitingForBatch:
			all_done = false;
			break;

		case EStatus::BatchRetrieved:
			outGroupedIslandIndex = uint(g - mGroupedIslands);
			return EStatus::BatchRetrieved;
		}

	return all_done? EStatus::AllBatchesDone : EStatus::WaitingForBatch;
}

bool IslandGroupBuilder::MarkBatchProcessed(uint inGroupedIslandIndex, const uint32 *inConstraintsBegin, const uint32 *inConstraintsEnd, const uint32 *inContactsBegin, const uint32 *inContactsEnd)
{
	JPH_ASSERT(inGroupedIslandIndex < mNextGroupedIsland.load(memory_order_relaxed));
	Groups &groups = mGroupedIslands[inGroupedIslandIndex];
	return groups.MarkBatchProcessed(inConstraintsBegin, inConstraintsEnd, inContactsBegin, inContactsEnd);
}

void IslandGroupBuilder::Reset(TempAllocator *inTempAllocator)
{
	JPH_PROFILE_FUNCTION();

	// Everything should have been used
	JPH_ASSERT(mContactAndConstraintsNextFree == mContactAndConstraintsSize);
	JPH_ASSERT(mNumGroupedIslandsCreated == mNumGroupedIslands);

	// Free grouped islands
	if (mNumGroupedIslands > 0)
	{
		inTempAllocator->Free(mGroupedIslands, mNumGroupedIslands * sizeof(Groups));
		mGroupedIslands = nullptr;

		mNumGroupedIslands = 0;
		mNextGroupedIsland = 0;
		mNumGroupedIslandsCreated = 0;
	}

	// Free contact and constraint buffers
	if (mContactAndConstraintsSize > 0)
	{
		inTempAllocator->Free(mContactAndConstraintIndices, mContactAndConstraintsSize * sizeof(uint32));
		mContactAndConstraintIndices = nullptr;

		inTempAllocator->Free(mContactAndConstaintsGroupIdx, mContactAndConstraintsSize * sizeof(uint32));
		mContactAndConstaintsGroupIdx = nullptr;

		mContactAndConstraintsNextFree = 0;
	}

	// Free group masks
	if (mGroupMasks != nullptr)
	{
		inTempAllocator->Free(mGroupMasks, mNumActiveBodies * sizeof(GroupMask));
		mGroupMasks = nullptr;
		mNumActiveBodies = 0;
	}
}

JPH_NAMESPACE_END
