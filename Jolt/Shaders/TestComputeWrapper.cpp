// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_CPU_COMPUTE

#define JPH_SHADER_NAME TestCompute
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "TestCompute.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "TestComputeBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#define JPH_SHADER_NAME TestCompute2
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "TestCompute2.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "TestCompute2Bindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>

#endif // JPH_USE_CPU_COMPUTE
