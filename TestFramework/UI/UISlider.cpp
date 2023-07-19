// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <UI/UISlider.h>
#include <UI/UIManager.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(UISlider)
{
	JPH_ADD_BASE_CLASS(UISlider, UIElement)
}

void UISlider::CopyTo(UIElement *ioElement) const
{
	UIElement::CopyTo(ioElement);

	UISlider *element = StaticCast<UISlider>(ioElement);
	element->mCurrentValue = mCurrentValue;
	element->mMinValue = mMinValue;
	element->mMaxValue = mMaxValue;
	element->mStepValue = mStepValue;
	element->mDecreaseButton = mDecreaseButton;
	element->mIncreaseButton = mIncreaseButton;
	element->mSpaceBetweenButtonAndSlider = mSpaceBetweenButtonAndSlider;
	element->mSlider = mSlider;
	element->mThumb = mThumb;
	element->mValueChangedAction = mValueChangedAction;
}

void UISlider::GetSliderRange(int &outSliderStart, int &outSliderEnd) const
{
	outSliderStart = GetX() + mDecreaseButton->GetWidth() + mSpaceBetweenButtonAndSlider;
	outSliderEnd = GetX() + GetWidth() - mIncreaseButton->GetWidth() - mSpaceBetweenButtonAndSlider;
}

int UISlider::GetThumbStart(int inSliderStart, int inSliderEnd) const
{
	return inSliderStart + int(float(inSliderEnd - inSliderStart - mThumb.mWidth) * (mCurrentValue - mMinValue) / (mMaxValue - mMinValue));
}

bool UISlider::HandleUIEvent(EUIEvent inEvent, UIElement *inSender)
{
	if (inEvent == EVENT_BUTTON_DOWN)
	{
		if (inSender == mDecreaseButton)
		{
			SetValueInternal(mCurrentValue - mStepValue);
			return true;
		}
		else if (inSender == mIncreaseButton)
		{
			SetValueInternal(mCurrentValue + mStepValue);
			return true;
		}
	}

	return UIElement::HandleUIEvent(inEvent, inSender);
}

bool UISlider::MouseDown(int inX, int inY)
{
	if (Contains(inX, inY))
	{
		int slider_start, slider_end;
		GetSliderRange(slider_start, slider_end);

		int tx = GetThumbStart(slider_start, slider_end);
		if (inX >= tx && inX < tx + mThumb.mWidth)
		{
			mThumbDragPoint = inX - tx;
			return true;
		}
	}

	return UIElement::MouseDown(inX, inY);
}

bool UISlider::MouseUp(int inX, int inY)
{
	if (mThumbDragPoint != -1)
	{
		mThumbDragPoint = -1;
		return true;
	}

	return UIElement::MouseUp(inX, inY);
}

bool UISlider::MouseMove(int inX, int inY)
{
	if (mThumbDragPoint != -1)
	{
		// Calculate new value
		int slider_start, slider_end;
		GetSliderRange(slider_start, slider_end);
		SetValueInternal(mMinValue + float(inX - mThumbDragPoint - slider_start) * (mMaxValue - mMinValue) / float(slider_end - slider_start - mThumb.mWidth));
		return true;
	}

	return UIElement::MouseMove(inX, inY);
}

void UISlider::MouseCancel()
{
	UIElement::MouseCancel();

	mThumbDragPoint = -1;
}

void UISlider::Draw() const
{
	UIElement::Draw();

	int slider_start, slider_end;
	GetSliderRange(slider_start, slider_end);

	// Draw slider
	int sy = (GetHeight() - mSlider.mHeight) / 2;
	GetManager()->DrawQuad(slider_start, GetY() + sy, slider_end - slider_start, mSlider.mHeight, mSlider, Color::sWhite);

	// Draw thumb
	int tx = GetThumbStart(slider_start, slider_end);
	int ty = (GetHeight() - mThumb.mHeight) / 2;
	GetManager()->DrawQuad(tx, GetY() + ty, mThumb.mWidth, mThumb.mHeight, mThumb, Color::sWhite);
}

void UISlider::AutoLayout()
{
	UIElement::AutoLayout();

	// Position decrease button
	mDecreaseButton->SetRelativeX(0);
	mDecreaseButton->SetRelativeY((GetHeight() - mDecreaseButton->GetHeight()) / 2);

	// Position increase button
	mIncreaseButton->SetRelativeX(GetWidth() - mIncreaseButton->GetWidth());
	mIncreaseButton->SetRelativeY((GetHeight() - mIncreaseButton->GetHeight()) / 2);
}

void UISlider::SetValueInternal(float inValue)
{
	float old_value = mCurrentValue;

	float step = round((inValue - mMinValue) / mStepValue);
	mCurrentValue = Clamp(mMinValue + step * mStepValue, mMinValue, mMaxValue);

	if (mCurrentValue != old_value)
	{
		if (mValueChangedAction)
			mValueChangedAction(mCurrentValue);

		UpdateStaticText();
	}
}

void UISlider::UpdateStaticText()
{
	if (mStaticText != nullptr)
	{
		float step_frac = mStepValue - trunc(mStepValue);
		float min_frac = mMinValue - trunc(mMinValue);
		float max_frac = mMaxValue - trunc(mMaxValue);

		float smallest = step_frac;
		if (min_frac < smallest && abs(min_frac) > 1.0e-6f)
			smallest = min_frac;
		if (max_frac < smallest && abs(max_frac) > 1.0e-6f)
			smallest = max_frac;

		if (smallest == 0.0f)
		{
			// Integer values
			mStaticText->SetText(ConvertToString(int(round(mCurrentValue))));
		}
		else
		{
			int num_digits = -int(floor(log10(smallest)));
			stringstream ss;
			ss.precision(num_digits);
			ss << fixed << mCurrentValue;
			mStaticText->SetText(ss.str());
		}
	}
}
