// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

class Renderer;

/// A binary blob that can be used to pass constants to a shader
class ConstantBuffer
{
public:
	/// Constructor
										ConstantBuffer(Renderer *inRenderer, uint64 inBufferSize);
										~ConstantBuffer();

	/// Map / unmap buffer (get pointer to data). This will discard all data in the buffer.
	template <typename T> T *			Map()											{ return reinterpret_cast<T *>(MapInternal()); }
	void								Unmap();

	// Bind the constant buffer to a slot
	void								Bind(int inSlot);

private:
	friend class Renderer;

	void *								MapInternal();

	Renderer *							mRenderer;
    ComPtr<ID3D12Resource>				mBuffer;
	uint64								mBufferSize;
};
