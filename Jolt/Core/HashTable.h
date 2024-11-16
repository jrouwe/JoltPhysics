// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Math/BVec16.h>

JPH_NAMESPACE_BEGIN

/// Helper class for implementing a HashSet or HashMap
/// Based on CppCon 2017: Matt Kulukundis "Designing a Fast, Efficient, Cache-friendly Hash Table, Step by Step"
/// See: https://www.youtube.com/watch?v=ncHmEUmJZf4
template <class Key, class KeyValue, class HashTableDetail, class Hash, class KeyEqual>
class HashTable
{
public:
	/// Properties
	using value_type = KeyValue;
	using size_type = uint32;
	using difference_type = ptrdiff_t;

private:
	/// Base class for iterators
	template <class Table, class Iterator>
	class IteratorBase
	{
	public:
        /// Properties
		using difference_type = typename Table::difference_type;
        using value_type = typename Table::value_type;
        using iterator_category = std::forward_iterator_tag;

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

	/// Allocate space for the hash table
	void					AllocateTable(size_type inMaxSize)
	{
		JPH_ASSERT(mData == nullptr);

		mMaxSize = inMaxSize;
		size_type required_size = mMaxSize * (sizeof(KeyValue) + 1) + 15; // Add 15 bytes to mirror the first 15 bytes of the control values
		if constexpr (cNeedsAlignedAllocate)
			mData = reinterpret_cast<KeyValue *>(AlignedAllocate(required_size, alignof(KeyValue)));
		else
			mData = reinterpret_cast<KeyValue *>(Allocate(required_size));
		mControl = reinterpret_cast<uint8 *>(mData + mMaxSize);
	}

	/// Copy the contents of another hash table
	void					CopyTable(const HashTable &inRHS)
	{
		if (inRHS.empty())
			return;

		AllocateTable(inRHS.mMaxSize);

		// Copy control bytes
		memcpy(mControl, inRHS.mControl, mMaxSize + 15);

		// Copy elements
		uint index = 0;
		for (uint8 *control = mControl, *control_end = mControl + mMaxSize; control != control_end; ++control, ++index)
			if (*control & cBucketUsed)
				::new (mData + index) KeyValue(inRHS.mData[index]);
		mSize = inRHS.mSize;
	}

protected:
	/// Get an element by index
	KeyValue &				GetElement(size_type inIndex) const
	{
		return mData[inIndex];
	}

	/// Insert a key into the map, returns true if the element was inserted, false if it already existed.
	/// outIndex is the index at which the element should be constructed / where it is located.
	bool					InsertKey(const Key &inKey, size_type &outIndex)
	{
		// Ensure we have enough space
		if (mData == nullptr)
			reserve(1);

		// Calculate hash
		Hash hash;
		uint64 hash_value = hash(inKey); // TODO: Ensure we have 64 bit hash

		// Split hash into control byte and index
		uint8 control = cBucketUsed | uint8(hash_value);
		size_type index = size_type(hash_value >> 7) & (mMaxSize - 1);

		BVec16 control16 = BVec16::sReplicate(control);
		BVec16 bucket_empty = BVec16::sZero();
		BVec16 bucket_deleted = BVec16::sReplicate(cBucketDeleted);

		// Keeps track of the index of the first deleted bucket we found
		constexpr size_type cNoDeleted = ~size_type(0);
		size_type first_deleted_index = cNoDeleted;

		// Linear probing
		KeyEqual equal;
		for (;;)
		{
			// Read 16 control values (note that we added 15 bytes at the end of the control values that mirror the first 15 bytes)
			BVec16 control_bytes = BVec16::sLoadByte16(mControl + index);

			// Check for the control value we're looking for
			uint32 control_equal = uint32(BVec16::sEquals(control_bytes, control16).GetTrues());

			// Check for empty buckets
			uint32 control_empty = uint32(BVec16::sEquals(control_bytes, bucket_empty).GetTrues());

			// Check if we're still scanning for deleted buckets
			if (first_deleted_index == cNoDeleted)
			{
				// Check if any buckets have been deleted, if so store the first one
				uint32 control_deleted = uint32(BVec16::sEquals(control_bytes, bucket_deleted).GetTrues());
				if (control_deleted != 0)
					first_deleted_index = index + CountTrailingZeros(control_deleted);
			}

			// Index within the 16 buckets
			size_type local_index = index;

			// Loop while there's still buckets to process
			while ((control_equal | control_empty) != 0)
			{
				// Get the index of the first bucket that is either equal or empty
				uint first_equal = CountTrailingZeros(control_equal);
				uint first_empty = CountTrailingZeros(control_empty);

				// Check if we first found a bucket with equal control value before an empty bucket
				if (first_equal < first_empty)
				{
					// Skip to the bucket
					local_index += first_equal;

					// We found a bucket with same control value
					if (equal(HashTableDetail::sGetKey(mData[local_index]), inKey))
					{
						// Element already exists
						outIndex = local_index;
						return false;
					}

					// Skip past this bucket
					local_index++;
					uint shift = first_equal + 1;
					control_equal >>= shift;
					control_empty >>= shift;
				}
				else
				{
					// An empty bucket was found, we can insert a new item
					JPH_ASSERT(control_empty != 0);

					// Get the location of the first empty or deleted bucket
					local_index += first_empty;
					if (first_deleted_index < local_index)
						local_index = first_deleted_index;

					// Update control byte
					mControl[local_index] = control;
					if (local_index < 15)
						mControl[mMaxSize + local_index] = control; // Mirror the first 15 bytes at the end of the control values
					++mSize;

					// Return index to newly allocated bucket
					outIndex = local_index;
					return true;
				}
			}

			// Move to next batch of 16 buckets
			index = (index + 16) & (mMaxSize - 1);
		}
	}

public:
	/// Non-const iterator
	class iterator : public IteratorBase<HashTable, iterator>
	{
		using Base = IteratorBase<HashTable, iterator>;

	public:
        /// Properties
        using reference = typename Base::value_type &;
        using pointer = typename Base::value_type *;

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
        /// Properties
        using reference = const typename Base::value_type &;
        using pointer = const typename Base::value_type *;

		/// Constructors
							const_iterator(const HashTable *inTable) : Base(inTable) { }
							const_iterator(const HashTable *inTable, size_type inIndex) : Base(inTable, inIndex) { }
							const_iterator(const const_iterator &inRHS) : Base(inRHS) { }
							const_iterator(const iterator &inIterator) : Base(inIterator.mTable, inIterator.mIndex) { }
	};

	/// Default constructor
							HashTable() = default;

	/// Copy constructor
							HashTable(const HashTable &inRHS)
	{
		CopyTable(inRHS);
	}

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

	/// Assignment operator
	HashTable &				operator = (const HashTable &inRHS)
	{
		if (this != &inRHS)
		{
			clear();

			CopyTable(inRHS);
		}

		return *this;
	}

	/// Destructor
							~HashTable()
	{
		clear();
	}

	/// Reserve memory for a certain number of elements
	void					reserve(size_type inMaxSize)
	{
		// Calculate max size based on load factor
		size_type max_size = GetNextPowerOf2(max<uint32>(cMaxLoadFactorDenominator * inMaxSize / cMaxLoadFactorNumerator, 16));

		// Allocate buffers
		AllocateTable(max_size);

		// Reset all control bytes
		memset(mControl, cBucketEmpty, mMaxSize + 15);
	}

	/// Destroy the entire hash table
	void					clear()
	{
		if (!empty())
		{
			// Delete all elements
			if constexpr (!is_trivially_destructible<KeyValue>())
				for (size_type i = 0; i < mMaxSize; ++i)
					if (mControl[i] & cBucketUsed)
						mData[i].~KeyValue();

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
		size_type index;
		bool inserted = InsertKey(HashTableDetail::sGetKey(inValue), index);
		if (inserted)
			::new (mData + index) KeyValue(inValue);
		return std::make_pair(iterator(this, index), inserted);
	}

	/// Find an element, returns iterator to element or end() if not found
	const_iterator			find(const Key &inKey) const
	{
		// Check if we have any data
		if (empty())
			return cend();

		// Calculate hash
		Hash hash;
		uint64 hash_value = hash(inKey); // TODO: Ensure we have 64 bit hash

		// Split hash into control byte and index
		uint8 control = cBucketUsed | uint8(hash_value);
		size_type index = size_type(hash_value >> 7) & (mMaxSize - 1);

		BVec16 control16 = BVec16::sReplicate(control);
		BVec16 bucket_empty = BVec16::sZero();

		// Linear probing
		KeyEqual equal;
		for (;;)
		{
			// Read 16 control values (note that we added 15 bytes at the end of the control values that mirror the first 15 bytes)
			BVec16 control_bytes = BVec16::sLoadByte16(mControl + index);

			// Check for the control value we're looking for
			uint32 control_equal = uint32(BVec16::sEquals(control_bytes, control16).GetTrues());

			// Check for empty buckets
			uint32 control_empty = uint32(BVec16::sEquals(control_bytes, bucket_empty).GetTrues());

			// Index within the 16 buckets
			size_type local_index = index;

			// Loop while there's still buckets to process
			while ((control_equal | control_empty) != 0)
			{
				// Get the index of the first bucket that is either equal or empty
				uint first_equal = CountTrailingZeros(control_equal);
				uint first_empty = CountTrailingZeros(control_empty);

				// Check if we first found a bucket with equal control value before an empty bucket
				if (first_equal < first_empty)
				{
					// Skip to the bucket
					local_index += first_equal;

					// We found a bucket with same control value
					if (equal(HashTableDetail::sGetKey(mData[local_index]), inKey))
					{
						// Element found
						return const_iterator(this, local_index);
					}

					// Skip past this bucket
					local_index++;
					uint shift = first_equal + 1;
					control_equal >>= shift;
					control_empty >>= shift;
				}
				else
				{
					// An empty bucket was found, we didn't find the element
					JPH_ASSERT(control_empty != 0);
					return cend();
				}
			}

			// Move to next bucket
			index = (index + 16) & (mMaxSize - 1);
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
	using size_type = typename Base::size_type;
	using iterator = typename Base::iterator;
	using const_iterator = typename Base::const_iterator;
	using value_type = typename Base::value_type;

	Value &					operator [] (const Key &inKey)
	{
		size_type index;
		bool inserted = this->InsertKey(inKey, index);
		value_type &key_value = this->GetElement(index);
		if (inserted)
			::new (&key_value) value_type(inKey, Value());
		return key_value.second;
	}

	template<class... Args>
	std::pair<iterator, bool> try_emplace(const Key &inKey, Args &&...inArgs)
	{
		size_type index;
		bool inserted = this->InsertKey(inKey, index);
		if (inserted)
			::new (&this->GetElement(index)) value_type(std::piecewise_construct, std::forward_as_tuple(inKey), std::forward_as_tuple(std::forward<Args>(inArgs)...));
		return std::make_pair(iterator(this, index), inserted);
	}

	template<class... Args>
	std::pair<iterator, bool> try_emplace(Key &&inKey, Args &&...inArgs)
	{
		size_type index;
		bool inserted = this->InsertKey(inKey, index);
		if (inserted)
			::new (&this->GetElement(index)) value_type(std::piecewise_construct, std::forward_as_tuple(std::move(inKey)), std::forward_as_tuple(std::forward<Args>(inArgs)...));
		return std::make_pair(iterator(this, index), inserted);
	}

	template<class K, class... Args>
	std::pair<iterator, bool> try_emplace(K &&inKey, Args &&...inArgs)
	{
		size_type index;
		bool inserted = this->InsertKey(inKey, index);
		if (inserted)
			::new (&this->GetElement(index)) value_type(std::piecewise_construct, std::forward_as_tuple(std::forward<K>(inKey)), std::forward_as_tuple(std::forward<Args>(inArgs)...));
		return std::make_pair(iterator(this, index), inserted);
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
