// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_USE_DX12

#include <Jolt/Compute/ComputeShader.h>
#include <Jolt/Compute/DX12/IncludeDX12.h>
#include <Jolt/Core/UnorderedMap.h>

JPH_NAMESPACE_BEGIN

/// Compute shader handle for DirectX
class JPH_EXPORT ComputeShaderDX12 : public ComputeShader
{
public:
	JPH_OVERRIDE_NEW_DELETE

	/// Constructor
									ComputeShaderDX12(ComPtr<ID3DBlob> inShader, ComPtr<ID3D12RootSignature> inRootSignature, ComPtr<ID3D12PipelineState> inPipelineState, Array<String> &&inBindingNames, UnorderedMap<string_view, uint> &&inNameToIndex, uint32 inGroupSizeX, uint32 inGroupSizeY, uint32 inGroupSizeZ) :
		ComputeShader(inGroupSizeX, inGroupSizeY, inGroupSizeZ),
		mShader(inShader),
		mRootSignature(inRootSignature),
		mPipelineState(inPipelineState),
		mBindingNames(std::move(inBindingNames)),
		mNameToIndex(std::move(inNameToIndex))
	{
	}

	/// Get index of shader parameter
	uint							NameToIndex(const char *inName) const
	{
		UnorderedMap<string_view, uint>::const_iterator it = mNameToIndex.find(inName);
		JPH_ASSERT(it != mNameToIndex.end());
		return it->second;
	}

	/// Getters
	ID3D12PipelineState *			GetPipelineState() const				{ return mPipelineState.Get(); }
	ID3D12RootSignature *			GetRootSignature() const				{ return mRootSignature.Get(); }

private:
	ComPtr<ID3DBlob>				mShader;								///< The compiled shader
	ComPtr<ID3D12RootSignature>		mRootSignature;							///< The root signature for this shader
	ComPtr<ID3D12PipelineState>		mPipelineState;							///< The pipeline state object for this shader
	Array<String>					mBindingNames;							///< A list of binding names, mNameToIndex points to these strings
	UnorderedMap<string_view, uint>	mNameToIndex;							///< Maps names to indices for the shader parameters, using a string_view so we can do find() without an allocation
};

JPH_NAMESPACE_END

#endif // JPH_USE_DX12
