// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <cstdlib>
JPH_SUPPRESS_WARNINGS_STD_END
#include <stdlib.h>

JPH_NAMESPACE_BEGIN

AllocFunction Alloc = nullptr;
FreeFunction Free = nullptr;
AlignedAllocFunction AlignedAlloc = nullptr;
AlignedFreeFunction AlignedFree = nullptr;

static void *AllocImpl(size_t inSize)
{
	return malloc(inSize);
}

static void FreeImpl(void *inBlock)
{
	free(inBlock);
}

static void *AlignedAllocImpl(size_t inSize, size_t inAlignment)
{
#if defined(JPH_PLATFORM_WINDOWS)
	// Microsoft doesn't implement C++17 std::aligned_alloc
	return _aligned_malloc(inSize, inAlignment);
#elif defined(JPH_PLATFORM_ANDROID)
	return memalign(inAlignment, AlignUp(inSize, inAlignment));
#else
	return std::aligned_alloc(inAlignment, AlignUp(inSize, inAlignment));
#endif
}

static void AlignedFreeImpl(void *inBlock)
{
#if defined(JPH_PLATFORM_WINDOWS)
	_aligned_free(inBlock);
#elif defined(JPH_PLATFORM_ANDROID)
	free(inBlock);
#else
	std::free(inBlock);
#endif
}

void RegisterDefaultAlloc()
{
	Alloc = AllocImpl;
	Free = FreeImpl;
	AlignedAlloc = AlignedAllocImpl;
	AlignedFree = AlignedFreeImpl;
}

JPH_NAMESPACE_END
