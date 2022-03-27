// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/RTTI.h>
#include <Jolt/Core/Color.h>
#include <UI/UIEventListener.h>

class UIManager;
class UIElement;
class UIAnimation;

using UIElementVector = vector<UIElement *>;
using UIAnimationVector = vector<UIAnimation *>;

/// Base class UI element. Forms a tree of UI elements.
class UIElement : public UIEventListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL_BASE(UIElement)

	/// Constructor
						UIElement();
	virtual				~UIElement() override;

	/// Add / remove child elements
	void				Add(UIElement *inElement);
	void				Clear();
	virtual void		OnAdded()									{ }
	
	/// Start / stop animations
	void				StartAnimation(UIAnimation *inAnimation);
	void				StopAnimation(const RTTI *inAnimationType);

	/// Cloning / copying
	UIElement *			Clone() const;
	virtual void		CopyTo(UIElement *ioElement) const;
	
	/// Units
	enum EUnits
	{
		PIXELS,
		PERCENTAGE,
	};
	
	/// Alignment
	enum EAlignment
	{
		LEFT,
		ONE_THIRD,
		CENTER,
		RIGHT
	};

	/// Properties
	int					GetID() const								{ return mID; }
	void				SetID(int inID)								{ mID = inID; }
	int					GetX() const								{ return GetRelativeX() + (mParent != nullptr? mParent->GetX() : 0); }
	int					GetY() const								{ return GetRelativeY() + (mParent != nullptr? mParent->GetY() : 0); }
	int					GetRelativeX() const						{ return mRelativeX.GetPosition(this, &UIElement::GetWidth); }
	void				SetRelativeX(int inX, EUnits inUnits = PIXELS, EAlignment inAlignment = LEFT) { mRelativeX.Set(inX, inUnits, inAlignment); }
	int					GetRelativeY() const						{ return mRelativeY.GetPosition(this, &UIElement::GetHeight); }
	void				SetRelativeY(int inY, EUnits inUnits = PIXELS, EAlignment inAlignment = LEFT) { mRelativeY.Set(inY, inUnits, inAlignment); }
	int					GetWidth() const							{ return mWidth.GetSize(this, &UIElement::GetWidth); }
	void				SetWidth(int inWidth, EUnits inUnits = PIXELS) { mWidth.Set(inWidth, inUnits); } 
	int					GetHeight() const							{ return mHeight.GetSize(this, &UIElement::GetHeight); }
	void				SetHeight(int inHeight, EUnits inUnits = PIXELS) { mHeight.Set(inHeight, inUnits); } 
	int					GetPaddingRight() const						{ return mPaddingRight.GetSize(this, &UIElement::GetWidth); }
	void				SetPaddingRight(int inY, EUnits inUnits = PIXELS) { mPaddingRight.Set(inY, inUnits); }
	int					GetPaddingBottom() const					{ return mPaddingBottom.GetSize(this, &UIElement::GetHeight); }
	void				SetPaddingBottom(int inY, EUnits inUnits = PIXELS) { mPaddingBottom.Set(inY, inUnits); }
	void				SetVisible(bool inShow)						{ mIsVisible = inShow; }
	bool				IsVisible() const							{ return mIsVisible && mAnimatedIsVisible; }
	void				SetDisabled(bool inDisabled);
	bool				IsDisabled() const							{ return mIsDisabled; }
	void				SetHighlighted(bool inHighlighted);
	bool				IsHighlighted() const						{ return mIsHighlighted; }
	void				SetSelected(bool inSelected);
	bool				IsSelected() const							{ return mIsSelected; }

	/// Animation
	void				SetAnimatedVisible(bool inShow)				{ mAnimatedIsVisible = inShow; } ///< Visibility flag that can be set by UIAnimations
	bool				HasActivateAnimation() const				{ return mHasActivateAnimation; }
	bool				HasDeactivateAnimation() const				{ return mHasDeactivateAnimation; }

	/// Manager
	UIManager *			GetManager() const							{ return mManager; }

	/// Parent child linking
	UIElement *			GetParent() const							{ return mParent; }
	int					GetNumChildren() const						{ return (int)mChildren.size(); }
	UIElement *			GetChild(int inIdx) const					{ return mChildren[inIdx]; }
	const UIElementVector &GetChildren() const						{ return mChildren; }

	/// Hit testing
	bool				Contains(int inX, int inY) const;
	bool				ContainsWidened(int inX, int inY, int inBorder) const;

	/// Calculate auto layout
	virtual void		AutoLayout();

	/// Find element by ID
	virtual UIElement *	FindByID(int inID);

	/// Update element
	virtual void		Update(float inDeltaTime);

	/// Draw element
	virtual void		Draw() const;

	/// Actions
	virtual bool		MouseDown(int inX, int inY);
	virtual bool		MouseUp(int inX, int inY);
	virtual bool		MouseMove(int inX, int inY);
	virtual void		MouseCancel();

	/// Event handling (returns true if the event has been handled)
	virtual bool		HandleUIEvent(EUIEvent inEvent, UIElement *inSender) override;

protected:
	/// ID
	int					mID;

	/// Hierarchy
	UIElement *			mParent;
	UIElementVector		mChildren;

	/// Abstract GetSize function
	using fGetSize = int (UIElement::*)() const;

	/// Size
	class Size
	{
	public:
		/// Constructor
						Size() : mSize(0), mUnit(PIXELS) { }

		/// Get size 
		int				GetSize(const UIElement *inElement, fGetSize inGetSize) const;

		/// Assignment
		void			Set(int inValue, EUnits inUnits);

	private:
		int				mSize;
		EUnits			mUnit;
	};

	/// Position
	class Position
	{
	public:
		/// Constructor
						Position() : mAlignment(LEFT) { }

		/// Get position
		int				GetPosition(const UIElement *inElement, fGetSize inGetSize) const;

		/// Assignment
		void			Set(int inValue, EUnits inUnits, EAlignment inAlignment);

	private:
		EAlignment		mAlignment;
		Size			mSize;
	};

	/// Position
	Position			mRelativeX;
	Position			mRelativeY;
	Size				mWidth;
	Size				mHeight;
	Size				mPaddingRight;
	Size				mPaddingBottom;
	bool				mIsVisible;
	bool				mAnimatedIsVisible;
	bool				mIsHighlighted;
	bool				mIsSelected;
	bool				mIsDisabled;

	/// Animations
	bool				mHasActivateAnimation;
	bool				mHasDeactivateAnimation;
	UIAnimationVector	mAnimations;

	/// Manager
	UIManager *			mManager;
};
