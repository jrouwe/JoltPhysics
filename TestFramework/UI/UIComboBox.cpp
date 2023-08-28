// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <UI/UIComboBox.h>
#include <UI/UIManager.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(UIComboBox)
{
	JPH_ADD_BASE_CLASS(UIComboBox, UIElement)
}

void UIComboBox::CopyTo(UIElement *ioElement) const
{
	UIElement::CopyTo(ioElement);

	UIComboBox *element = StaticCast<UIComboBox>(ioElement);
	element->mCurrentItem = mCurrentItem;
	element->mItems = mItems;
	element->mPreviousButton = mPreviousButton;
	element->mNextButton = mNextButton;
	element->mStaticText = mStaticText;
	element->mItemChangedAction = mItemChangedAction;
}

bool UIComboBox::HandleUIEvent(EUIEvent inEvent, UIElement *inSender)
{
	if (inEvent == EVENT_BUTTON_DOWN)
	{
		if (inSender == mPreviousButton)
		{
			SetItemInternal(mCurrentItem - 1);
			return true;
		}
		else if (inSender == mNextButton)
		{
			SetItemInternal(mCurrentItem + 1);
			return true;
		}
	}

	return UIElement::HandleUIEvent(inEvent, inSender);
}

void UIComboBox::AutoLayout()
{
	UIElement::AutoLayout();

	// Position previous button
	mPreviousButton->SetRelativeX(0);
	mPreviousButton->SetRelativeY((GetHeight() - mPreviousButton->GetHeight()) / 2);

	// Position static text
	mStaticText->SetRelativeX((GetWidth() - mStaticText->GetWidth()) / 2);
	mStaticText->SetRelativeY((GetHeight() - mStaticText->GetHeight()) / 2);

	// Position next button
	mNextButton->SetRelativeX(GetWidth() - mNextButton->GetWidth());
	mNextButton->SetRelativeY((GetHeight() - mNextButton->GetHeight()) / 2);
}

void UIComboBox::SetItemInternal(int inItem)
{
	int old_item = mCurrentItem;

	if (inItem < 0)
		mCurrentItem = 0;
	else if (inItem > int(mItems.size()) - 1)
		mCurrentItem = int(mItems.size()) - 1;
	else
		mCurrentItem = inItem;

	if (mCurrentItem != old_item)
	{
		if (mItemChangedAction)
			mItemChangedAction(mCurrentItem);

		UpdateStaticText();
	}
}

void UIComboBox::UpdateStaticText()
{
	if (mStaticText != nullptr)
		mStaticText->SetText(mItems[mCurrentItem]);
}
