// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/NonCopyable.h>

JPH_NAMESPACE_BEGIN

#ifdef JPH_USE_SSE

/// Helper class that needs to be put on the stack to update the state of the floating point control word.
/// This state is kept per thread.
template <uint Value, uint Mask>
class FPControlWord : public NonCopyable
{
public:
				FPControlWord()
	{
		mPrevState = _mm_getcsr();
		_mm_setcsr((mPrevState & ~Mask) | Value);
	}

				~FPControlWord()
	{
		_mm_setcsr((_mm_getcsr() & ~Mask) | (mPrevState & Mask));
	}

private:
	uint		mPrevState;	
};

#elif defined(JPH_USE_NEON)

/// Helper class that needs to be put on the stack to update the state of the floating point control word.
/// This state is kept per thread.
template <uint64 Value, uint64 Mask>
class FPControlWord : public NonCopyable
{
public:
				FPControlWord()
	{
		uint64 val;
	    asm volatile("mrs %0, fpcr" : "=r" (val));
		mPrevState = val;
		val &= ~Mask;
		val |= Value;
	    asm volatile("msr fpcr, %0" : /* no output */ : "r" (val));
	}

				~FPControlWord()
	{
		uint64 val;
		asm volatile("mrs %0, fpcr" : "=r" (val));
		val &= ~Mask;
		val |= mPrevState & Mask;
		asm volatile("msr fpcr, %0" : /* no output */ : "r" (val));
	}

private:
	uint64		mPrevState;
};

#else

#error Unsupported CPU architecture

#endif

JPH_NAMESPACE_END
