// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <unordered_map>
#include <assert.h>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

template <typename Key, typename Value, typename HashFunc> class HashMap;


template <typename Key, typename Value, typename HashFunc>
class HashMapIterator
{
public:
	HashMapIterator(HashMap<Key, Value, HashFunc>* map_, std::pair<Key, Value>* bucket_)
		: map(map_), bucket(bucket_) {}


	bool operator == (const HashMapIterator& other) const
	{
		return bucket == other.bucket;
	}

	bool operator != (const HashMapIterator& other) const
	{
		return bucket != other.bucket;
	}

	void operator ++ ()
	{
		// Advance to next non-empty bucket
		do
		{
			bucket++;
		}
		while((bucket < (map->buckets + map->buckets_size)) && (bucket->first == map->empty_key));
	}

	std::pair<Key, Value>& operator * ()
	{
		return *bucket;
	}
	const std::pair<Key, Value>& operator * () const
	{
		return *bucket;
	}

	std::pair<Key, Value>* operator -> ()
	{
		return bucket;
	}
	const std::pair<Key, Value>* operator -> () const
	{
		return bucket;
	}

	HashMap<Key, Value, HashFunc>* map;
	std::pair<Key, Value>* bucket;
};


template <typename Key, typename Value, typename HashFunc>
class ConstHashMapIterator
{
public:
	ConstHashMapIterator(const HashMap<Key, Value, HashFunc>* map_, const std::pair<Key, Value>* bucket_)
		: map(map_), bucket(bucket_) {}

	ConstHashMapIterator(const HashMapIterator<Key, Value, HashFunc>& it)
		: map(it.map), bucket(it.bucket) {}


	bool operator == (const ConstHashMapIterator& other) const
	{
		return bucket == other.bucket;
	}

	bool operator != (const ConstHashMapIterator& other) const
	{
		return bucket != other.bucket;
	}

	void operator ++ ()
	{
		// Advance to next non-empty bucket
		do
		{
			bucket++;
		}
		while((bucket < (map->buckets + map->buckets_size)) && (bucket->first == map->empty_key));
	}

	const std::pair<Key, Value>& operator * () const
	{
		return *bucket;
	}

	const std::pair<Key, Value>* operator -> () const
	{
		return bucket;
	}

	const HashMap<Key, Value, HashFunc>* map;
	const std::pair<Key, Value>* bucket;
};


/*=====================================================================
HashMap
-------
A map class using a hash table.
This class requires passing an 'empty key' to the constructor.
This is a sentinel value that is never inserted in the map, and marks empty buckets.
=====================================================================*/
template <typename Key, typename Value, typename HashFunc = std::hash<Key>>
class HashMap
{
public:

	typedef HashMapIterator<Key, Value, HashFunc> iterator;
	typedef ConstHashMapIterator<Key, Value, HashFunc> const_iterator;
	typedef std::pair<Key, Value> KeyValuePair;

	typedef Key key_type;
	typedef std::pair<Key, Value> value_type;


	HashMap(Key empty_key_)
	:	buckets((std::pair<Key, Value>*)JPH::AlignedAllocate(sizeof(std::pair<Key, Value>) * 32, 64)), buckets_size(32), num_items(0), hash_mask(31), empty_key(empty_key_)
	{
		// Initialise elements
		std::pair<Key, Value> empty_key_val(empty_key, Value());
		for(std::pair<Key, Value>* elem=buckets; elem<buckets + buckets_size; ++elem)
			::new (elem) std::pair<Key, Value>(empty_key_val);
	}

	size_t myMax(size_t x, size_t y) { return x > y ? x : y; }

	// Adapted from http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
	// Not correct for 0 input: returns 0 in this case.
	inline uint64 roundToNextHighestPowerOf2(uint64 v)
	{
		v--;
		v |= v >> 1;
		v |= v >> 2;
		v |= v >> 4;
		v |= v >> 8;
		v |= v >> 16;
		v |= v >> 32;
		return v + 1;
	}

	HashMap(Key empty_key_, size_t expected_num_items)
	:	num_items(0), empty_key(empty_key_)
	{
		buckets_size = myMax(32ULL, roundToNextHighestPowerOf2(expected_num_items*2));
		
		buckets = (std::pair<Key, Value>*)JPH::AlignedAllocate(sizeof(std::pair<Key, Value>) * buckets_size, 64);

		// Initialise elements
		if(std::is_pod<Key>::value && std::is_pod<Value>::value)
		{
			std::pair<Key, Value>* const buckets_ = buckets; // Having this as a local var gives better codegen in VS.
			for(size_t z=0; z<buckets_size; ++z)
				buckets_[z].first = empty_key_;
		}
		else
		{
			std::pair<Key, Value> empty_key_val(empty_key, Value());
			for(std::pair<Key, Value>* elem=buckets; elem<buckets + buckets_size; ++elem)
				::new (elem) std::pair<Key, Value>(empty_key_val);
		}

		hash_mask = buckets_size - 1;
	}

	HashMap(const HashMap& other)
	{
		buckets_size = other.buckets_size;
		hash_func = other.hash_func;
		empty_key = other.empty_key;
		num_items = other.num_items;
		hash_mask = other.hash_mask;

		buckets = (std::pair<Key, Value>*)JPH::AlignedAllocate(sizeof(std::pair<Key, Value>) * buckets_size, 64);

		// Initialise elements
		for(size_t i=0; i<buckets_size; ++i)
			::new (&buckets[i]) std::pair<Key, Value>(other.buckets[i]);
	}

	~HashMap()
	{
		// Destroy objects
		for(size_t i=0; i<buckets_size; ++i)
			(buckets + i)->~KeyValuePair();

		JPH::AlignedFree(buckets);
	}


	iterator begin()
	{
		// Find first bucket with a value in it.
		for(size_t i=0; i<buckets_size; ++i)
			if(!(buckets[i].first == empty_key))
				return iterator(this, buckets + i);

		return iterator(this, buckets + buckets_size);
	}

	const_iterator begin() const
	{
		// Find first bucket with a value in it.
		for(size_t i=0; i<buckets_size; ++i)
			if(buckets[i].first != empty_key)
				return const_iterator(this, buckets + i);

		return const_iterator(this, buckets + buckets_size);
	}

	iterator end() { return HashMapIterator<Key, Value, HashFunc>(this, buckets + buckets_size); }
	const_iterator end() const { return ConstHashMapIterator<Key, Value, HashFunc>(this, buckets + buckets_size); }


	iterator find(const Key& k)
	{
		size_t bucket_i = hashKey(k);

		// Search for bucket item is in
		while(1)
		{
			if(buckets[bucket_i].first == k)
				return HashMapIterator<Key, Value, HashFunc>(this, &buckets[bucket_i]); // Found it

			if(buckets[bucket_i].first == empty_key)
				return end(); // No such key in map.

			// Else advance to next bucket, with wrap-around
			bucket_i = (bucket_i + 1) & hash_mask; // (bucket_i + 1) % buckets.size();
		}
	}

	const_iterator find(const Key& k) const
	{
		size_t bucket_i = hashKey(k);

		// Search for bucket item is in
		while(1)
		{
			if(buckets[bucket_i].first == k)
				return ConstHashMapIterator<Key, Value, HashFunc>(this, &buckets[bucket_i]); // Found it

			if(buckets[bucket_i].first == empty_key)
				return end(); // No such key in map.

			// Else advance to next bucket, with wrap-around
			bucket_i = (bucket_i + 1) & hash_mask; // (bucket_i + 1) % buckets.size();
		}
		return end();
	}

	// NOTE: not sure if this code is correct.
	template <class... Args>
	std::pair<iterator, bool>
	try_emplace(const key_type& k, Args&&... args)
	{
		auto it = find(k);
		if(it == end())
		{
			return insert(
				value_type(std::piecewise_construct,
				std::forward_as_tuple(k),
				std::forward_as_tuple(std::forward<Args>(args)...))
			);
		}
		else
			return {it, false};
	}


	// The basic idea here is instead of marking bucket i empty immediately, we will scan right, looking for objects that can be moved left to fill the empty slot.
	// See https://en.wikipedia.org/wiki/Open_addressing and https://en.wikipedia.org/w/index.php?title=Hash_table&oldid=95275577
	// This is also pretty much the same algorithm as 'Algorithm R (Deletion with linear probing)' in Section 6.4 of The Art of Computer Programming, Volume 3.
	void erase(const Key& key)
	{
		// Search for bucket item is in, or until we get to an empty bucket, which indicates the key is not in the map.
		// Bucket i is the bucket we will finally mark as empty.
		size_t i = hashKey(key);
		while(1)
		{
			if(buckets[i].first == empty_key)
				return; // No such key in map.

			if(buckets[i].first == key)
				break;

			// Else advance to next bucket, with wrap-around
			i = (i + 1) & hash_mask;
		}

		assert(buckets[i].first == key);

		size_t j = i; // j = current probe index to right of i, i = the current slot we will make empty
		while(1)
		{
			j = (j + 1) & hash_mask;
			if(buckets[j].first == empty_key)
				break;
			/*
			We are considering whether the item at location j can be moved to location i.
			This is allowed if the natural hash location k of the item is <= i, before modulo.

			Two cases to handle here: case where j does not wrap relative to i (j > i):
			Then acceptable ranges for k (natural hash location of j), such that j can be moved to location i:
			basically k has to be <= i before the modulo.
			
			-------------------------------------
			|   |   |   | a | b | c |   |   |   |
			-------------------------------------
			              i       j
			----------------|       |-----------
			  k <= i                     k > j

			Case where j does wrap relative to i (j < i):
			Then acceptable ranges for k (natural hash location of j), such that j can be moved to location i:
			basically k has to be <= i before the modulo.

			-------------------------------------
			| c | d |   |   |   |   |   | a | b |
			-------------------------------------
			      j                       i
			        |-----------------------|
			          k > j && k <= i

			Note that the natural hash location of an item at j is always <= j (before modulo)
			*/
			const size_t k = hashKey(buckets[j].first); // k = natural hash location of item in bucket j.
			if((j > i) ? (k <= i || k > j) : (k <= i && k > j))
			{
				buckets[i] = buckets[j];
				buckets[j].second = Value();
				i = j;
			}
		}

		buckets[i].first = empty_key;
		buckets[i].second = Value();

		num_items--;
	}

	size_t count(const Key& k) const
	{
		return (find(k) == end()) ? 0 : 1;
	}


	// If key was already in map, returns iterator to existing item and false.
	// If key was not already in map, inserts it, then returns iterator to new item and true.
	std::pair<iterator, bool> insert(const std::pair<Key, Value>& key_val_pair)
	{
		assert(!(key_val_pair.first == empty_key));

		iterator find_res = find(key_val_pair.first);
		if(find_res == end())
		{
			// Item is not already inserted, insert:

			num_items++;
			checkForExpand();

			size_t bucket_i = hashKey(key_val_pair.first);
			// Search for bucket item is in, or an empty bucket
			while(1)
			{
				if(buckets[bucket_i].first == empty_key) // If bucket is empty:
				{
					buckets[bucket_i] = key_val_pair;
					return std::make_pair(HashMapIterator<Key, Value, HashFunc>(this, buckets/*.begin()*/ + bucket_i), /*inserted=*/true);
				}

				// Else advance to next bucket, with wrap-around
				bucket_i = (bucket_i + 1) & hash_mask; // bucket_i = (bucket_i + 1) % buckets.size();
			}
		}
		else
		{
			// Item was already in map: return (iterator to existing item, false)
			return std::make_pair(find_res, /*inserted=*/false);
		}
	}

	Value& operator [] (const Key& k)
	{
		iterator i = find(k);
		if(i == end())
		{
			std::pair<iterator, bool> result = insert(std::make_pair(k, Value()));
			return result.first->second;
		}
		else
		{
			return i->second;
		}
	}


	void clear()
	{
		for(size_t i=0; i<buckets_size; ++i)
		{
			buckets[i].first = empty_key;

			if(!std::is_pod<Value>::value)
				buckets[i].second = Value();
		}
		num_items = 0;
	}

	void invariant()
	{
		for(size_t i=0; i<buckets_size; ++i)
		{
			const Key key = buckets[i].first;
			if(key != empty_key)
			{
				const size_t k = hashKey(key);

				for(size_t z=k; z != i; z = (z + 1) & hash_mask)
				{
					assert(buckets[z].first != empty_key);
				}
			}
		}
	}

	bool empty() const { return num_items == 0; }

	size_t size() const { return num_items; }

private:
	size_t hashKey(const Key& k) const
	{
		//return hash_func(k) % buckets.size();
		return hash_func(k) & hash_mask;
	}

	// Returns true if expanded
	bool checkForExpand()
	{
		//size_t load_factor = num_items / buckets.size();
		//const float load_factor = (float)num_items / buckets.size();

		//if(load_factor > 0.5f)
		if(num_items >= buckets_size / 2)
		{
			expand(/*buckets.size() * 2*/);
			return true;
		}
		else
		{
			return false;
		}
	}


	void expand(/*size_t new_num_buckets*/)
	{
		// Get pointer to old buckets
		const std::pair<Key, Value>* const old_buckets = this->buckets;
		const size_t old_buckets_size = this->buckets_size;

		// Allocate new buckets
		this->buckets_size = old_buckets_size * 2;
		this->buckets = (std::pair<Key, Value>*)JPH::AlignedAllocate(sizeof(std::pair<Key, Value>) * this->buckets_size, 64);
		
		// Initialise elements
		if(std::is_pod<Key>::value && std::is_pod<Value>::value)
		{
			const Key empty_key_ = empty_key; // Having this as a local var gives better codegen in VS.
			std::pair<Key, Value>* const buckets_ = buckets; // Having this as a local var gives better codegen in VS.
			for(size_t z=0; z<buckets_size; ++z)
				buckets_[z].first = empty_key_;
		}
		else
		{
			// Initialise elements
			std::pair<Key, Value> empty_key_val(empty_key, Value());
			if(buckets)
				for(size_t z=0; z<buckets_size; ++z)
					::new (buckets + z) std::pair<Key, Value>(empty_key_val);
		}

		hash_mask = this->buckets_size - 1;

		// Insert items into new buckets
		
		for(size_t b=0; b<old_buckets_size; ++b) // For each old bucket
		{
			if(!(old_buckets[b].first == empty_key)) // If there is an item in the old bucket:
			{
				size_t bucket_i = hashKey(old_buckets[b].first); // Hash key of item to get new bucket index:

				// Search for an empty bucket
				while(1)
				{
					if(buckets[bucket_i].first == empty_key) // If bucket is empty:
					{
						buckets[bucket_i] = old_buckets[b]; // Write item to bucket.
						break;
					}

					// Else advance to next bucket, with wrap-around
					bucket_i = (bucket_i + 1) & hash_mask; // bucket_i = (bucket_i + 1) % buckets.size();
				}
			}
		}

		// Destroy old bucket data
		for(size_t i=0; i<old_buckets_size; ++i)
			(old_buckets + i)->~KeyValuePair();
		JPH::AlignedFree((std::pair<Key, Value>*)old_buckets);
	}

public:
	std::pair<Key, Value>* buckets; // Elements
	size_t buckets_size;
	HashFunc hash_func;
	Key empty_key;
private:
	size_t num_items;
	size_t hash_mask;
};



template <class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>> using UnorderedMap = HashMap<Key, T, Hash>;

//template <class Key, class T, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>> using UnorderedMap = std::unordered_map<Key, T, Hash, KeyEqual, STLAllocator<pair<const Key, T>>>;

JPH_NAMESPACE_END
