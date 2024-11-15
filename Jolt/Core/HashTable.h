// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// Helper class for implementing a HashSet or HashMap
/// Based on CppCon 2017: Matt Kulukundis "Designing a Fast, Efficient, Cache-friendly Hash Table, Step by Step"
/// See: https://www.youtube.com/watch?v=ncHmEUmJZf4
template <class Key, class KeyValue, class HashTableDetail, class Hash, class KeyEqual>
class HashTable
{
public:
	using value_type = KeyValue;
	using size_type = uint32;

private:
	/// Base class for iterators
	template <class Table, class Iterator>
	class IteratorBase
	{
	public:
		/// Copy constructor
							IteratorBase(const IteratorBase &inRHS) = default;

		/// Assignment operator
		IteratorBase &		operator = (const IteratorBase &inRHS) = default;

		/// Iterator at start of table
							IteratorBase(Table *inTable) :
			mTable(inTable),
			mIndex(0)
		{
			while (mIndex < mTable->mMaxSize && (mTable->mControl[mIndex] & cBucketUsed) == 0)
				++mIndex;
		}

		/// Iterator at specific index
							IteratorBase(Table *inTable, size_type inIndex) :
			mTable(inTable),
			mIndex(inIndex)
		{
		}

		/// Prefix increment
		Iterator &			operator ++ ()
		{
			JPH_ASSERT(IsValid());

			do
			{
				++mIndex;
			}
			while (mIndex < mTable->mMaxSize && (mTable->mControl[mIndex] & cBucketUsed) == 0);

			return static_cast<Iterator &>(*this);
		}

		/// Postfix increment
		Iterator			operator ++ (int)
		{
			JPH_ASSERT(IsValid());
			Iterator result(mTable, mIndex);
			++(*this);
			return result;
		}

		/// Access to key value pair
		const KeyValue &	operator * () const
		{
			JPH_ASSERT(IsValid());
			return mTable->mData[mIndex];
		}

		/// Access to key value pair
		const KeyValue *	operator -> () const
		{
			JPH_ASSERT(IsValid());
			return mTable->mData + mIndex;
		}

		/// Equality operator
		bool				operator == (const Iterator &inRHS) const
		{
			return mTable == inRHS.mTable && mIndex == inRHS.mIndex;
		}

		/// Inequality operator
		bool				operator != (const Iterator &inRHS) const
		{
			return !(*this == inRHS);
		}

		/// Check that the iterator is valid
		bool				IsValid() const
		{
			return mIndex < mTable->mMaxSize
				&& (mTable->mControl[mIndex] & cBucketUsed) != 0;
		}

		Table *				mTable;
		size_type			mIndex;
	};

public:
	/// Non-const iterator
	class iterator : public IteratorBase<HashTable, iterator>
	{
		using Base = IteratorBase<HashTable, iterator>;

	public:
		/// Constructors
							iterator(HashTable *inTable) : Base(inTable) { }
							iterator(HashTable *inTable, size_type inIndex) : Base(inTable, inIndex) { }
							iterator(const iterator &inIterator) : Base(inIterator) { }

		using Base::operator *;

		/// Non-const access to key value pair
		KeyValue &			operator * ()
		{
			JPH_ASSERT(this->IsValid());
			return this->mTable->mData[this->mIndex];
		}

		using Base::operator ->;

		/// Non-const access to key value pair
		KeyValue *			operator -> ()
		{
			JPH_ASSERT(this->IsValid());
			return this->mTable->mData + this->mIndex;
		}
	};

	/// Const iterator
	class const_iterator : public IteratorBase<const HashTable, const_iterator>
	{
		using Base = IteratorBase<const HashTable, const_iterator>;

	public:
		/// Constructors
							const_iterator(const HashTable *inTable) : Base(inTable) { }
							const_iterator(const HashTable *inTable, size_type inIndex) : Base(inTable, inIndex) { }
							const_iterator(const const_iterator &inRHS) : Base(inRHS) { }
							const_iterator(const iterator &inIterator) : Base(inIterator.mTable, inIterator.mIndex) { }
	};

	/// Default constructor
							HashTable() = default;

	/// Copy constructor
							HashTable(const HashTable &inRHS) = delete; // TODO
							HashTable &operator = (const HashTable &) = delete; // TODO

	/// Move constructor
							HashTable(HashTable &&ioRHS) :
		mData(ioRHS.mData),
		mControl(ioRHS.mControl),
		mSize(ioRHS.mSize),
		mMaxSize(ioRHS.mMaxSize)
	{
		ioRHS.mData = nullptr;
		ioRHS.mControl = nullptr;
		ioRHS.mSize = 0;
		ioRHS.mMaxSize = 0;
	}

	/// Destructor
							~HashTable()								{ clear(); }

	/// Reserve memory for a certain number of elements
	void					reserve(size_type inMaxSize)
	{
		JPH_ASSERT(mData == nullptr);

		// Calculate max size based on load factor
		mMaxSize = GetNextPowerOf2(max<uint32>(cMaxLoadFactorDenominator * inMaxSize / cMaxLoadFactorNumerator, 16));

		// Allocate memory
		size_type required_size = mMaxSize * (sizeof(KeyValue) + 1);
		if constexpr (cNeedsAlignedAllocate)
			mData = reinterpret_cast<KeyValue *>(AlignedAllocate(required_size, alignof(KeyValue)));
		else
			mData = reinterpret_cast<KeyValue *>(Allocate(required_size));

		// Reset all control bytes
		mControl = reinterpret_cast<uint8 *>(mData + mMaxSize);
		memset(mControl, cBucketEmpty, mMaxSize);
	}

	/// Destroy the entire hash table
	void					clear()
	{
		if (mData != nullptr)
		{
			// Delete all elements
			if constexpr (!is_trivially_destructible<KeyValue>())
				for (int i = 0; i < mMaxSize; ++i)
					if (mControl[i] & cBucketUsed)
						mData[i].~T();

			// Free memory
			if constexpr (cNeedsAlignedAllocate)
				AlignedFree(mData);
			else
				Free(mData);

			// Reset members
			mData = nullptr;
			mSize = 0;
			mMaxSize = 0;
		}
	}

	/// Iterator to first element
	iterator				begin()
	{
		return iterator(this);
	}

	/// Iterator to one beyond last element
	iterator				end()
	{
		return iterator(this, mMaxSize);
	}

	/// Iterator to first element
	const_iterator			begin() const
	{
		return const_iterator(this);
	}

	/// Iterator to one beyond last element
	const_iterator			end() const
	{
		return const_iterator(this, mMaxSize);
	}

	/// Iterator to first element
	const_iterator			cbegin() const
	{
		return const_iterator(this);
	}

	/// Iterator to one beyond last element
	const_iterator			cend() const
	{
		return const_iterator(this, mMaxSize);
	}

	/// Check if there are no elements in the table
	bool					empty() const
	{
		return mSize == 0;
	}

	/// Number of elements in the table
	size_type				size() const
	{
		return mSize;
	}

	/// Max number of elements that can be stored in the table
	size_type				max_size() const
	{
		return mMaxSize;
	}

	/// Insert a new element, returns iterator and if the element was inserted
	std::pair<iterator, bool> insert(const value_type &inValue)
	{
		// Calculate hash
		Hash hash;
		uint64 hash_value = hash(HashTableDetail::sGetKey(inValue)); // TODO: Ensure we have 64 bit hash

		// Split hash into control byte and index
		uint8 control = cBucketUsed | uint8(hash_value & 0x7f);
		size_type index = size_type(hash_value) & (mMaxSize - 1);

		// Linear probing
		KeyEqual equal;
		for (;;)
		{
			uint8 this_control = mControl[index];
			if (this_control == control && equal(HashTableDetail::sGetKey(mData[index]), HashTableDetail::sGetKey(inValue)))
			{
				// Element already exists
				return std::make_pair(iterator(this, index), false);
			}
			else if (this_control == cBucketEmpty || this_control == cBucketDeleted)
			{
				// Insert new element
				::new (mData + index) KeyValue(inValue);
				mControl[index] = control;
				++mSize;
				return std::make_pair(iterator(this, index), true);
			}

			// Move to next bucket
			index = (index + 1) & (mMaxSize - 1);
		}
	}

	/// Find an element, returns iterator to element or end() if not found
	const_iterator			find(const Key &inKey) const
	{
		// Calculate hash
		Hash hash;
		uint64 hash_value = hash(inKey); // TODO: Ensure we have 64 bit hash

		// Split hash into control byte and index
		uint8 control = cBucketUsed | uint8(hash_value & 0x7f);
		size_type index = size_type(hash_value) & (mMaxSize - 1);

		// Linear probing
		KeyEqual equal;
		for (;;)
		{
			uint8 this_control = mControl[index];
			if (this_control == control && equal(HashTableDetail::sGetKey(mData[index]), inKey))
			{
				// Element found
				return const_iterator(this, index);
			}
			else if (this_control == cBucketEmpty)
			{
				// Element not found
				return end();
			}

			// Move to next bucket
			index = (index + 1) & (mMaxSize - 1);
		}
	}

private:
	/// If this allocator needs to fall back to aligned allocations because the type requires it
	static constexpr bool	cNeedsAlignedAllocate = alignof(KeyValue) > (JPH_CPU_ADDRESS_BITS == 32? 8 : 16);

	/// Max load factor is cMaxLoadFactorNumerator / cMaxLoadFactorDenominator
	static constexpr uint64	cMaxLoadFactorNumerator = 8;
	static constexpr uint64	cMaxLoadFactorDenominator = 10;

	/// Values that the control bytes can have
	static constexpr uint8	cBucketEmpty = 0;
	static constexpr uint8	cBucketDeleted = 0x7f;
	static constexpr uint8	cBucketUsed = 0x80;	// Lowest 7 bits are lowest 7 bits of the hash value

	/// The buckets, an array of size mMaxSize
	KeyValue *				mData = nullptr;

	/// Control bytes
	uint8 *					mControl = nullptr;

	/// Number of elements in the table
	size_type				mSize = 0;

	/// Max number of elements that can be stored in the table
	size_type				mMaxSize = 0;
};

/// Internal helper class to provide context for HashMap
template <class Key, class Value>
class HashMapDetail
{
public:
	/// Get key from key value pair
	static const Key &		sGetKey(const std::pair<Key, Value> &inKeyValue)
	{
		return inKeyValue.first;
	}
};

/// Hash Map class
/// @tparam Key Key type
/// @tparam Value Value type
/// @tparam Hash Hash function (note should be 64-bits)
/// @tparam KeyEqual Equality comparison function
template <class Key, class Value, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
class HashMap : public HashTable<Key, std::pair<Key, Value>, HashMapDetail<Key, Value>, Hash, KeyEqual>
{
	using Base = HashTable<Key, std::pair<Key, Value>, HashMapDetail<Key, Value>, Hash, KeyEqual>;

public:
	Value &					operator [] (const Key &inKey)
	{
		std::pair<typename Base::iterator, bool> result = this->insert({ inKey, Value() });
		return result.first->second;
	}
};

/// Internal helper class to provide context for HashSet
template <class Key>
class HashSetDetail
{
public:
	/// The key is the key, just return it
	static const Key &		sGetKey(const Key &inKey)
	{
		return inKey;
	}
};

/// Hash Set class
/// @tparam Key Key type
/// @tparam Hash Hash function (note should be 64-bits)
/// @tparam KeyEqual Equality comparison function
template <class Key, class Hash = std::hash<Key>, class KeyEqual = std::equal_to<Key>>
class HashSet : public HashTable<Key, Key, HashSetDetail<Key>, Hash, KeyEqual>
{
};

JPH_NAMESPACE_END
