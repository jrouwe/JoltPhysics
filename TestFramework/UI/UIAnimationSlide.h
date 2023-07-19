// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <UI/UIAnimation.h>

/// Animation that slides an element on or off screen
class UIAnimationSlide : public UIAnimation
{
public:
	JPH_DECLARE_RTTI_ABSTRACT(JPH_NO_EXPORT, UIAnimationSlide)

	/// Mode of sliding
	enum EMode
	{
		SLIDE_ON_SCREEN,
		SLIDE_OFF_SCREEN,
	};

	/// Constructor
					UIAnimationSlide(EMode inMode, int inSlideDistanceH, int inSlideDistanceV, float inTimeBeforeSlide, float inSlideTime);

	///@name Interface
	virtual void	Init(UIElement *inElement) override;
	virtual bool	Update(UIElement *inElement, float inDeltaTime) override;
	virtual void	Exit(UIElement *inElement) override;

private:
	EMode			mSlideMode;
	int				mSlideDistanceH;
	int				mSlideDistanceV;
	float			mTimeBeforeSlide;
	float			mSlideTime;
	int				mInitialRelativeX;
	int				mInitialRelativeY;
	int				mTargetRelativeX;
	int				mTargetRelativeY;
	float			mTime;
};
