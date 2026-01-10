// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/ComputeShader.h>

#ifdef JPH_USE_CPU_COMPUTE

JPH_NAMESPACE_BEGIN

class ShaderWrapper;

/// Compute shader handle for CPU compute
class JPH_EXPORT ComputeShaderCPU : public ComputeShader
{
public:
	JPH_OVERRIDE_NEW_DELETE

	using CreateShader = ShaderWrapper *(*)();

	/// Constructor
									ComputeShaderCPU(CreateShader inCreateShader, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ) :
		ComputeShader(inGroupSizeX, inGroupSizeY, inGroupSizeZ),
		mCreateShader(inCreateShader)
	{
	}

	/// Create an instance of the shader wrapper
	ShaderWrapper *					CreateWrapper() const
	{
		return mCreateShader();
	}

private:
	CreateShader					mCreateShader;
};

JPH_NAMESPACE_END

#endif // JPH_USE_CPU_COMPUTE
