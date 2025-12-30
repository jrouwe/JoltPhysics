// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <Jolt/Core/NonCopyable.h>
#include <Jolt/Core/Result.h>

JPH_NAMESPACE_BEGIN

/// Compute shader handle
class JPH_EXPORT ComputeShader : public RefTarget<ComputeShader>, public NonCopyable
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor / destructor
							ComputeShader(uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ) :
		mGroupSizeX(inGroupSizeX),
		mGroupSizeY(inGroupSizeY),
		mGroupSizeZ(inGroupSizeZ)
	{
	}
	virtual					~ComputeShader() = default;

	/// Get group sizes
	uint32					GetGroupSizeX() const						{ return mGroupSizeX; }
	uint32					GetGroupSizeY() const						{ return mGroupSizeY; }
	uint32					GetGroupSizeZ() const						{ return mGroupSizeZ; }

private:
	uint32					mGroupSizeX;
	uint32					mGroupSizeY;
	uint32					mGroupSizeZ;
};

using ComputeShaderResult = Result<Ref<ComputeShader>>;

JPH_NAMESPACE_END
