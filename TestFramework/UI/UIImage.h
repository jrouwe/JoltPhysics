// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <UI/UIElement.h>
#include <UI/UITexturedQuad.h>

/// A static image UI element
class UIImage : public UIElement
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, UIImage)

	/// Set properties
	void				SetImage(const UITexturedQuad &inImage)			{ mImage = inImage; }

	/// Cloning / copying
	virtual void		CopyTo(UIElement *ioElement) const override;

	/// Draw element
	virtual void		Draw() const override;

private:
	UITexturedQuad		mImage;
};
