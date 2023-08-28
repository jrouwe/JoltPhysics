// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <UI/UITextButton.h>
#include <UI/UIManager.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(UITextButton)
{
	JPH_ADD_BASE_CLASS(UITextButton, UIStaticText)
}

void UITextButton::CopyTo(UIElement *ioElement) const
{
	UIStaticText::CopyTo(ioElement);

	UITextButton *element = StaticCast<UITextButton>(ioElement);
	element->mDownTextColor = mDownTextColor;
	element->mHighlightTextColor = mHighlightTextColor;
	element->mSelectedTextColor = mSelectedTextColor;
	element->mRepeatStartTime = mRepeatStartTime;
	element->mRepeatTime = mRepeatTime;
	element->mClickAction = mClickAction;
}

bool UITextButton::MouseDown(int inX, int inY)
{
	if (UIStaticText::MouseDown(inX, inY))
		return true;

	if (Contains(inX, inY))
	{
		mPressed = true;
		mIsRepeating = false;
		mRepeatTimeLeft = mRepeatStartTime;
		return true;
	}

	return false;
}

bool UITextButton::MouseUp(int inX, int inY)
{
	if (UIStaticText::MouseUp(inX, inY))
		return true;

	if (mPressed)
	{
		mPressed = false;

		if (!mIsRepeating && Contains(inX, inY))
		{
			HandleUIEvent(EVENT_BUTTON_DOWN, this);

			if (mClickAction)
				mClickAction();
		}
		return true;
	}

	return false;
}

bool UITextButton::MouseMove(int inX, int inY)
{
	if (UIStaticText::MouseMove(inX, inY))
		return true;

	return mPressed;
}

void UITextButton::MouseCancel()
{
	UIStaticText::MouseCancel();

	mPressed = false;
}

void UITextButton::Update(float inDeltaTime)
{
	UIStaticText::Update(inDeltaTime);

	if (mPressed && mRepeatStartTime > 0)
	{
		// Check repeat
		mRepeatTimeLeft -= inDeltaTime;
		if (mRepeatTimeLeft <= 0.0f)
		{
			// We're repeating
			mIsRepeating = true;
			mRepeatTimeLeft = mRepeatTime;

			HandleUIEvent(EVENT_BUTTON_DOWN, this);

			if (mClickAction)
				mClickAction();
		}
	}
}

void UITextButton::DrawCustom() const
{
	UIStaticText::DrawCustom(IsDisabled()? mDisabledTextColor : (mPressed? mDownTextColor : (mIsHighlighted? mHighlightTextColor : (mIsSelected? mSelectedTextColor : mTextColor))));
}

void UITextButton::Draw() const
{
	DrawCustom();

	// Skip direct base class, we modify the text color
	UIElement::Draw();
}
