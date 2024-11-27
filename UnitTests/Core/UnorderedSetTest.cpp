// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"

#include <Jolt/Core/UnorderedSet.h>

TEST_SUITE("UnorderedSetTest")
{
	TEST_CASE("TestUnorderedSet")
	{
		UnorderedSet<int> set;
		CHECK(set.bucket_count() == 0);
		set.reserve(10);
		CHECK(set.bucket_count() == 16);

		// Check system limits
		CHECK(set.max_bucket_count() == 0x80000000);
		CHECK(set.max_size() == uint64(0x80000000) * 7 / 8);

		// Insert some entries
		CHECK(*set.insert(1).first == 1);
		CHECK(set.insert(3).second);
		CHECK(!set.insert(3).second);
		CHECK(set.size() == 2);
		CHECK(*set.find(1) == 1);
		CHECK(*set.find(3) == 3);
		CHECK(set.find(5) == set.cend());

		// Validate all elements are visited by a visitor
		int count = 0;
		bool visited[10] = { false };
		for (UnorderedSet<int>::const_iterator i = set.begin(); i != set.end(); ++i)
		{
			visited[*i] = true;
			++count;
		}
		CHECK(count == 2);
		CHECK(visited[1]);
		CHECK(visited[3]);
		for (UnorderedSet<int>::iterator i = set.begin(); i != set.end(); ++i)
		{
			visited[*i] = false;
			--count;
		}
		CHECK(count == 0);
		CHECK(!visited[1]);
		CHECK(!visited[3]);

		// Copy the set
		UnorderedSet<int> set2;
		set2 = set;
		CHECK(*set2.find(1) == 1);
		CHECK(*set2.find(3) == 3);
		CHECK(set2.find(5) == set2.cend());

		// Swap
		UnorderedSet<int> set3;
		set3.swap(set);
		CHECK(*set3.find(1) == 1);
		CHECK(*set3.find(3) == 3);
		CHECK(set3.find(5) == set3.end());
		CHECK(set.empty());

		// Move construct
		UnorderedSet<int> set4(std::move(set3));
		CHECK(*set4.find(1) == 1);
		CHECK(*set4.find(3) == 3);
		CHECK(set4.find(5) == set4.end());
		CHECK(set3.empty());

		// Move assign
		UnorderedSet<int> set5;
		set5.insert(999);
		CHECK(*set5.find(999) == 999);
		set5 = std::move(set4);
		CHECK(set5.find(999) == set5.end());
		CHECK(*set5.find(1) == 1);
		CHECK(*set5.find(3) == 3);
		CHECK(set4.empty());
	}

	TEST_CASE("TestUnorderedSetGrow")
	{
		UnorderedSet<int> set;
		for (int i = 0; i < 10000; ++i)
			CHECK(set.insert(i).second);

		CHECK(set.size() == 10000);

		for (int i = 0; i < 10000; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(10001) == set.cend());

		for (int i = 0; i < 5000; ++i)
			CHECK(set.erase(i) == 1);

		CHECK(set.size() == 5000);

		for (int i = 0; i < 5000; ++i)
			CHECK(set.find(i) == set.end());

		for (int i = 5000; i < 10000; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(10001) == set.cend());

		for (int i = 0; i < 5000; ++i)
			CHECK(set.insert(i).second);

		CHECK(!set.insert(0).second);

		CHECK(set.size() == 10000);

		for (int i = 0; i < 10000; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(10001) == set.cend());
	}

	TEST_CASE("TestUnorderedSetHashCollision")
	{
		// A hash function that's guaranteed to collide
		class MyBadHash
		{
		public:
			size_t operator () (int inValue) const
			{
				return 0;
			}
		};

		UnorderedSet<int, MyBadHash> set;
		for (int i = 0; i < 10; ++i)
			CHECK(set.insert(i).second);

		CHECK(set.size() == 10);

		for (int i = 0; i < 10; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(11) == set.cend());

		for (int i = 0; i < 5; ++i)
			CHECK(set.erase(i) == 1);

		CHECK(set.size() == 5);

		for (int i = 0; i < 5; ++i)
			CHECK(set.find(i) == set.end());

		for (int i = 5; i < 10; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(11) == set.cend());

		for (int i = 0; i < 5; ++i)
			CHECK(set.insert(i).second);

		CHECK(!set.insert(0).second);

		CHECK(set.size() == 10);

		for (int i = 0; i < 10; ++i)
			CHECK(*set.find(i) == i);

		CHECK(set.find(11) == set.cend());
	}

	TEST_CASE("TestUnorderedSetAddRemoveCyles")
	{
		UnorderedSet<int> set;
		constexpr int cBucketCount = 64;
		set.reserve(int(set.max_load_factor() * cBucketCount));
		CHECK(set.bucket_count() == cBucketCount);

		// Repeatedly add and remove elements to see if the set cleans up tombstones
		constexpr int cNumElements = 64 * 6 / 8; // We make sure that the map is max 6/8 full to ensure that we never grow the map but rehash it instead
		int add_counter = 0;
		int remove_counter = 0;
		for (int i = 0; i < 100; ++i)
		{
			for (int j = 0; j < cNumElements; ++j)
			{
				CHECK(set.find(add_counter) == set.end());
				CHECK(set.insert(add_counter).second);
				CHECK(set.find(add_counter) != set.end());
				++add_counter;
			}

			CHECK(set.size() == cNumElements);

			for (int j = 0; j < cNumElements; ++j)
			{
				CHECK(set.find(remove_counter) != set.end());
				CHECK(set.erase(remove_counter) == 1);
				CHECK(set.erase(remove_counter) == 0);
				CHECK(set.find(remove_counter) == set.end());
				++remove_counter;
			}

			CHECK(set.size() == 0);
			CHECK(set.empty());
		}

		// Test that adding and removing didn't resize the set
		CHECK(set.bucket_count() == cBucketCount);
	}

	TEST_CASE("TestUnorderedSetManyTombStones")
	{
		// A hash function that makes sure that consecutive ints end up in consecutive buckets starting at bucket 63
		class MyBadHash
		{
		public:
			size_t operator () (int inValue) const
			{
				return (inValue + 63) << 7;
			}
		};

		UnorderedSet<int, MyBadHash> set;
		constexpr int cBucketCount = 64;
		set.reserve(int(set.max_load_factor() * cBucketCount));
		CHECK(set.bucket_count() == cBucketCount);

		// Fill 32 buckets
		int add_counter = 0;
		for (int i = 0; i < 32; ++i)
			CHECK(set.insert(add_counter++).second);

		// Since we control the hash, we know in which order we'll visit the elements
		// The first element was inserted in bucket 63, so we start at 1
		int expected = 1;
		for (int i : set)
		{
			CHECK(i == expected);
			expected = (expected + 1) & 31;
		}
		expected = 1;
		for (int i : set)
		{
			CHECK(i == expected);
			expected = (expected + 1) & 31;
		}

		// Remove a bucket in the middle with so that the number of occupied slots
		// surrounding the bucket exceed 16 to force creating a tombstone,
		// then add one at the end
		int remove_counter = 16;
		for (int i = 0; i < 100; ++i)
		{
			CHECK(set.find(remove_counter) != set.end());
			CHECK(set.erase(remove_counter) == 1);
			CHECK(set.find(remove_counter) == set.end());

			CHECK(set.find(add_counter) == set.end());
			CHECK(set.insert(add_counter).second);
			CHECK(set.find(add_counter) != set.end());

			++add_counter;
			++remove_counter;
		}

		// Check that the elements we inserted are still there
		CHECK(set.size() == 32);
		for (int i = 0; i < 16; ++i)
			CHECK(*set.find(i) == i);
		for (int i = 0; i < 16; ++i)
			CHECK(*set.find(add_counter - 1 - i) == add_counter - 1 - i);

		// Test that adding and removing didn't resize the set
		CHECK(set.bucket_count() == cBucketCount);
	}

	static bool sReversedHash = false;

	TEST_CASE("TestUnorderedSetRehash")
	{
		// A hash function for which we can switch the hashing algorithm
		class MyBadHash
		{
		public:
			size_t operator () (int inValue) const
			{
				return (sReversedHash? 127 - inValue : inValue) << 7;
			}
		};

		using Set = UnorderedSet<int, MyBadHash>;
		Set set;
		constexpr int cBucketCount = 128;
		set.reserve(int(set.max_load_factor() * cBucketCount));
		CHECK(set.bucket_count() == cBucketCount);

		// Fill buckets
		sReversedHash = false;
		constexpr int cNumElements = 96;
		for (int i = 0; i < cNumElements; ++i)
			CHECK(set.insert(i).second);

		// Check that we get the elements in the expected order
		int expected = 0;
		for (int i : set)
			CHECK(i == expected++);

		// Change the hashing algorithm so that a rehash is forced to move elements.
		// The test is designed in such a way that it will both need to move elements to empty slots
		// and to move elements to slots that currently already have another element.
		sReversedHash = true;
		set.rehash(0);

		// Check that all elements are still there
		for (int i = 0; i < cNumElements; ++i)
			CHECK(*set.find(i) == i);

		// The hash went from filling buckets 0 .. 95 with values 0 .. 95 to bucket 127 .. 31 with values 0 .. 95
		// However, we don't move elements if they still fall within the same batch, this means that the first 8
		// elements didn't move
		Set::const_iterator it = set.begin();
		for (int i = 0; i < 8; ++i, ++it)
			CHECK(*it == i);

		// The rest will have been reversed
		for (int i = 95; i > 7; --i, ++it)
			CHECK(*it == i);

		// Test that adding and removing didn't resize the set
		CHECK(set.bucket_count() == cBucketCount);
	}
}
