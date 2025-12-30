// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Core/NonCopyable.h>
#include <Jolt/Core/Result.h>

JPH_NAMESPACE_BEGIN

class ComputeShader;
class ComputeBuffer;

/// A command queue for executing compute workloads on the GPU.
///
/// Note that only a single thread should be using a ComputeQueue at any time (although an implementation could be made that is thread safe).
class JPH_EXPORT ComputeQueue : public RefTarget<ComputeQueue>, public NonCopyable
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Destructor
	virtual					~ComputeQueue() = default;

	/// Activate a shader. Shader must be set first before buffers can be bound.
	/// After every Dispatch call, the shader must be set again and all buffers must be bound again.
	virtual void			SetShader(const ComputeShader *inShader) = 0;

	/// If a barrier should be placed before accessing the buffer
	enum class EBarrier
	{
		Yes,
		No
	};

	/// Bind a constant buffer to the shader. Note that the contents of the buffer cannot be modified until execution finishes.
	/// A reference to the buffer is added to make sure it stays alive until execution finishes.
	/// @param inName Name of the buffer as specified in the shader.
	/// @param inBuffer The buffer to bind.
	virtual void			SetConstantBuffer(const char *inName, const ComputeBuffer *inBuffer) = 0;

	/// Bind a read only buffer to the shader. Note that the contents of the buffer cannot be modified on CPU until execution finishes (only relevant for buffers of type UploadBuffer).
	/// A reference to the buffer is added to make sure it stays alive until execution finishes.
	/// @param inName Name of the buffer as specified in the shader.
	/// @param inBuffer The buffer to bind.
	virtual void			SetBuffer(const char *inName, const ComputeBuffer *inBuffer) = 0;

	/// Bind a read/write buffer to the shader.
	/// A reference to the buffer is added to make sure it stays alive until execution finishes.
	/// @param inName Name of the buffer as specified in the shader.
	/// @param inBuffer The buffer to bind.
	/// @param inBarrier If set to Yes, a barrier will be placed before accessing the buffer to ensure all previous writes to the buffer are visible.
	virtual void 			SetRWBuffer(const char *inName, ComputeBuffer *inBuffer, EBarrier inBarrier = EBarrier::Yes) = 0;

	/// Dispatch a compute shader with the specified number of thread groups
	virtual void			Dispatch(uint inThreadGroupsX, uint inThreadGroupsY = 1, uint inThreadGroupsZ = 1) = 0;

	/// Schedule buffer to be copied from GPU to CPU.
	/// A reference to the buffers is added to make sure they stay alive until execution finishes.
	virtual void			ScheduleReadback(ComputeBuffer *inDst, const ComputeBuffer *inSrc) = 0;

	/// Execute accumulated command list.
	/// No more commands can be added until Wait is called.
	virtual void			Execute() = 0;

	/// After executing, this waits until execution is done.
	/// This also makes sure that any readback operations have completed and the data is available on CPU.
	virtual void			Wait() = 0;

	/// Execute and wait for the command list to finish
	/// @see Execute, Wait
	void					ExecuteAndWait()
	{
		Execute();
		Wait();
	}
};

using ComputeQueueResult = Result<Ref<ComputeQueue>>;

JPH_NAMESPACE_END
