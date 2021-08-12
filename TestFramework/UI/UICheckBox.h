// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <UI/UIStaticText.h>
#include <UI/UITexturedQuad.h>

/// Check box control that allows the user to select between true or false
class UICheckBox : public UIStaticText
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(UICheckBox)

	enum EState
	{
		STATE_UNCHECKED,
		STATE_CHECKED
	};

	using ClickAction = function<void(EState)>;
	
	/// Properties
	void				SetState(EState inState)							{ mState = inState; }
	EState				GetState() const									{ return mState; }
	void				SetClickAction(ClickAction inAction)				{ mClickAction = inAction; }
	void				SetUncheckedStateQuad(const UITexturedQuad &inQuad)	{ mUncheckedState = inQuad; }
	void				SetCheckedStateQuad(const UITexturedQuad &inQuad)	{ mCheckedState = inQuad; }

	/// When added to a parent
	virtual void		OnAdded() override;

	/// Cloning / copying
	virtual void		CopyTo(UIElement *ioElement) const override;

	/// Actions
	virtual bool		MouseDown(int inX, int inY) override;
	virtual bool		MouseUp(int inX, int inY) override;
	virtual bool		MouseMove(int inX, int inY) override;
	virtual void		MouseCancel() override;

	/// Draw element
	virtual void		Draw() const override;

protected:
	/// Properties
	Color				mDownTextColor { Color::sGrey };
	Color				mHighlightTextColor { Color::sWhite };
	int					mPaddingBetweenCheckboxAndText = 8;
	ClickAction			mClickAction;
	UITexturedQuad		mUncheckedState;
	UITexturedQuad		mCheckedState;

	/// State
	EState				mState = STATE_UNCHECKED;
	bool				mPressed = false;
};
