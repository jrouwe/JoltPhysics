// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <UI/UIVerticalStack.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(UIVerticalStack)
{
	JPH_ADD_BASE_CLASS(UIVerticalStack, UIElement)
}

void UIVerticalStack::AutoLayout()
{
	UIElement::AutoLayout();

	mHeight.Set(0, PIXELS);
	for (UIElement *e : mChildren)
		if (e->IsVisible() || mPlaceInvisibleChildren)
		{
			e->SetRelativeY(GetHeight());
			mHeight.Set(GetHeight() + e->GetHeight() + e->GetPaddingBottom() + mDeltaY, PIXELS);
		}
}
