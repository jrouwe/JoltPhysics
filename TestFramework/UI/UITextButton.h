// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <UI/UIStaticText.h>

/// Clickable text button
class UITextButton : public UIStaticText
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(UITextButton)

	using ClickAction = function<void()>;

	/// Properties
	void				SetDownColor(ColorArg inColor)							{ mDownTextColor = inColor; }
	void				SetHighlightColor(ColorArg inColor)						{ mHighlightTextColor = inColor; }
	void				SetSelectedColor(ColorArg inColor)						{ mSelectedTextColor = inColor; }
	void				SetRepeat(float inRepeatStartTime, float inRepeatTime)	{ mRepeatStartTime = inRepeatStartTime; mRepeatTime = inRepeatTime; }
	void				SetClickAction(ClickAction inAction)					{ mClickAction = inAction; }

	/// Cloning / copying
	virtual void		CopyTo(UIElement *ioElement) const override;

	/// Actions
	virtual bool		MouseDown(int inX, int inY) override;
	virtual bool		MouseUp(int inX, int inY) override;
	virtual bool		MouseMove(int inX, int inY) override;
	virtual void		MouseCancel() override;

	/// Update element
	virtual void		Update(float inDeltaTime) override;

	/// Draw element
	virtual void		Draw() const override;

protected:
	/// Draw element custom
	void				DrawCustom() const;

	/// Properties
	Color				mDownTextColor { Color::sGrey };
	Color				mHighlightTextColor { Color::sWhite };
	Color				mSelectedTextColor { Color::sWhite };
	float				mRepeatStartTime = -1.0f;
	float				mRepeatTime = 0.5f;
	ClickAction			mClickAction;

	/// State
	bool				mPressed = false;
	bool				mIsRepeating = false;
	float				mRepeatTimeLeft = 0;
};
