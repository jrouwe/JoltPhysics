// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

namespace JPH {

template <class Key, class Value>
void LockFreeHashMap<Key, Value>::Init(uint inObjectStoreSizeBytes, uint32 inMaxBuckets)
{
	JPH_ASSERT(inMaxBuckets >= 4 && IsPowerOf2(inMaxBuckets));
	JPH_ASSERT(mObjectStore == nullptr);
	JPH_ASSERT(mBuckets == nullptr);

	mObjectStoreSizeBytes = inObjectStoreSizeBytes;
	mNumBuckets = inMaxBuckets;
	mMaxBuckets = inMaxBuckets;

	mObjectStore = new uint8 [inObjectStoreSizeBytes];
	mBuckets = new atomic<uint32> [inMaxBuckets];

	Clear();
}

template <class Key, class Value>
LockFreeHashMap<Key, Value>::~LockFreeHashMap()
{
	delete [] mObjectStore;
	delete [] mBuckets;
}

template <class Key, class Value>
void LockFreeHashMap<Key, Value>::Clear()
{
	// Reset write offset and number of key value pairs
	mWriteOffset = 0;
	mNumKeyValues = 0;

	// Reset buckets 4 at a time
	static_assert(sizeof(atomic<uint32>) == sizeof(uint32));
	UVec4 invalid_handle = UVec4::sReplicate(cInvalidHandle);
	uint32 *start = reinterpret_cast<uint32 *>(mBuckets), *end = start + mNumBuckets;
	JPH_ASSERT(IsAligned(start, 16));
	while (start < end)
	{
		invalid_handle.StoreInt4Aligned(start);
		start += 4;
	}
}

template <class Key, class Value>
void LockFreeHashMap<Key, Value>::SetNumBuckets(uint32 inNumBuckets)
{
	JPH_ASSERT(mNumKeyValues == 0);
	JPH_ASSERT(inNumBuckets <= mMaxBuckets);
	JPH_ASSERT(inNumBuckets >= 4 && IsPowerOf2(inNumBuckets));

	mNumBuckets = inNumBuckets;	
}

template <class Key, class Value>
template <class... Params>
inline typename LockFreeHashMap<Key, Value>::KeyValue *LockFreeHashMap<Key, Value>::Create(const Key &inKey, size_t inKeyHash, int inExtraBytes, Params &&... inConstructorParams)
{
	// This is not a multi map, test the key hasn't been inserted yet
	JPH_ASSERT(Find(inKey, inKeyHash) == nullptr);

	// Calculate total size
	uint size = sizeof(KeyValue) + inExtraBytes;

	// Allocate entry in the cache
	uint32 write_offset = mWriteOffset.fetch_add(size);
	if (write_offset + size > mObjectStoreSizeBytes)
		return nullptr;
	++mNumKeyValues;

	// Construct the key/value pair
	KeyValue *kv = reinterpret_cast<KeyValue *>(mObjectStore + write_offset);
#ifdef _DEBUG
	memset(kv, 0xcd, size);
#endif
	kv->mKey = inKey;
	new (&kv->mValue) Value(forward<Params>(inConstructorParams)...);

	// Get the offset to the first object from the bucket with corresponding hash
	atomic<uint32> &offset = mBuckets[inKeyHash & (mNumBuckets - 1)];

	// Add this entry as the first element in the linked list
	uint32 new_offset = uint32(reinterpret_cast<uint8 *>(kv) - mObjectStore);
	for (;;)
	{
		uint32 old_offset = offset;
		kv->mNextOffset = old_offset;
		if (offset.compare_exchange_strong(old_offset, new_offset))
			break;
	}

	return kv;
}

template <class Key, class Value>
inline const typename LockFreeHashMap<Key, Value>::KeyValue *LockFreeHashMap<Key, Value>::Find(const Key &inKey, size_t inKeyHash) const
{
	// Get the offset to the keyvalue object from the bucket with corresponding hash
	uint32 offset = mBuckets[inKeyHash & (mNumBuckets - 1)];
	while (offset != cInvalidHandle)
	{
		// Loop through linked list of values until the right one is found
		const KeyValue *kv = reinterpret_cast<const KeyValue *>(mObjectStore + offset);
		if (kv->mKey == inKey)
			return kv;
		offset = kv->mNextOffset;
	}

	// Not found
	return nullptr;
}

template <class Key, class Value>
inline uint32 LockFreeHashMap<Key, Value>::ToHandle(const KeyValue *inKeyValue) const
{
	const uint8 *kv = reinterpret_cast<const uint8 *>(inKeyValue);
	JPH_ASSERT(kv >= mObjectStore && kv < mObjectStore + mObjectStoreSizeBytes);
	return uint32(kv - mObjectStore);
}

template <class Key, class Value>
inline const typename LockFreeHashMap<Key, Value>::KeyValue *LockFreeHashMap<Key, Value>::FromHandle(uint32 inHandle) const
{
	JPH_ASSERT(inHandle < mObjectStoreSizeBytes);
	return reinterpret_cast<const KeyValue *>(mObjectStore + inHandle);
}

template <class Key, class Value>
inline void LockFreeHashMap<Key, Value>::GetAllKeyValues(vector<const KeyValue *> &outAll) const
{
	for (atomic<uint32> *bucket = mBuckets; bucket < mBuckets + mNumBuckets; ++bucket)
	{
		uint32 offset = *bucket;
		while (offset != cInvalidHandle)
		{
			const KeyValue *kv = reinterpret_cast<const KeyValue *>(mObjectStore + offset);
			outAll.push_back(kv);
			offset = kv->mNextOffset;
		}
	}
}

template <class Key, class Value>
typename LockFreeHashMap<Key, Value>::Iterator LockFreeHashMap<Key, Value>::begin()
{
	// Start with the first bucket
	Iterator it { this, 0, mBuckets[0] };

	// If it doesn't contain a valid entry, use the ++ operator to find the first valid entry
	if (it.mOffset == cInvalidHandle)
		++it;

	return it;
}

template <class Key, class Value>
typename LockFreeHashMap<Key, Value>::Iterator LockFreeHashMap<Key, Value>::end()
{
	return { this, mNumBuckets, cInvalidHandle };
}

template <class Key, class Value>
typename LockFreeHashMap<Key, Value>::KeyValue &LockFreeHashMap<Key, Value>::Iterator::operator* ()
{
	JPH_ASSERT(mOffset != cInvalidHandle);

	return *reinterpret_cast<KeyValue *>(mMap->mObjectStore + mOffset);
}		

template <class Key, class Value>
typename LockFreeHashMap<Key, Value>::Iterator &LockFreeHashMap<Key, Value>::Iterator::operator++ ()
{
	JPH_ASSERT(mBucket < mMap->mNumBuckets);

	// Find the next key value in this bucket
	if (mOffset != cInvalidHandle)
	{
		const KeyValue *kv = reinterpret_cast<const KeyValue *>(mMap->mObjectStore + mOffset);
		mOffset = kv->mNextOffset;
		if (mOffset != cInvalidHandle)
			return *this;
	}

	// Loop over next buckets
	for (;;)
	{
		// Next bucket
		++mBucket;
		if (mBucket >= mMap->mNumBuckets)
			return *this;

		// Fetch the first entry in the bucket
		mOffset = mMap->mBuckets[mBucket];
		if (mOffset != cInvalidHandle)
			return *this;
	}
}

#ifdef _DEBUG

template <class Key, class Value>
void LockFreeHashMap<Key, Value>::TraceStats() const
{
	const int cMaxPerBucket = 256;

	int max_objects_per_bucket = 0;
	int num_objects = 0;
	int histogram[cMaxPerBucket];
	for (int i = 0; i < cMaxPerBucket; ++i)
		histogram[i] = 0;

	for (atomic<uint32> *bucket = mBuckets, *bucket_end = mBuckets + mNumBuckets; bucket < bucket_end; ++bucket)
	{
		int objects_in_bucket = 0;
		uint32 offset = *bucket;
		while (offset != cInvalidHandle)
		{
			const KeyValue *kv = reinterpret_cast<const KeyValue *>(mObjectStore + offset);
			offset = kv->mNextOffset;
			++objects_in_bucket;
			++num_objects;
		}
		max_objects_per_bucket = max(objects_in_bucket, max_objects_per_bucket);
		histogram[min(objects_in_bucket, cMaxPerBucket - 1)]++;
	}

	Trace("max_objects_per_bucket = %d, num_buckets = %d, num_objects = %d", max_objects_per_bucket, mNumBuckets, num_objects);
	
	for (int i = 0; i < cMaxPerBucket; ++i)
		if (histogram[i] != 0)
			Trace("%d: %d", i, histogram[i]);
}

#endif

} // JPH
