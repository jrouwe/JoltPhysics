// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <UI/UITexturedQuad.h>
#include <UI/UIButton.h>

/// Combo box with previous and next button
class UIComboBox : public UIElement
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, UIComboBox)

	using ItemChangedAction = function<void(int)>;

	/// Properties
	void				SetItems(const Array<String> &inItems)				{ mItems = inItems; }
	void				SetCurrentItem(int inItem)							{ mCurrentItem = inItem; }
	void				SetPreviousButton(UIButton *inPreviousButton)		{ mPreviousButton = inPreviousButton; }
	void				SetNextButton(UIButton *inNextButton)				{ mNextButton = inNextButton; }
	void				SetStaticText(UIStaticText *inStaticText)			{ mStaticText = inStaticText; UpdateStaticText(); }
	void				SetItemChangedAction(ItemChangedAction inAction)	{ mItemChangedAction = inAction; }

	/// Cloning / copying
	virtual void		CopyTo(UIElement *ioElement) const override;

	/// Event handling (returns true if the event has been handled)
	virtual bool		HandleUIEvent(EUIEvent inEvent, UIElement *inSender) override;

	/// Calculate auto layout
	virtual void		AutoLayout() override;

protected:
	/// Internal function to update the current item
	void				SetItemInternal(int inItem);

	/// Update static text box
	void				UpdateStaticText();

	/// Properties
	Array<String>		mItems;
	int					mCurrentItem = 0;
	UIButton *			mPreviousButton = nullptr;
	UIButton *			mNextButton = nullptr;
	UIStaticText *		mStaticText = nullptr;
	ItemChangedAction	mItemChangedAction;
};
