// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#define JPH_SHADER_NAME TestCompute
#include <Jolt/Compute/CPU/WrapShaderBegin.h>
#include "TestCompute.hlsl"
#include <Jolt/Compute/CPU/WrapShaderBindings.h>
#include "TestComputeBindings.h"
#include <Jolt/Compute/CPU/WrapShaderEnd.h>
