// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/Renderer.h>
#include <UI/UIAnimationSlide.h>
#include <UI/UIElement.h>
#include <UI/UIManager.h>

JPH_IMPLEMENT_RTTI_ABSTRACT(UIAnimationSlide)
{
	JPH_ADD_BASE_CLASS(UIAnimationSlide, UIAnimation)
}

UIAnimationSlide::UIAnimationSlide(EMode inMode, int inSlideDistanceH, int inSlideDistanceV, float inTimeBeforeSlide, float inSlideTime) :
	mSlideMode(inMode),
	mSlideDistanceH(inSlideDistanceH),
	mSlideDistanceV(inSlideDistanceV),
	mTimeBeforeSlide(inTimeBeforeSlide),
	mSlideTime(inSlideTime)
{
}

void UIAnimationSlide::Init(UIElement *inElement)
{
	mTargetRelativeX = inElement->GetRelativeX();
	mTargetRelativeY = inElement->GetRelativeY();

	Renderer *renderer = inElement->GetManager()->GetRenderer();
	int dl = inElement->GetX();
	int dr = renderer->GetWindowWidth() - (inElement->GetX() + inElement->GetWidth());
	int dt = inElement->GetY();
	int db = renderer->GetWindowHeight() - (inElement->GetY() + inElement->GetHeight());

	if (min(dl, dr) < min(dt, db)) 
	{
		mInitialRelativeX = mTargetRelativeX + (dl < dr? -mSlideDistanceH : mSlideDistanceH);
		mInitialRelativeY = mTargetRelativeY;
	}
	else
	{
		mInitialRelativeX = mTargetRelativeX;
		mInitialRelativeY = mTargetRelativeY + (dt < db? -mSlideDistanceV : mSlideDistanceV);
	}

	if (mSlideMode == SLIDE_ON_SCREEN)
		inElement->SetAnimatedVisible(true);

	mTime = 0.0f;
}

bool UIAnimationSlide::Update(UIElement *inElement, float inDeltaTime)
{
	mTime += inDeltaTime;
	
	float factor = (mTime - mTimeBeforeSlide) / mSlideTime;
	if (factor >= 1.0f)
		return false;
	if (factor < 0.0f) 
		factor = 0.0f;

	if (mSlideMode == SLIDE_OFF_SCREEN) 
		factor = 1.0f - factor;

	float x = mInitialRelativeX * (1.0f - factor) + mTargetRelativeX * factor;
	float y = mInitialRelativeY * (1.0f - factor) + mTargetRelativeY * factor;

	inElement->SetRelativeX((int)x);
	inElement->SetRelativeY((int)y);
	return true;
}

void UIAnimationSlide::Exit(UIElement *inElement)
{
	inElement->SetRelativeX(mTargetRelativeX);
	inElement->SetRelativeY(mTargetRelativeY);

	inElement->SetAnimatedVisible(mSlideMode == SLIDE_ON_SCREEN);
}
