// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Renderer/Renderer.h>
#include <UI/UIImage.h>
#include <UI/UIManager.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(UIImage)
{
	JPH_ADD_BASE_CLASS(UIImage, UIElement)
}

void UIImage::Draw() const
{
	GetManager()->DrawQuad(GetX(), GetY(), GetWidth(), GetHeight(), mImage, Color::sWhite);

	UIElement::Draw();
}

void UIImage::CopyTo(UIElement *ioElement) const
{
	UIElement::CopyTo(ioElement);

	UIImage *element = StaticCast<UIImage>(ioElement);
	element->mImage = mImage;
}
