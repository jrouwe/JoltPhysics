// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/PhysicsLock.h>

namespace JPH {

#ifdef JPH_ENABLE_ASSERTS

thread_local uint32 PhysicsLock::sLockedMutexes = 0;

#endif

} // JPH