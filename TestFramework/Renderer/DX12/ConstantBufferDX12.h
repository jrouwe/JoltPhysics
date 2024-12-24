// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

class RendererDX12;

/// A binary blob that can be used to pass constants to a shader
class ConstantBufferDX12
{
public:
	/// Constructor
										ConstantBufferDX12(RendererDX12 *inRenderer, uint64 inBufferSize);
										~ConstantBufferDX12();

	/// Map / unmap buffer (get pointer to data). This will discard all data in the buffer.
	template <typename T> T *			Map()											{ return reinterpret_cast<T *>(MapInternal()); }
	void								Unmap();

	// Bind the constant buffer to a slot
	void								Bind(int inSlot);

private:
	friend class RendererDX12;

	void *								MapInternal();

	RendererDX12 *						mRenderer;
	ComPtr<ID3D12Resource>				mBuffer;
	uint64								mBufferSize;
};
