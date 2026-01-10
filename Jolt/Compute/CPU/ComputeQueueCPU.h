// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Compute/ComputeQueue.h>

#ifdef JPH_USE_CPU_COMPUTE

#include <Jolt/Compute/CPU/ComputeShaderCPU.h>
#include <Jolt/Core/UnorderedSet.h>

JPH_NAMESPACE_BEGIN

/// A command queue for the CPU compute system
class JPH_EXPORT ComputeQueueCPU final : public ComputeQueue
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Destructor
	virtual								~ComputeQueueCPU() override;

	// See: ComputeQueue
	virtual void						SetShader(const ComputeShader *inShader) override;
	virtual void						SetConstantBuffer(const char *inName, const ComputeBuffer *inBuffer) override;
	virtual void						SetBuffer(const char *inName, const ComputeBuffer *inBuffer) override;
	virtual void 						SetRWBuffer(const char *inName, ComputeBuffer *inBuffer, EBarrier inBarrier = EBarrier::Yes) override;
	virtual void						ScheduleReadback(ComputeBuffer *inDst, const ComputeBuffer *inSrc) override;
	virtual void						Dispatch(uint inThreadGroupsX, uint inThreadGroupsY, uint inThreadGroupsZ) override;
	virtual void						Execute() override;
	virtual void						Wait() override;

private:
	RefConst<ComputeShaderCPU>			mShader = nullptr;							///< Current active shader
	ShaderWrapper *						mWrapper = nullptr;							///< The active shader wrapper
	UnorderedSet<RefConst<ComputeBuffer>> mUsedBuffers;								///< Buffers that are in use by the current execution, these will be retained until execution is finished so that we don't free buffers that are in use
};

JPH_NAMESPACE_END

#endif // JPH_USE_CPU_COMPUTE
