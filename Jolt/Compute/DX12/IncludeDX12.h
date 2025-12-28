// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_USE_DX12

JPH_SUPPRESS_WARNINGS_STD_BEGIN
JPH_MSVC_SUPPRESS_WARNING(4265) // 'X': class has virtual functions, but its non-trivial destructor is not virtual; instances of this class may not be destructed correctly
JPH_MSVC_SUPPRESS_WARNING(4625) // 'X': copy constructor was implicitly defined as deleted
JPH_MSVC_SUPPRESS_WARNING(4626) // 'X': assignment operator was implicitly defined as deleted
JPH_MSVC_SUPPRESS_WARNING(5204) // 'X': class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly
JPH_MSVC_SUPPRESS_WARNING(5220) // 'X': a non-static data member with a volatile qualified type no longer implies
JPH_MSVC2026_PLUS_SUPPRESS_WARNING(4865) // wingdi.h(2806,1): '<unnamed-enum-DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER>': the underlying type will change from 'int' to '__int64' when '/Zc:enumTypes' is specified on the command line
JPH_CLANG_SUPPRESS_WARNING("-Wreserved-macro-identifier") // Complains about _WIN32_WINNT being reserved
#define WINVER 0x0A00 // Targeting Windows 10 and above
#define _WIN32_WINNT 0x0A00
#define WIN32_LEAN_AND_MEAN
#ifndef JPH_COMPILER_MINGW
	#include <Windows.h>
#else
	#include <windows.h>
#endif
#undef min // We'd like to use std::min and max instead of the ones defined in windows.h
#undef max
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <wrl.h>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

using Microsoft::WRL::ComPtr;

inline bool HRFailed(HRESULT inHR)
{
	if (SUCCEEDED(inHR))
		return false;

	Trace("Call failed with error code: %08X", inHR);
	JPH_ASSERT(false);
	return true;
}

JPH_NAMESPACE_END

#endif // JPH_USE_DX12
