// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/Renderer.h>
#include <UI/UIButton.h>
#include <UI/UIManager.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(UIButton)
{
	JPH_ADD_BASE_CLASS(UIButton, UITextButton)
}

void UIButton::CopyTo(UIElement *ioElement) const
{
	UITextButton::CopyTo(ioElement);

	UIButton *element = StaticCast<UIButton>(ioElement);
	element->mUpQuad = mUpQuad;
	element->mUpColor = mUpColor;
	element->mDownQuad = mDownQuad;
	element->mDownColor = mDownColor;
	element->mSelectedQuad = mSelectedQuad;
	element->mSelectedColor = mSelectedColor;
	element->mDisabledQuad = mDisabledQuad;
	element->mDisabledColor = mDisabledColor;
}

void UIButton::Draw() const
{
	if (mUpQuad.mTexture != nullptr)
	{
		int x = GetX(), y = GetY();
		const UITexturedQuad &q = IsDisabled()? mDisabledQuad : (mPressed? mDownQuad : (mIsHighlighted? mHighlightQuad : mUpQuad));
		Color c = IsDisabled()? mDisabledColor : (mPressed? mDownColor : (mIsHighlighted? mHighlightColor : mUpColor));
		int ew = GetWidth();
		int eh = GetHeight();

		if (!q.HasInnerPart())
		{
			// Center image in button if it is smaller than the button
			int w = min(ew, q.mWidth);
			int h = min(eh, q.mHeight);
			int x2 = x + (ew - w) / 2;
			int y2 = y + (eh - h) / 2;
			GetManager()->DrawQuad(x2, y2, w, h, q, c);
		}
		else
		{
			// This is a scale-9 quad, it will scale itself
			GetManager()->DrawQuad(x, y, ew, eh, q, c);
		}

		// Draw selected quad
		if (mIsSelected)
			GetManager()->DrawQuad(x, y, ew, eh, mSelectedQuad, mSelectedColor);
	}

	DrawCustom();

	// Skip direct base classes, we modify text color
	UIElement::Draw();
}

void UIButton::SetButtonQuad(const UITexturedQuad &inQuad)
{
	mUpQuad = inQuad;
	mDownQuad = inQuad;
	mHighlightQuad = inQuad;
	mDisabledQuad = inQuad;

	if (GetWidth() <= 0)
		SetWidth(inQuad.mWidth);
	if (GetHeight() <= 0)
		SetHeight(inQuad.mHeight);
}
