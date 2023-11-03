// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <cstdlib>
JPH_SUPPRESS_WARNINGS_STD_END
#include <stdlib.h>

JPH_NAMESPACE_BEGIN

#ifdef JPH_DISABLE_CUSTOM_ALLOCATOR
	#define JPH_ALLOC_FN(x)	x
	#define JPH_ALLOC_SCOPE
#else
	#define JPH_ALLOC_FN(x)	x##Impl
	#define JPH_ALLOC_SCOPE static
#endif

JPH_ALLOC_SCOPE void *JPH_ALLOC_FN(Allocate)(size_t inSize)
{
	return malloc(inSize);
}

JPH_ALLOC_SCOPE void JPH_ALLOC_FN(Free)(void *inBlock)
{
	free(inBlock);
}

JPH_ALLOC_SCOPE void *JPH_ALLOC_FN(AlignedAllocate)(size_t inSize, size_t inAlignment)
{
#if defined(JPH_PLATFORM_WINDOWS)
	// Microsoft doesn't implement posix_memalign
	return _aligned_malloc(inSize, inAlignment);
#else
	void *block = nullptr;
	JPH_SUPPRESS_WARNING_PUSH
	JPH_GCC_SUPPRESS_WARNING("-Wunused-result")
	posix_memalign(&block, inAlignment, inSize);
	JPH_SUPPRESS_WARNING_POP
	return block;
#endif
}

JPH_ALLOC_SCOPE void JPH_ALLOC_FN(AlignedFree)(void *inBlock)
{
#if defined(JPH_PLATFORM_WINDOWS)
	_aligned_free(inBlock);
#else
	free(inBlock);
#endif
}

#ifndef JPH_DISABLE_CUSTOM_ALLOCATOR

AllocateFunction Allocate = nullptr;
FreeFunction Free = nullptr;
AlignedAllocateFunction AlignedAllocate = nullptr;
AlignedFreeFunction AlignedFree = nullptr;

void RegisterDefaultAllocator()
{
	Allocate = AllocateImpl;
	Free = FreeImpl;
	AlignedAllocate = AlignedAllocateImpl;
	AlignedFree = AlignedFreeImpl;
}

#endif // JPH_DISABLE_CUSTOM_ALLOCATOR

JPH_NAMESPACE_END
