// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/Renderer.h>
#include <UI/UIElement.h>
#include <UI/UIAnimation.h>

JPH_IMPLEMENT_RTTI_VIRTUAL_BASE(UIElement)
{
}

UIElement::UIElement() :
	mID(-1),
	mParent(nullptr),
	mIsVisible(true),
	mAnimatedIsVisible(true),
	mIsHighlighted(false),
	mIsSelected(false),
	mIsDisabled(false),
	mHasActivateAnimation(true),
	mHasDeactivateAnimation(true),
	mManager(nullptr)
{
}

UIElement::~UIElement()
{
	Clear();
}

void UIElement::Add(UIElement *inElement)
{
	inElement->mParent = this;
	inElement->mManager = mManager;
	mChildren.push_back(inElement);
	inElement->OnAdded();
}

void UIElement::Clear()
{
	for (UIAnimation *a : mAnimations)
		delete a;

	for (UIElement *e : mChildren)
		delete e;
}

void UIElement::StartAnimation(UIAnimation *inAnimation)
{
	mAnimations.push_back(inAnimation);

	inAnimation->Init(this);
	inAnimation->Update(this, 0.0f);
}

void UIElement::StopAnimation(const RTTI *inAnimationType)
{
	for (int i = (int)mAnimations.size() - 1; i >= 0; --i)
		if (mAnimations[i]->GetRTTI()->IsKindOf(inAnimationType))
		{
			mAnimations[i]->Exit(this);
			delete mAnimations[i];
			mAnimations.erase(mAnimations.begin() + i);
		}
}

UIElement *UIElement::Clone() const
{
	UIElement *element = reinterpret_cast<UIElement *>(GetRTTI()->CreateObject());
	CopyTo(element);
	return element;
}

void UIElement::SetHighlighted(bool inHighlighted)
{
	mIsHighlighted = inHighlighted; 

	for (UIElement *e : mChildren)
		e->SetHighlighted(inHighlighted);
}

void UIElement::SetSelected(bool inSelected)
{
	mIsSelected = inSelected; 

	for (UIElement *e : mChildren)
		e->SetSelected(inSelected);
}

void UIElement::SetDisabled(bool inDisabled)
{
	mIsDisabled = inDisabled; 

	for (UIElement *e : mChildren)
		e->SetDisabled(inDisabled);
}

void UIElement::CopyTo(UIElement *ioElement) const
{
	// Clone properties
	ioElement->mID = mID;
	ioElement->mRelativeX = mRelativeX;
	ioElement->mRelativeY = mRelativeY;
	ioElement->mWidth = mWidth;
	ioElement->mHeight = mHeight;
	ioElement->mIsVisible = mIsVisible;
	ioElement->mAnimatedIsVisible = mAnimatedIsVisible;
	ioElement->mHasActivateAnimation = mHasActivateAnimation;
	ioElement->mHasDeactivateAnimation = mHasDeactivateAnimation;
	ioElement->mManager = mManager;

	// Clone children
	for (const UIElement *e : mChildren)
		ioElement->Add(e->Clone());
}

bool UIElement::Contains(int inX, int inY) const
{
	int x = GetX(), y = GetY();
	return inX >= x && inX < x + GetWidth() && inY >= y && inY < y + GetHeight();
}

bool UIElement::ContainsWidened(int inX, int inY, int inBorder) const
{
	int x = GetX(), y = GetY();
	return inX >= x - inBorder && inX < x + GetWidth() + inBorder && inY >= y - inBorder && inY < y + GetHeight() + inBorder;
}

void UIElement::Update(float inDeltaTime)
{
	for (int i = 0; i < (int)mAnimations.size(); ++i)
	{
		UIAnimation *animation = mAnimations[i];
		if (!animation->Update(this, inDeltaTime))
		{
			animation->Exit(this);
			delete animation;
			mAnimations.erase(mAnimations.begin() + i, mAnimations.begin() + i + 1);
			--i;
		}
	}

	for (UIElement *e : mChildren)
		if (e->IsVisible())
			e->Update(inDeltaTime);
}

void UIElement::Draw() const
{
	for (const UIElement *e : mChildren)
		if (e->IsVisible())
			e->Draw();
}

bool UIElement::MouseDown(int inX, int inY)
{
	for (UIElement *e : mChildren)
		if (e->IsVisible() && !e->mIsDisabled)
			if (e->MouseDown(inX, inY))
				return true;
	return false;
}

bool UIElement::MouseUp(int inX, int inY)
{
	for (UIElement *e : mChildren)
		if (e->IsVisible() && !e->mIsDisabled)
			if (e->MouseUp(inX, inY))
				return true;
	return false;
}

bool UIElement::MouseMove(int inX, int inY)
{
	if (Contains(inX, inY))
		mIsHighlighted = true;
	else
		mIsHighlighted = false;

	for (UIElement *e : mChildren)
		if (e->IsVisible() && !e->mIsDisabled)
			if (e->MouseMove(inX, inY))
				return true;
	
	return false;
}

void UIElement::MouseCancel()
{
	for (UIElement *e : mChildren)
		if (e->IsVisible() && !e->mIsDisabled)
			e->MouseCancel();
}

UIElement *UIElement::FindByID(int inID)
{
	if (inID == mID)
		return this;

	for (UIElement *e : mChildren)
	{
		UIElement *element = e->FindByID(inID);
		if (element != nullptr)
			return element;
	}

	return nullptr;
}

void UIElement::AutoLayout()
{
	for (UIElement *e : mChildren)
	{
		// Recurse
		e->AutoLayout();

		// Encapsulate height and width of children
		if (e->IsVisible())
		{
			mWidth.Set(max(GetWidth(), e->GetX() + e->GetWidth() - GetX() + e->GetPaddingRight()), PIXELS);
			mHeight.Set(max(GetHeight(), e->GetY() + e->GetHeight() - GetY() + e->GetPaddingBottom()), PIXELS);
		}
	}
}

bool UIElement::HandleUIEvent(EUIEvent inEvent, UIElement *inSender)
{
	if (mParent != nullptr)
		return mParent->HandleUIEvent(inEvent, inSender);

	return false;
}

void UIElement::Size::Set(int inValue, EUnits inUnits)
{
	mUnit = inUnits;
	mSize = inValue;
}

int UIElement::Size::GetSize(const UIElement *inElement, fGetSize inGetSize) const
{
	switch (mUnit)
	{
	case PIXELS:
		return mSize;

	case PERCENTAGE:
		{
			const UIElement *parent = inElement->GetParent();
			if (parent != nullptr)
				return (mSize * (parent->*inGetSize)()) / 100;
			else
				return 0;
		}

	default:
		JPH_ASSERT(false);
		return 0;
	}
}

void UIElement::Position::Set(int inValue, EUnits inUnits, EAlignment inAlignment)
{
	mAlignment = inAlignment;
	mSize.Set(inValue, inUnits);
}

int UIElement::Position::GetPosition(const UIElement *inElement, fGetSize inGetSize) const
{
	int pos = mSize.GetSize(inElement, inGetSize);

	switch (mAlignment)
	{
	case LEFT:
		return pos;

	case ONE_THIRD:
		{
			const UIElement *parent = inElement->GetParent();
			if (parent != nullptr)
				return ((parent->*inGetSize)() - (inElement->*inGetSize)()) / 3 + pos;
			else
				return 0;
		}

	case CENTER:
		{
			const UIElement *parent = inElement->GetParent();
			if (parent != nullptr)
				return ((parent->*inGetSize)() - (inElement->*inGetSize)()) / 2 + pos;
			else
				return 0;
		}

	case RIGHT:
		{
			const UIElement *parent = inElement->GetParent();
			if (parent != nullptr)
				return (parent->*inGetSize)() - (inElement->*inGetSize)() + pos;
			else
				return 0;
		}

	default:
		JPH_ASSERT(false);
		return 0;
	}
}
