// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Math/DVec3.h>
#include <Jolt/Math/DMat44.h>

JPH_NAMESPACE_BEGIN

#ifdef JPH_DOUBLE_PRECISION

// Define real to double
using Real = double;
using RVec3 = DVec3;
using RVec3Arg = DVec3Arg;
using RMat44 = DMat44;
using RMat44Arg = DMat44Arg;

#define JPH_RVECTOR_ALIGNMENT JPH_DVECTOR_ALIGNMENT

#else

// Define real to float
using Real = float;
using RVec3  = Vec3;
using RVec3Arg = Vec3Arg;
using RMat44 = Mat44;
using RMat44Arg = Mat44Arg;

#define JPH_RVECTOR_ALIGNMENT JPH_VECTOR_ALIGNMENT

#endif // JPH_DOUBLE_PRECISION

JPH_NAMESPACE_END
