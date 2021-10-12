// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Body/BodyAccess.h>

#ifdef JPH_ENABLE_ASSERTS

namespace JPH {

thread_local BodyAccess::EAccess	BodyAccess::sVelocityAccess = BodyAccess::EAccess::ReadWrite;
thread_local BodyAccess::EAccess	BodyAccess::sPositionAccess = BodyAccess::EAccess::ReadWrite;

} // JPH

#endif
