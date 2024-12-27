// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>

/// Forward declares
class Surface;

class Texture : public RefTarget<Texture>
{
public:
	/// Constructor
										Texture(int inWidth, int inHeight) : mWidth(inWidth), mHeight(inHeight) { }
	virtual								~Texture() = default;

	/// Get dimensions of texture
	inline int							GetWidth() const		{ return mWidth; }
	inline int							GetHeight() const		{ return mHeight; }

	/// Bind texture to the pixel shader
	virtual void						Bind() const = 0;

protected:
	int									mWidth;
	int									mHeight;
};
