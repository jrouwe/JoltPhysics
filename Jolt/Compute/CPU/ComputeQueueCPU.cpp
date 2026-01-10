// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#ifdef JPH_USE_CPU_COMPUTE

#include <Jolt/Compute/CPU/ComputeQueueCPU.h>
#include <Jolt/Compute/CPU/ComputeShaderCPU.h>
#include <Jolt/Compute/CPU/ComputeBufferCPU.h>
#include <Jolt/Compute/CPU/ShaderWrapper.h>
#include <Jolt/Compute/CPU/HLSLToCPP.h>

JPH_NAMESPACE_BEGIN

ComputeQueueCPU::~ComputeQueueCPU()
{
	JPH_ASSERT(mShader == nullptr && mWrapper == nullptr);
}

void ComputeQueueCPU::SetShader(const ComputeShader *inShader)
{
	JPH_ASSERT(mShader == nullptr && mWrapper == nullptr);

	mShader = static_cast<const ComputeShaderCPU *>(inShader);
	mWrapper = mShader->CreateWrapper();
}

void ComputeQueueCPU::SetConstantBuffer(const char *inName, const ComputeBuffer *inBuffer)
{
	if (inBuffer == nullptr)
		return;
	JPH_ASSERT(inBuffer->GetType() == ComputeBuffer::EType::ConstantBuffer);
	const ComputeBufferCPU *buffer = static_cast<const ComputeBufferCPU *>(inBuffer);
	mWrapper->Bind(inName, buffer->GetData(), buffer->GetSize() * buffer->GetStride());

	mUsedBuffers.insert(buffer);
}

void ComputeQueueCPU::SetBuffer(const char *inName, const ComputeBuffer *inBuffer)
{
	if (inBuffer == nullptr)
		return;
	JPH_ASSERT(inBuffer->GetType() == ComputeBuffer::EType::UploadBuffer || inBuffer->GetType() == ComputeBuffer::EType::Buffer || inBuffer->GetType() == ComputeBuffer::EType::RWBuffer);
	const ComputeBufferCPU *buffer = static_cast<const ComputeBufferCPU *>(inBuffer);
	mWrapper->Bind(inName, buffer->GetData(), buffer->GetSize() * buffer->GetStride());

	mUsedBuffers.insert(buffer);
}

void ComputeQueueCPU::SetRWBuffer(const char *inName, ComputeBuffer *inBuffer, EBarrier inBarrier)
{
	if (inBuffer == nullptr)
		return;
	JPH_ASSERT(inBuffer->GetType() == ComputeBuffer::EType::RWBuffer);
	const ComputeBufferCPU *buffer = static_cast<const ComputeBufferCPU *>(inBuffer);
	mWrapper->Bind(inName, buffer->GetData(), buffer->GetSize() * buffer->GetStride());

	mUsedBuffers.insert(buffer);
}

void ComputeQueueCPU::ScheduleReadback(ComputeBuffer *inDst, const ComputeBuffer *inSrc)
{
	/* Nothing to read back */
}

void ComputeQueueCPU::Dispatch(uint inThreadGroupsX, uint inThreadGroupsY, uint inThreadGroupsZ)
{
	uint nx = inThreadGroupsX * mShader->GetGroupSizeX();
	uint ny = inThreadGroupsY * mShader->GetGroupSizeY();
	uint nz = inThreadGroupsZ * mShader->GetGroupSizeZ();

	for (uint z = 0; z < nz; ++z)
		for (uint y = 0; y < ny; ++y)
			for (uint x = 0; x < nx; ++x)
			{
				HLSLToCPP::uint3 tid { x, y, z };
				mWrapper->Main(tid);
			}

	delete mWrapper;
	mWrapper = nullptr;

	mUsedBuffers.clear();
	mShader = nullptr;
}

void ComputeQueueCPU::Execute()
{
	/* Nothing to do */
}

void ComputeQueueCPU::Wait()
{
	/* Nothing to do */
}

JPH_NAMESPACE_END

#endif // JPH_USE_CPU_COMPUTE
