// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <UI/UIStaticText.h>
#include <Renderer/Renderer.h>
#include <Renderer/Font.h>
#include <UI/UIManager.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(UIStaticText)
{
	JPH_ADD_BASE_CLASS(UIStaticText, UIElement)
}

void UIStaticText::CopyTo(UIElement *ioElement) const
{
	UIElement::CopyTo(ioElement);

	UIStaticText *element = StaticCast<UIStaticText>(ioElement);
	element->mFont = mFont;
	element->mText = mText;
	element->mTextColor = mTextColor;
	element->mDisabledTextColor = mDisabledTextColor;
	element->mTextPadLeft = mTextPadLeft;
	element->mTextPadRight = mTextPadRight;
	element->mTextPadTop = mTextPadTop;
	element->mTextPadBottom = mTextPadBottom;
	element->mTextAlignment = mTextAlignment;
	element->mWrap = mWrap;
}

void UIStaticText::Draw() const
{
	DrawCustom(IsDisabled()? mDisabledTextColor : mTextColor);

	UIElement::Draw();
}

void UIStaticText::AutoLayout()
{
	UIElement::AutoLayout();

	// Update size
	if (mFont != nullptr)
	{
		Float2 size = mFont->MeasureText(GetWrappedText());
		int w = int(size.x * mFont->GetCharHeight()) + mTextPadLeft + mTextPadRight;
		int h = int(size.y * mFont->GetCharHeight()) + mTextPadTop + mTextPadBottom;
		if (GetWidth() <= 0)
			mWidth.Set(w, PIXELS);
		if (GetHeight() <= 0)
			mHeight.Set(h, PIXELS);
	}
}

String UIStaticText::GetWrappedText() const
{
	String text;

	if (mWrap)
	{
		int width = GetWidth() - mTextPadLeft - mTextPadRight;

		size_t start_pos = 0, prev_end_pos = size_t(-1);
		for (;;)
		{
			// Find next space or end of text
			size_t end_pos = mText.find(' ', prev_end_pos + 1);
			if (end_pos == String::npos)
				end_pos = mText.length();

			// Get line to test for width
			String sub = mText.substr(start_pos, end_pos - start_pos);

			// Measure width
			Float2 size = mFont->MeasureText(sub);
			int w = int(size.x * mFont->GetCharHeight());

			// Check if still fits
			if (width < w)
			{
				// If nothing was found yet, add the last word anyhow so that we don't get into an infinite loop
				if (start_pos >= prev_end_pos
					|| prev_end_pos == size_t(-1))
					prev_end_pos = end_pos;

				// Add current line
				text += mText.substr(start_pos, prev_end_pos - start_pos);
				text += "\n";

				// Start here for new line
				start_pos = prev_end_pos + 1;
			}
			else
			{
				// Store last fitting line
				prev_end_pos = end_pos;
			}

			// Check end
			if (end_pos >= mText.length())
				break;
		}

		// Add last line
		if (start_pos < mText.length())
			text += mText.substr(start_pos, mText.length() - start_pos);
	}
	else
	{
		// Don't autowrap
		text = mText;
	}

	return text;
}

void UIStaticText::DrawCustom(ColorArg inColor) const
{
	if (mFont != nullptr && !mText.empty())
	{
		String text = GetWrappedText();

		int y = GetY() + mTextPadTop;

		if (mTextAlignment == LEFT)
		{
			GetManager()->DrawText(GetX() + mTextPadLeft, y, text, mFont, inColor);
		}
		else if (mTextAlignment == CENTER)
		{
			// Split lines
			Array<String> lines;
			StringToVector(text, lines, "\n");

			// Amount of space we have horizontally
			int width = GetWidth() - mTextPadLeft - mTextPadRight;

			// Center each line individually
			for (const String &l : lines)
			{
				Float2 size = mFont->MeasureText(l);
				int w = int(size.x * mFont->GetCharHeight());
				GetManager()->DrawText(GetX() + (width - w) / 2 + mTextPadLeft, y, l, mFont, inColor);
				y += mFont->GetCharHeight();
			}
		}
		else
		{
			JPH_ASSERT(mTextAlignment == RIGHT);

			// Split lines
			Array<String> lines;
			StringToVector(text, lines, "\n");

			// Center each line individually
			for (const String &l : lines)
			{
				Float2 size = mFont->MeasureText(l);
				int w = int(size.x * mFont->GetCharHeight());
				GetManager()->DrawText(GetX() + GetWidth() - mTextPadRight - w, y, l, mFont, inColor);
				y += mFont->GetCharHeight();
			}
		}
	}
}
