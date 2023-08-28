// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <UI/UITexturedQuad.h>
#include <UI/UIButton.h>

/// Slider control with up/down button and thumb to select a value
class UISlider : public UIElement
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, UISlider)

	using ValueChangedAction = function<void(float)>;

	/// Properties
	void				SetValue(float inValue)								{ mCurrentValue = inValue; }
	void				SetRange(float inMin, float inMax, float inStep)	{ mMinValue = inMin; mMaxValue = inMax; mStepValue = inStep; }
	void				SetDecreaseButton(UIButton *inDecreaseButton)		{ mDecreaseButton = inDecreaseButton; }
	void				SetIncreaseButton(UIButton *inIncreaseButton)		{ mIncreaseButton = inIncreaseButton; }
	void				SetStaticText(UIStaticText *inStaticText)			{ mStaticText = inStaticText; UpdateStaticText(); }
	void				SetSlider(const UITexturedQuad &inSlider)			{ mSlider = inSlider; }
	void				SetThumb(const UITexturedQuad &inThumb)				{ mThumb = inThumb; }
	void				SetValueChangedAction(ValueChangedAction inAction)	{ mValueChangedAction = inAction; }

	/// Cloning / copying
	virtual void		CopyTo(UIElement *ioElement) const override;

	/// Actions
	virtual bool		MouseDown(int inX, int inY) override;
	virtual bool		MouseUp(int inX, int inY) override;
	virtual bool		MouseMove(int inX, int inY) override;
	virtual void		MouseCancel() override;

	/// Draw element
	virtual void		Draw() const override;

	/// Event handling (returns true if the event has been handled)
	virtual bool		HandleUIEvent(EUIEvent inEvent, UIElement *inSender) override;

	/// Calculate auto layout
	virtual void		AutoLayout() override;

protected:
	/// Pixel range of slider relative to parent
	void				GetSliderRange(int &outSliderStart, int &outSliderEnd) const;

	/// X coordinate of thumb start
	int					GetThumbStart(int inSliderStart, int inSliderEnd) const;

	/// Internal function to update the value
	void				SetValueInternal(float inValue);

	/// Update static text box
	void				UpdateStaticText();

	/// Properties
	float				mCurrentValue = 0.0f;
	float				mMinValue = 0.0f;
	float				mMaxValue = 1.0f;
	float				mStepValue = 0.1f;
	UIButton *			mDecreaseButton = nullptr;
	UIButton *			mIncreaseButton = nullptr;
	UIStaticText *		mStaticText = nullptr;
	int					mSpaceBetweenButtonAndSlider = 5;
	UITexturedQuad		mSlider;
	UITexturedQuad		mThumb;
	ValueChangedAction	mValueChangedAction;

	/// State
	int					mThumbDragPoint = -1;
};
