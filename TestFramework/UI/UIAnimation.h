// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/RTTI.h>

class UIElement;

/// Base class for UI element animations
class UIAnimation
{
public:
	JPH_DECLARE_RTTI_VIRTUAL_BASE(JPH_NO_EXPORT, UIAnimation)

	/// Destructor
	virtual			~UIAnimation() = default;

	///@name Interface
	virtual void	Init(UIElement *inElement)							{ }
	virtual bool	Update(UIElement *inElement, float inDeltaTime)		{ return true; } ///< Returns false when done
	virtual void	Exit(UIElement *inElement)							{ }
};

using UIAnimationVector = Array<UIAnimation *>;
