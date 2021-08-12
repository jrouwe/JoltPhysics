// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <UI/UITextButton.h>
#include <UI/UITexturedQuad.h>

/// Button with a background image and text on it
class UIButton : public UITextButton
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(UIButton)

	/// Cloning / copying
	virtual void		CopyTo(UIElement *ioElement) const override;

	/// Draw element
	virtual void		Draw() const override;

	/// Set quad
	void				SetButtonQuad(const UITexturedQuad &inQuad);

private:
	UITexturedQuad		mUpQuad;
	Color				mUpColor { Color(220, 220, 220) };
	UITexturedQuad		mDownQuad;
	Color				mDownColor { Color::sGrey };
	UITexturedQuad		mHighlightQuad;
	Color				mHighlightColor { Color::sWhite };
	UITexturedQuad		mSelectedQuad;	
	Color				mSelectedColor { Color::sWhite };
	UITexturedQuad		mDisabledQuad;	
	Color				mDisabledColor { Color::sGrey };
};
