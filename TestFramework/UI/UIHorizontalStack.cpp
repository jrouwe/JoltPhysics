// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <UI/UIHorizontalStack.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(UIHorizontalStack)
{
	JPH_ADD_BASE_CLASS(UIHorizontalStack, UIElement)
}

void UIHorizontalStack::sUniformChildWidth(UIElement *inParent)
{
	Array<int> sizes;
	sizes.resize(1);
	for (UIElement *e : inParent->GetChildren())
	{
		e->AutoLayout();

		UIHorizontalStack *horiz = DynamicCast<UIHorizontalStack>(e);
		if (horiz != nullptr)
		{
			if (horiz->GetNumChildren() > (int)sizes.size())
				sizes.resize(horiz->GetNumChildren());
			for (int i = 0; i < horiz->GetNumChildren(); ++i)
				sizes[i] = max(sizes[i], horiz->GetChild(i)->GetWidth());
		}
		else
		{
			sizes[0] = max(sizes[0], e->GetWidth());
		}
	}

	for (UIElement *e : inParent->GetChildren())
	{
		UIHorizontalStack *horiz = DynamicCast<UIHorizontalStack>(e);
		if (horiz != nullptr)
		{
			for (int i = 0; i < horiz->GetNumChildren(); ++i)
				horiz->GetChild(i)->SetWidth(sizes[i]);
		}
		else
		{
			e->SetWidth(sizes[0]);
		}
	}
}

void UIHorizontalStack::AutoLayout()
{
	UIElement::AutoLayout();

	mWidth.Set(0, PIXELS);
	for (UIElement *e : mChildren)
		if (e->IsVisible() || mPlaceInvisibleChildren)
		{
			e->SetRelativeX(GetWidth());
			mWidth.Set(GetWidth() + e->GetWidth() + e->GetPaddingRight() + mDeltaX, PIXELS);
		}
}
