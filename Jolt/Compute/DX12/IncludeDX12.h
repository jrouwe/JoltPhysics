// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/IncludeWindows.h>
#include <Jolt/Core/StringTools.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
JPH_MSVC_SUPPRESS_WARNING(4265) // 'X': class has virtual functions, but its non-trivial destructor is not virtual; instances of this class may not be destructed correctly
JPH_MSVC_SUPPRESS_WARNING(4625) // 'X': copy constructor was implicitly defined as deleted
JPH_MSVC_SUPPRESS_WARNING(4626) // 'X': assignment operator was implicitly defined as deleted
JPH_MSVC_SUPPRESS_WARNING(5204) // 'X': class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly
JPH_MSVC_SUPPRESS_WARNING(5220) // 'X': a non-static data member with a volatile qualified type no longer implies
JPH_MSVC2026_PLUS_SUPPRESS_WARNING(4865) // wingdi.h(2806,1): '<unnamed-enum-DISPLAYCONFIG_OUTPUT_TECHNOLOGY_OTHER>': the underlying type will change from 'int' to '__int64' when '/Zc:enumTypes' is specified on the command line
#include <d3d12.h>
#include <dxgi1_6.h>
#include <dxgidebug.h>
#include <wrl.h>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

using Microsoft::WRL::ComPtr;

template <class Result>
inline bool HRFailed(HRESULT inHR, Result &outResult)
{
	if (SUCCEEDED(inHR))
		return false;

	String error = StringFormat("Call failed with error code: %08X", inHR);
	outResult.SetError(error);
	JPH_ASSERT(false);
	return true;
}

inline bool HRFailed(HRESULT inHR)
{
	if (SUCCEEDED(inHR))
		return false;

	Trace("Call failed with error code: %08X", inHR);
	JPH_ASSERT(false);
	return true;
}

JPH_NAMESPACE_END
