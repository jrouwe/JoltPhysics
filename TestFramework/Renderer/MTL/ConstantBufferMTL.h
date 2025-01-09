// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

class RendererMTL;

/// A binary blob that can be used to pass constants to a shader
class ConstantBufferMTL
{
public:
	/// Constructor
										ConstantBufferMTL(RendererMTL *inRenderer, uint inBufferSize);
										~ConstantBufferMTL();

	/// Map / unmap buffer (get pointer to data). This will discard all data in the buffer.
	template <typename T> T *			Map()											{ return reinterpret_cast<T *>(MapInternal()); }
	void								Unmap();

private:
	void *								MapInternal();
};
