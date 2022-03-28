// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/FPControlWord.h>

JPH_NAMESPACE_BEGIN

#ifdef JPH_FLOATING_POINT_EXCEPTIONS_ENABLED

#if defined(JPH_USE_SSE)

/// Enable floating point divide by zero exception and exceptions on invalid numbers
class FPExceptionsEnable : public FPControlWord<0, _MM_MASK_DIV_ZERO | _MM_MASK_INVALID> { };

/// Disable invalid floating point value exceptions
class FPExceptionDisableInvalid : public FPControlWord<_MM_MASK_INVALID, _MM_MASK_INVALID> { };

/// Disable division by zero floating point exceptions
class FPExceptionDisableDivByZero : public FPControlWord<_MM_MASK_DIV_ZERO, _MM_MASK_DIV_ZERO> { };

#elif defined(JPH_USE_NEON)

/// Invalid operation exception bit
static constexpr uint64 FP_IOE = 1 << 8;

/// Enable divide by zero exception bit
static constexpr uint64 FP_DZE = 1 << 9;

/// Enable floating point divide by zero exception and exceptions on invalid numbers
class FPExceptionsEnable : public FPControlWord<FP_IOE | FP_DZE, FP_IOE | FP_DZE> { };

/// Disable invalid floating point value exceptions
class FPExceptionDisableInvalid : public FPControlWord<0, FP_IOE> { };

/// Disable division by zero floating point exceptions
class FPExceptionDisableDivByZero : public FPControlWord<0, FP_DZE> { };

#else

#error Unsupported CPU architecture

#endif

#else

/// Dummy implementations
class FPExceptionsEnable { };
class FPExceptionDisableInvalid { };
class FPExceptionDisableDivByZero { };

#endif

JPH_NAMESPACE_END
