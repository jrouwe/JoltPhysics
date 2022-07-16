// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Skeleton/SkeletonMapper.h>

JPH_NAMESPACE_BEGIN

void SkeletonMapper::Initialize(const Skeleton *inSkeleton1, const Mat44 *inNeutralPose1, const Skeleton *inSkeleton2, const Mat44 *inNeutralPose2, const CanMapJoint &inCanMapJoint)
{
	JPH_ASSERT(mMappings.empty() && mChains.empty() && mUnmapped.empty()); // Should not be initialized yet

	// Count joints
	int n1 = inSkeleton1->GetJointCount();
	int n2 = inSkeleton2->GetJointCount();
	JPH_ASSERT(n1 <= n2, "Skeleton 1 should be the low detail skeleton!");

	// Keep track of mapped joints (initialize to false)
	Array<bool> mapped1(n1, false);
	Array<bool> mapped2(n2, false);

	// Find joints that can be mapped directly
	for (int j1 = 0; j1 < n1; ++j1)
		for (int j2 = 0; j2 < n2; ++j2)
			if (inCanMapJoint(inSkeleton1, j1, inSkeleton2, j2))
			{
				mMappings.emplace_back(j1, j2, inNeutralPose1[j1].Inversed() * inNeutralPose2[j2]);
				mapped1[j1] = true;
				mapped2[j2] = true;
				break;
			}

	Array<int> cur_chain; // Taken out of the loop to minimize amount of allocations

	// Find joint chains
	for (int m1 = 0; m1 < (int)mMappings.size(); ++m1)
	{
		Array<int> chain2;
		int chain2_m = -1;

		for (int m2 = m1 + 1; m2 < (int)mMappings.size(); ++m2)
		{
			// Find the chain from back from m2 to m1
			int start = mMappings[m1].mJointIdx2;
			int end = mMappings[m2].mJointIdx2;
			int cur = end;
			cur_chain.clear(); // Should preserve memory
			do
			{
				cur_chain.push_back(cur);
				cur = inSkeleton2->GetJoint(cur).mParentJointIndex;
			}
			while (cur >= 0 && cur != start && !mapped2[cur]);
			cur_chain.push_back(start);

			if (cur == start // This should be the correct chain
				&& cur_chain.size() > 2 // It should have joints between the mapped joints
				&& cur_chain.size() > chain2.size()) // And it should be the longest so far
			{
				chain2.swap(cur_chain);
				chain2_m = m2;
			}
		}

		if (!chain2.empty())
		{
			// Get the chain for 1
			Array<int> chain1;
			int start = mMappings[m1].mJointIdx1;
			int cur = mMappings[chain2_m].mJointIdx1;
			do
			{
				chain1.push_back(cur);
				cur = inSkeleton1->GetJoint(cur).mParentJointIndex;
			}
			while (cur >= 0 && cur != start && !mapped1[cur]);
			chain1.push_back(start);

			// If the chain exists in 1 too
			if (cur == start)
			{		
				// Reverse the chains
				reverse(chain1.begin(), chain1.end());
				reverse(chain2.begin(), chain2.end());

				// Mark elements mapped
				for (int j1 : chain1)
					mapped1[j1] = true;
				for (int j2 : chain2)
					mapped2[j2] = true;

				// Insert the chain
				mChains.emplace_back(move(chain1), move(chain2));
			}
		}
	}

	// Collect unmapped joints from 2
	for (int j2 = 0; j2 < n2; ++j2)
		if (!mapped2[j2])
			mUnmapped.emplace_back(j2, inSkeleton2->GetJoint(j2).mParentJointIndex);
}

void SkeletonMapper::Map(const Mat44 *inPose1ModelSpace, const Mat44 *inPose2LocalSpace, Mat44 *outPose2ModelSpace) const
{
	// Apply direct mappings
	for (const Mapping &m : mMappings)
		outPose2ModelSpace[m.mJointIdx2] = inPose1ModelSpace[m.mJointIdx1] * m.mJoint1To2;

	// Apply chain mappings
	for (const Chain &c : mChains)
	{
		// Calculate end of chain given local space transforms of the joints of the chain
		Mat44 &chain_start = outPose2ModelSpace[c.mJointIndices2.front()];
		Mat44 chain_end = chain_start;
		for (int j = 1; j < (int)c.mJointIndices2.size(); ++j)
			chain_end = chain_end * inPose2LocalSpace[c.mJointIndices2[j]];

		// Calculate the direction in world space for skeleton 1 and skeleton 2 and the rotation between them
		Vec3 actual = chain_end.GetTranslation() - chain_start.GetTranslation();
		Vec3 desired = inPose1ModelSpace[c.mJointIndices1.back()].GetTranslation() - inPose1ModelSpace[c.mJointIndices1.front()].GetTranslation();
		Quat rotation = Quat::sFromTo(actual, desired);

		// Rotate the start of the chain
		chain_start.SetRotation(Mat44::sRotation(rotation) * chain_start.GetRotation());

		// Update all joints but the first and the last joint using their local space transforms
		for (int j = 1; j < (int)c.mJointIndices2.size() - 1; ++j)
		{
			int parent = c.mJointIndices2[j - 1];
			int child = c.mJointIndices2[j];
			outPose2ModelSpace[child] = outPose2ModelSpace[parent] * inPose2LocalSpace[child];
		}
	}

	// All unmapped joints take the local pose and convert it to model space
	for (const Unmapped &u : mUnmapped)
		if (u.mParentJointIdx >= 0)
		{
			JPH_ASSERT(u.mParentJointIdx < u.mJointIdx, "Joints must be ordered: parents first");
			outPose2ModelSpace[u.mJointIdx] = outPose2ModelSpace[u.mParentJointIdx] * inPose2LocalSpace[u.mJointIdx];
		}
		else
			outPose2ModelSpace[u.mJointIdx] = inPose2LocalSpace[u.mJointIdx];
}

void SkeletonMapper::MapReverse(const Mat44 *inPose2ModelSpace, Mat44 *outPose1ModelSpace) const
{
	// Normally each joint in skeleton 1 should be present in the mapping, so we only need to apply the direct mappings
	for (const Mapping &m : mMappings)
		outPose1ModelSpace[m.mJointIdx1] = inPose2ModelSpace[m.mJointIdx2] * m.mJoint2To1;
}

JPH_NAMESPACE_END
