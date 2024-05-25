// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <UI/UIElement.h>

/// Layout class that will horizontally place elements next to each other according to their widths
class UIHorizontalStack : public UIElement
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, UIHorizontalStack)

	/// Helper function to resize a list of child elements consisting of UIHorizontalStack's to make them the same width.
	/// Can be used to give them the appearance of a table.
	/// Find the width of all UIHorizontalStack child elements in inParent and use the maximum width for all of them
	/// Non UIHorizontalStack elements will be treated as a UIHorizontalStack with only 1 element inside
	static void			sUniformChildWidth(UIElement *inParent);

	/// Calculate auto layout
	virtual void		AutoLayout() override;

private:
	int					mDeltaX = 0;
	bool				mPlaceInvisibleChildren = false;
};
