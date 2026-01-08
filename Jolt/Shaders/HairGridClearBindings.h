// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairStructs.h"

JPH_SHADER_BIND_BEGIN(JPH_HairGridClear)
	JPH_SHADER_BIND_RW_BUFFER(JPH_int4, gVelocityAndDensity)
JPH_SHADER_BIND_END(JPH_HairGridClear)
