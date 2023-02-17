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
		int num_contacts_in_island = int(contacts_end - contacts_start);

		// Get the constraints in this island
		uint32 *constraints_start, *constraints_end;
		inIslandBuilder.GetConstraintsInIsland(island, constraints_start, constraints_end);
		int num_constraints_in_island = int(constraints_end - constraints_start);

		int island_size = num_contacts_in_island + num_constraints_in_island;
		if (island_size >= cGroupBuilderTreshold)
			mContactAndConstraintsSize += island_size;
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
		mContactAndConstraintsNextFree = 0;
	}
}

uint IslandGroupBuilder::AssignGroup(const Body *inBody1, const Body *inBody2)
{
	int idx1 = inBody1->GetIndexInActiveBodiesInternal();
	int idx2 = inBody2->GetIndexInActiveBodiesInternal();

	// Test if either index is negative
	if ((idx1 | idx2) < 0)
	{
		// If the body is interacting with a non-active body, we only need to set 1 group
		int active_idx = max(idx1, idx2);
		JPH_ASSERT(active_idx >= 0 && (uint32)active_idx < mNumActiveBodies);
		GroupMask &mask = mGroupMasks[active_idx];
		uint group = min(32 - CountLeadingZeros(mask), cNonParallelGroupIdx);
		mask |= GroupMask(1) << group;
		return group;
	}
	else
	{
		// If both bodies are active, we need to set 2 groups
		JPH_ASSERT(idx1 >= 0 && (uint)idx1 < mNumActiveBodies);
		JPH_ASSERT(idx2 >= 0 && (uint)idx2 < mNumActiveBodies);
		GroupMask &mask1 = mGroupMasks[idx1];
		GroupMask &mask2 = mGroupMasks[idx2];
		uint group = min(32 - CountLeadingZeros(mask1 | mask2), cNonParallelGroupIdx);
		GroupMask mask = GroupMask(1) << group;
		mask1 |= mask;
		mask2 |= mask;
		return group;
	}
}

uint IslandGroupBuilder::AssignToNonParallelGroup(const Body *inBody)
{
	int idx = inBody->GetIndexInActiveBodiesInternal();
	if (idx >= 0)
	{
		JPH_ASSERT((uint)idx < mNumActiveBodies);
		mGroupMasks[idx] |= 1 << cNonParallelGroupIdx;
	}

	return cNonParallelGroupIdx;
}

void IslandGroupBuilder::BuildGroupsForIsland(uint32 inIslandIndex, const IslandBuilder &inIslandBuilder, const BodyManager &inBodyManager, const ContactConstraintManager &inContactManager, Constraint **inActiveConstraints, Groups &outGroups)
{
	JPH_PROFILE_FUNCTION();

	// Get the contacts in this island
	uint32 *contacts_start, *contacts_end;
	inIslandBuilder.GetContactsInIsland(inIslandIndex, contacts_start, contacts_end);
	int num_contacts_in_island = int(contacts_end - contacts_start);

	// Get the constraints in this island
	uint32 *constraints_start, *constraints_end;
	inIslandBuilder.GetConstraintsInIsland(inIslandIndex, constraints_start, constraints_end);
	int num_constraints_in_island = int(constraints_end - constraints_start);

	// Check if it exceeds the treshold
	int island_size = num_contacts_in_island + num_constraints_in_island;
	if (island_size < cGroupBuilderTreshold)
	{
		// Non parallel group doesn't count, it is always checked
		outGroups.mNumGroups = 0;

		// Just assign all constraints and contacts to the non parallel group.
		Group &group = outGroups.mGroups[cNonParallelGroupIdx];
		group.mContactBufferBegin = contacts_start;
		group.mContactBufferEnd = contacts_end;
		group.mConstraintBufferBegin = constraints_start;
		group.mConstraintBufferEnd = constraints_end;
	}
	else
	{
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
		outGroups.mNumGroups = 0;

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
					target_group = outGroups.mNumGroups++;
				else
					target_group = cNonParallelGroupIdx;
				Group &group = outGroups.mGroups[target_group];
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
		for (int c = 0; c < num_contacts_in_island; ++c)
		{
			uint group = group_remap_table[contact_group_idx[c]];
			*contact_buffer_cur[group]++ = contacts_start[c];
		}

		// Group the constraints
		for (int c = 0; c < num_constraints_in_island; ++c)
		{
			uint group = group_remap_table[constraint_group_idx[c]];
			*constraint_buffer_cur[group]++ = constraints_start[c];
		}

	#ifdef JPH_ENABLE_ASSERTS
		for (uint g = 0; g < cNumGroups; ++g)
		{
			// If there are no more groups, process the non-parallel group
			if (g >= outGroups.mNumGroups)
				g = cNonParallelGroupIdx;

			// Check that we wrote all elements
			Group &group = outGroups.mGroups[g];
			JPH_ASSERT(contact_buffer_cur[g] == group.mContactBufferEnd);
			JPH_ASSERT(constraint_buffer_cur[g] == group.mConstraintBufferEnd);
		}
	#endif // JPH_ENABLE_ASSERTS
	}
}

void IslandGroupBuilder::Reset(TempAllocator *inTempAllocator)
{
	JPH_PROFILE_FUNCTION();

	// Everything should have been used
	JPH_ASSERT(mContactAndConstraintsNextFree == mContactAndConstraintsSize);

	// Free contact and constraint buffers
	if (mContactAndConstraintsSize > 0)
	{
		inTempAllocator->Free(mContactAndConstraintIndices, mContactAndConstraintsSize * sizeof(uint32));
		mContactAndConstraintIndices = nullptr;

		inTempAllocator->Free(mContactAndConstaintsGroupIdx, mContactAndConstraintsSize * sizeof(uint32));
		mContactAndConstaintsGroupIdx = nullptr;
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
