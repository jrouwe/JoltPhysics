// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Core/NonCopyable.h>
#include <Jolt/Core/Result.h>

JPH_NAMESPACE_BEGIN

class ComputeBuffer;
using ComputeBufferResult = Result<Ref<ComputeBuffer>>;

/// Buffer that can be read from / written to by a compute shader
class JPH_EXPORT ComputeBuffer : public RefTarget<ComputeBuffer>, public NonCopyable
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Type of buffer
	enum class EType
	{
		UploadBuffer,			///< Buffer that can be written on the CPU and then uploaded to the GPU.
		ReadbackBuffer,			///< Buffer to be sent from the GPU to the CPU, used to read back data.
		ConstantBuffer,			///< A smallish buffer that is used to pass constants to a shader.
		Buffer,					///< Buffer that can be read from by a shader. Must be initialized with data at construction time and is read only thereafter.
		RWBuffer,				///< Buffer that can be read from and written to by a shader.
	};

	/// Constructor / Destructor
								ComputeBuffer(EType inType, uint64 inSize, uint inStride) : mType(inType), mSize(inSize), mStride(inStride) { }
	virtual						~ComputeBuffer()								{ JPH_ASSERT(!mIsMapped); }

	/// Properties
	EType						GetType() const									{ return mType; }
	uint64						GetSize() const									{ return mSize; }
	uint						GetStride() const								{ return mStride; }

	/// Mode in which the buffer is accessed
	enum class EMode
	{
		Read,					///< Read only access to the buffer
		Write,					///< Write only access to the buffer (this will discard all previous data in the buffer)
	};

	/// Map / unmap buffer (get pointer to data).
	void *						Map(EMode inMode)								{ JPH_ASSERT(!mIsMapped); JPH_IF_ENABLE_ASSERTS(mIsMapped = true;) return MapInternal(inMode); }
	template <typename T> T *	Map(EMode inMode)								{ JPH_ASSERT(!mIsMapped); JPH_IF_ENABLE_ASSERTS(mIsMapped = true;) JPH_ASSERT(sizeof(T) == mStride); return reinterpret_cast<T *>(MapInternal(inMode)); }
	void						Unmap()											{ JPH_ASSERT(mIsMapped); JPH_IF_ENABLE_ASSERTS(mIsMapped = false;) UnmapInternal(); }

	/// Create a readback buffer of the same size and stride that can be used to read the data stored in this buffer on CPU.
	/// Note that this could also be implemented as 'return this' in case the underlying implementation allows locking GPU data on CPU directly.
	virtual ComputeBufferResult	CreateReadBackBuffer() const = 0;

protected:
	EType						mType;
	uint64						mSize;
	uint						mStride;
#ifdef JPH_ENABLE_ASSERTS
	bool						mIsMapped = false;
#endif // JPH_ENABLE_ASSERTS

	virtual void *				MapInternal(EMode inMode) = 0;
	virtual void				UnmapInternal() = 0;
};

JPH_NAMESPACE_END
