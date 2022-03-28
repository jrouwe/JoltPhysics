// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

JPH_NAMESPACE_BEGIN

template <typename Object>
FixedSizeFreeList<Object>::~FixedSizeFreeList()
{
	// Ensure everything is freed before the freelist is destructed
	JPH_ASSERT(mNumFreeObjects == mNumPages * mPageSize);

	// Free memory for pages
	uint32 num_pages = mNumObjectsAllocated / mPageSize;
	for (uint32 page = 0; page < num_pages; ++page)
		AlignedFree(mPages[page]);
	delete [] mPages;
}

template <typename Object>
void FixedSizeFreeList<Object>::Init(uint inMaxObjects, uint inPageSize)
{
	// Check sanity
	JPH_ASSERT(inPageSize > 0 && IsPowerOf2(inPageSize));
	JPH_ASSERT(mPages == nullptr);

	// Store configuration parameters
	mNumPages = (inMaxObjects + inPageSize - 1) / inPageSize;
	mPageSize = inPageSize;
	mPageShift = CountTrailingZeros(inPageSize);
	mObjectMask = inPageSize - 1;
	JPH_IF_ENABLE_ASSERTS(mNumFreeObjects = mNumPages * inPageSize;)

	// Allocate page table
	mPages = new ObjectStorage * [mNumPages];

	// We didn't yet use any objects of any page
	mNumObjectsAllocated = 0;
	mFirstFreeObjectInNewPage = 0;

	// Start with 1 as the first tag
	mAllocationTag = 1;

	// Set first free object (with tag 0)
	mFirstFreeObjectAndTag = cInvalidObjectIndex;
}

template <typename Object>
template <typename... Parameters>
uint32 FixedSizeFreeList<Object>::ConstructObject(Parameters &&... inParameters)
{
	for (;;)
	{
		// Get first object from the linked list
		uint64 first_free_object_and_tag = mFirstFreeObjectAndTag;
		uint32 first_free = uint32(first_free_object_and_tag);
		if (first_free == cInvalidObjectIndex)
		{
			// The free list is empty, we take an object from the page that has never been used before
			first_free = mFirstFreeObjectInNewPage++;
			if (first_free >= mNumObjectsAllocated)
			{
				// Allocate new page
				lock_guard lock(mPageMutex);
				while (first_free >= mNumObjectsAllocated)
				{
					uint32 next_page = mNumObjectsAllocated / mPageSize;
					if (next_page == mNumPages)
						return cInvalidObjectIndex; // Out of space!
					mPages[next_page] = reinterpret_cast<ObjectStorage *>(AlignedAlloc(mPageSize * sizeof(ObjectStorage), JPH_CACHE_LINE_SIZE));
					mNumObjectsAllocated += mPageSize;
				}
			}

			// Allocation successful
			JPH_IF_ENABLE_ASSERTS(--mNumFreeObjects;)
			ObjectStorage &storage = GetStorage(first_free);
			new (&storage.mData) Object(forward<Parameters>(inParameters)...);
			storage.mNextFreeObject = first_free;
			return first_free;
		}
		else
		{
			// Load next pointer
			uint32 new_first_free = GetStorage(first_free).mNextFreeObject;

			// Construct a new first free object tag
			uint64 new_first_free_object_and_tag = uint64(new_first_free) + (uint64(mAllocationTag++) << 32);

			// Compare and swap
			if (mFirstFreeObjectAndTag.compare_exchange_strong(first_free_object_and_tag, new_first_free_object_and_tag))
			{
				// Allocation successful
				JPH_IF_ENABLE_ASSERTS(--mNumFreeObjects;)
				ObjectStorage &storage = GetStorage(first_free);
				new (&storage.mData) Object(forward<Parameters>(inParameters)...);
				storage.mNextFreeObject = first_free;
				return first_free;
			}
		}
	}
}

template <typename Object>
void FixedSizeFreeList<Object>::AddObjectToBatch(Batch &ioBatch, uint32 inObjectIndex)
{
	JPH_ASSERT(GetStorage(inObjectIndex).mNextFreeObject == inObjectIndex, "Trying to add a object to the batch that is already in a free list");
	JPH_ASSERT(ioBatch.mNumObjects != uint32(-1), "Trying to reuse a batch that has already been freed");

	// Link object in batch to free
	if (ioBatch.mFirstObjectIndex == cInvalidObjectIndex)
		ioBatch.mFirstObjectIndex = inObjectIndex;
	else
		GetStorage(ioBatch.mLastObjectIndex).mNextFreeObject = inObjectIndex;
	ioBatch.mLastObjectIndex = inObjectIndex;
	ioBatch.mNumObjects++;
}

template <typename Object>
void FixedSizeFreeList<Object>::DestructObjectBatch(Batch &ioBatch)
{
	if (ioBatch.mFirstObjectIndex != cInvalidObjectIndex)
	{
		// Call destructors
		if (!is_trivially_destructible<Object>())
		{
			uint32 object_idx = ioBatch.mFirstObjectIndex;
			do
			{
				ObjectStorage &storage = GetStorage(object_idx);
				reinterpret_cast<Object &>(storage.mData).~Object();
				object_idx = storage.mNextFreeObject;
			}
			while (object_idx != cInvalidObjectIndex);
		}

		// Add to objects free list
		for (;;)
		{
			// Get first object from the list
			uint64 first_free_object_and_tag = mFirstFreeObjectAndTag;
			uint32 first_free = uint32(first_free_object_and_tag);

			// Make it the next pointer of the last object in the batch that is to be freed
			GetStorage(ioBatch.mLastObjectIndex).mNextFreeObject = first_free;

			// Construct a new first free object tag
			uint64 new_first_free_object_and_tag = uint64(ioBatch.mFirstObjectIndex) + (uint64(mAllocationTag++) << 32);

			// Compare and swap
			if (mFirstFreeObjectAndTag.compare_exchange_strong(first_free_object_and_tag, new_first_free_object_and_tag))
			{
				// Free successful
				JPH_IF_ENABLE_ASSERTS(mNumFreeObjects += ioBatch.mNumObjects;)

				// Mark the batch as freed
#ifdef JPH_ENABLE_ASSERTS
				ioBatch.mNumObjects = uint32(-1);
#endif		
				return;
			}
		}
	}
}

template <typename Object>
void FixedSizeFreeList<Object>::DestructObject(uint32 inObjectIndex)
{
	JPH_ASSERT(inObjectIndex != cInvalidObjectIndex);

	// Call destructor
	ObjectStorage &storage = GetStorage(inObjectIndex); 
	reinterpret_cast<Object &>(storage.mData).~Object();

	// Add to object free list
	for (;;)
	{
		// Get first object from the list
		uint64 first_free_object_and_tag = mFirstFreeObjectAndTag;
		uint32 first_free = uint32(first_free_object_and_tag);

		// Make it the next pointer of the last object in the batch that is to be freed
		storage.mNextFreeObject = first_free;

		// Construct a new first free object tag
		uint64 new_first_free_object_and_tag = uint64(inObjectIndex) + (uint64(mAllocationTag++) << 32);

		// Compare and swap
		if (mFirstFreeObjectAndTag.compare_exchange_strong(first_free_object_and_tag, new_first_free_object_and_tag))
		{
			// Free successful
			JPH_IF_ENABLE_ASSERTS(mNumFreeObjects++;)
			return;
		}
	}
}

template<typename Object>
inline void FixedSizeFreeList<Object>::DestructObject(Object *inObject)
{
	uint32 index = reinterpret_cast<ObjectStorage *>(inObject)->mNextFreeObject;
	JPH_ASSERT(index < mNumObjectsAllocated);
	DestructObject(index);
}

JPH_NAMESPACE_END
