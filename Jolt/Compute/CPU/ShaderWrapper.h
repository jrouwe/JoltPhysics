// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_USE_CPU_COMPUTE

JPH_NAMESPACE_BEGIN

namespace HLSLToCPP { struct uint3; }

/// Wraps a compute shader to allow calling it from C++
class ShaderWrapper
{
public:
	/// Destructor
	virtual				~ShaderWrapper() = default;

	/// Bind buffer to shader
	virtual void		Bind(const char *inName, void *inData, uint64 inSize) = 0;

	/// Execute a single shader thread
	virtual void		Main(const HLSLToCPP::uint3 &inThreadID) = 0;
};

JPH_NAMESPACE_END

#endif // JPH_USE_CPU_COMPUTE
