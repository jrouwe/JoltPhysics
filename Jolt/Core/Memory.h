// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

#ifndef JPH_DISABLE_CUSTOM_ALLOCATOR

// Normal memory allocation, must be at least 8 byte aligned on 32 bit platform and 16 byte aligned on 64 bit platform
using AllocFunction = void *(*)(size_t inSize);
using FreeFunction = void (*)(void *inBlock);

// Aligned memory allocation
using AlignedAllocFunction = void *(*)(size_t inSize, size_t inAlignment);
using AlignedFreeFunction = void (*)(void *inBlock);

// User defined allocation / free functions
extern AllocFunction Alloc;
extern FreeFunction Free;
extern AlignedAllocFunction AlignedAlloc;
extern AlignedFreeFunction AlignedFree;

/// Register platform default allocation / free functions
void RegisterDefaultAlloc();

/// Macro to override the new and delete functions
#define JPH_OVERRIDE_NEW_DELETE \
	JPH_INLINE void *operator new (size_t inCount)											{ return Alloc(inCount); } \
	JPH_INLINE void operator delete (void *inPointer) noexcept								{ Free(inPointer); } \
	JPH_INLINE void *operator new[] (size_t inCount)										{ return Alloc(inCount); } \
	JPH_INLINE void operator delete[] (void *inPointer) noexcept							{ Free(inPointer); } \
	JPH_INLINE void *operator new (size_t inCount, align_val_t inAlignment)					{ return AlignedAlloc(inCount, static_cast<size_t>(inAlignment)); } \
	JPH_INLINE void operator delete (void *inPointer, align_val_t inAlignment) noexcept		{ AlignedFree(inPointer); } \
	JPH_INLINE void *operator new[] (size_t inCount, align_val_t inAlignment)				{ return AlignedAlloc(inCount, static_cast<size_t>(inAlignment)); } \
	JPH_INLINE void operator delete[] (void *inPointer, align_val_t inAlignment) noexcept	{ AlignedFree(inPointer); }

#else

// Directly define the allocation functions
void *Alloc(size_t inSize);
void Free(void *inBlock);
void *AlignedAlloc(size_t inSize, size_t inAlignment);
void AlignedFree(void *inBlock);

// Don't implement allocator registering
inline void RegisterDefaultAlloc() { }

// Don't override new/delete
#define JPH_OVERRIDE_NEW_DELETE

#endif // !JPH_DISABLE_CUSTOM_ALLOCATOR

JPH_NAMESPACE_END
