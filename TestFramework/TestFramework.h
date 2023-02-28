// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Jolt.h>

// Targetting Windows 10 and above
#define WINVER 0x0A00
#define _WIN32_WINNT 0x0A00

// Disable common warnings
JPH_SUPPRESS_WARNINGS
JPH_CLANG_SUPPRESS_WARNING("-Wheader-hygiene")
#ifdef JPH_DOUBLE_PRECISION
JPH_CLANG_SUPPRESS_WARNING("-Wdouble-promotion")
#endif // JPH_DOUBLE_PRECISION

JPH_SUPPRESS_WARNING_PUSH
JPH_MSVC_SUPPRESS_WARNING(5039) // winbase.h(13179): warning C5039: 'TpSetCallbackCleanupGroup': pointer or reference to potentially throwing function passed to 'extern "C"' function under -EHc. Undefined behavior may occur if this function throws an exception.
JPH_MSVC_SUPPRESS_WARNING(5204) // implements.h(65): warning C5204: 'Microsoft::WRL::CloakedIid<IMarshal>': class has virtual functions, but its trivial destructor is not virtual; instances of objects derived from this class may not be destructed correctly
JPH_MSVC_SUPPRESS_WARNING(4265) // implements.h(1449): warning C4265: 'Microsoft::WRL::FtmBase': class has virtual functions, but its non-trivial destructor is not virtual; instances of this class may not be destructed correctly
JPH_MSVC_SUPPRESS_WARNING(5220) // implements.h(1648): warning C5220: 'Microsoft::WRL::Details::RuntimeClassImpl<Microsoft::WRL::RuntimeClassFlags<2>,true,false,true,IWeakReference>::refcount_': a non-static data member with a volatile qualified type no longer implies
JPH_MSVC_SUPPRESS_WARNING(4986) // implements.h(2343): warning C4986: 'Microsoft::WRL::Details::RuntimeClassImpl<RuntimeClassFlagsT,true,true,false,I0,TInterfaces...>::GetWeakReference': exception specification does not match previous declaration
#define WIN32_LEAN_AND_MEAN
#define Ellipse DrawEllipse // Windows.h defines a name that we would like to use
#include <windows.h>
#undef Ellipse
#undef min // We'd like to use std::min and max instead of the ones defined in windows.h
#undef max
#undef DrawText // We don't want this to map to DrawTextW
#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h> // for ComPtr
JPH_SUPPRESS_WARNING_POP

using Microsoft::WRL::ComPtr;
using namespace JPH;
using namespace JPH::literals;
using namespace std;
