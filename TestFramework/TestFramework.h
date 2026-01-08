// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Jolt.h>

// Disable common warnings
JPH_SUPPRESS_WARNINGS
JPH_CLANG_SUPPRESS_WARNING("-Wheader-hygiene")
#ifdef JPH_DOUBLE_PRECISION
JPH_CLANG_SUPPRESS_WARNING("-Wdouble-promotion")
#endif // JPH_DOUBLE_PRECISION
JPH_CLANG_SUPPRESS_WARNING("-Wswitch-enum")
JPH_CLANG_SUPPRESS_WARNING("-Wswitch")
JPH_MSVC_SUPPRESS_WARNING(4061) // enumerator 'X' in switch of enum 'X' is not explicitly handled by a case label
JPH_MSVC_SUPPRESS_WARNING(4062) // enumerator 'X' in switch of enum 'X' is not handled

#ifdef JPH_PLATFORM_WINDOWS

#ifdef _WIN32_WINNT
	#undef _WIN32_WINNT
#endif
#define Ellipse DrawEllipse // Windows.h defines a name that we would like to use
#include <Jolt/Compute/DX12/IncludeDX12.h>
#undef Ellipse
#undef DrawText // We don't want this to map to DrawTextW

#elif defined(JPH_PLATFORM_LINUX)

#define Font X11Font
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/XKBlib.h>
#undef Font
#undef Success
#undef None
#undef Convex

#endif

// Precompile frequently used Jolt Physics headers for better build performance
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/Body/BodyLock.h>
#include <Jolt/Physics/Body/BodyLockInterface.h>
#include <Jolt/Physics/Body/BodyManager.h>
#include <Jolt/Physics/Collision/NarrowPhaseQuery.h>
#include <Jolt/Physics/Collision/Shape/Shape.h>
#include <Jolt/Physics/Constraints/ContactConstraintManager.h>
#include <Jolt/Core/Mutex.h>
#include <Jolt/Core/Profiler.h>
#include <Jolt/Skeleton/SkeletonPose.h>

using namespace JPH;
using namespace JPH::literals;
using namespace std;
