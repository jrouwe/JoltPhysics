// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Physics/Collision/CollisionDispatch.h>

namespace JPH {

CollisionDispatch::CollideShape CollisionDispatch::sCollideShape[NumSubShapeTypes][NumSubShapeTypes];
CollisionDispatch::CastShape CollisionDispatch::sCastShape[NumSubShapeTypes];

} // JPH