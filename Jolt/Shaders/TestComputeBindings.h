// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

JPH_SHADER_BIND_BEGIN(JPH_TestCompute)
	JPH_SHADER_BIND_BUFFER(JPH_uint, gUploadData)
	JPH_SHADER_BIND_BUFFER(JPH_uint, gOptionalData)
	JPH_SHADER_BIND_RW_BUFFER(JPH_uint, gData)
JPH_SHADER_BIND_END
