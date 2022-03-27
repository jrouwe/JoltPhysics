// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/PhysicsLock.h>

#ifdef JPH_ENABLE_ASSERTS

namespace JPH {

thread_local uint32 PhysicsLock::sLockedMutexes = 0;

} // JPH

#endif
