// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include "Layers.h"

TEST_SUITE("CollisionGroupTests")
{
	TEST_CASE("TestCollisionGroup1")
	{
		// Test group filter with no sub groups
		Ref<GroupFilterTable> group_filter = new GroupFilterTable;

		// Check that doesn't collide with self
		CollisionGroup g1(group_filter, 0, 0);
		CHECK(!g1.CanCollide(g1));

		// Check that collides with other group
		CollisionGroup g2(group_filter, 1, 0);
		CHECK(g1.CanCollide(g2));
		CHECK(g2.CanCollide(g1));
	}

	TEST_CASE("TestCollisionGroup2")
	{
		// Test group filter with no sub groups
		Ref<GroupFilterTable> group_filter1 = new GroupFilterTable(10);
		Ref<GroupFilterTable> group_filter2 = new GroupFilterTable(10);

		// Disable some pairs
		using SubGroupPair = pair<CollisionGroup::SubGroupID, CollisionGroup::SubGroupID>;
		vector<SubGroupPair> pairs = { 
			SubGroupPair(CollisionGroup::SubGroupID(1), CollisionGroup::SubGroupID(2)), 
			SubGroupPair(CollisionGroup::SubGroupID(9), CollisionGroup::SubGroupID(5)), 
			SubGroupPair(CollisionGroup::SubGroupID(3), CollisionGroup::SubGroupID(7)), 
			SubGroupPair(CollisionGroup::SubGroupID(6), CollisionGroup::SubGroupID(1)), 
			SubGroupPair(CollisionGroup::SubGroupID(8), CollisionGroup::SubGroupID(1)) 
		};
		for (const SubGroupPair &p : pairs)
		{
			group_filter1->DisableCollision(p.first, p.second);
			group_filter2->DisableCollision(p.first, p.second);
		}

		for (CollisionGroup::SubGroupID i = 0; i < 10; ++i)
			for (CollisionGroup::SubGroupID j = 0; j < 10; ++j)
			{
				// Check that doesn't collide with self
				CollisionGroup g1(group_filter1, 0, i);
				CHECK(!g1.CanCollide(g1));

				// Same filter, same group, check if pairs collide
				CollisionGroup g2(group_filter1, 0, j);
				if (i == j 
					|| find(pairs.begin(), pairs.end(), SubGroupPair(i, j)) != pairs.end() 
					|| find(pairs.begin(), pairs.end(), SubGroupPair(j, i)) != pairs.end())
				{
					CHECK(!g1.CanCollide(g2));
					CHECK(!g2.CanCollide(g1));
				}
				else
				{
					CHECK(g1.CanCollide(g2));
					CHECK(g2.CanCollide(g1));
				}
				
				// Using different group always collides
				CollisionGroup g3(group_filter1, 1, j);
				CHECK(g1.CanCollide(g3));
				CHECK(g3.CanCollide(g1));

				// Using different filter with equal group should not collide
				CollisionGroup g4(group_filter2, 0, j);
				CHECK(!g1.CanCollide(g4));
				CHECK(!g4.CanCollide(g1));

				// Using different filter with non-equal group should collide
				CollisionGroup g5(group_filter2, 1, j);
				CHECK(g1.CanCollide(g5));
				CHECK(g5.CanCollide(g1));
			}
	}
}
