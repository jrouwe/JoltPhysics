// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <UI/UIElement.h>
#include <Renderer/Font.h>

/// Static text string
class UIStaticText : public UIElement
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, UIStaticText)

	/// Cloning / copying
	virtual void		CopyTo(UIElement *ioElement) const override;

	/// Set properties
	void				SetTextColor(ColorArg inColor)				{ mTextColor = inColor; }
	void				SetDisabledTextColor(ColorArg inColor)		{ mDisabledTextColor = inColor; }
	void				SetFont(const Font *inFont)					{ mFont = inFont; }
	void				SetText(const string_view &inText)			{ mText = inText; }
	void				SetTextPadding(int inTop, int inLeft, int inBottom, int inRight) { mTextPadTop = inTop; mTextPadLeft = inLeft; mTextPadBottom = inBottom; mTextPadRight = inRight; }
	void				SetTextAlignment(EAlignment inAlignment)	{ JPH_ASSERT(inAlignment == LEFT || inAlignment == RIGHT || inAlignment == CENTER); mTextAlignment = inAlignment; }
	void				SetWrap(bool inWrap)						{ mWrap = inWrap; }

	/// Draw element
	virtual void		Draw() const override;

	/// Calculate auto layout
	virtual void		AutoLayout() override;

protected:
	/// Draw element custom
	void				DrawCustom(ColorArg inColor) const;

	String				GetWrappedText() const;

	RefConst<Font>		mFont;
	String				mText;
	Color				mTextColor { Color(220, 220, 200) };
	Color				mDisabledTextColor { Color::sGrey };
	int					mTextPadLeft = 0;
	int					mTextPadRight = 0;
	int					mTextPadTop = 0;
	int					mTextPadBottom = 0;
	EAlignment			mTextAlignment = LEFT;
	bool				mWrap = false;
};
