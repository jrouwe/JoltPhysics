// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Renderer/Texture.h>

/// Helper class that points to a subsection of a texture for rendering it as a quad. Can specify borders which won't scale (only inner part of the quad will scale).
class UITexturedQuad
{
public:
	/// Constructor
							UITexturedQuad() = default;
							UITexturedQuad(const Texture *inTexture) : mTexture(inTexture), mWidth(inTexture->GetWidth()), mHeight(inTexture->GetHeight()) { }
							UITexturedQuad(const Texture *inTexture, int inX, int inY, int inWidth, int inHeight) : mTexture(inTexture), mX(inX), mY(inY), mWidth(inWidth), mHeight(inHeight) { }
							UITexturedQuad(const Texture *inTexture, int inX, int inY, int inWidth, int inHeight, int inInnerX, int inInnerY, int inInnerWidth, int inInnerHeight) : mTexture(inTexture), mX(inX), mY(inY), mWidth(inWidth), mHeight(inHeight), mInnerX(inInnerX), mInnerY(inInnerY), mInnerWidth(inInnerWidth), mInnerHeight(inInnerHeight) { }

	/// Check if this quad consists of 9 parts
	bool					HasInnerPart() const			{ return mInnerX >= 0 && mInnerY >= 0 && mInnerWidth >= 0 && mInnerHeight >= 0; }

	/// The texture to use
	RefConst<Texture>		mTexture;

	/// These are the normal texel coordinates for the quad
	int						mX = 0;
	int						mY = 0;
	int						mWidth = 0;
	int						mHeight = 0;

	/// This quad can also scale its inner part leaving borders in tact, in this case the inner scaling part is defined here
	int						mInnerX = -1;
	int						mInnerY = -1;
	int						mInnerWidth = -1;
	int						mInnerHeight = -1;
};
