// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Reference.h>
#include <UI/UICheckBox.h>
#include <UI/UITextButton.h>
#include <UI/UISlider.h>
#include <UI/UIComboBox.h>
#include <UI/UIStaticText.h>

class UIManager;
class Texture;
class Font;
class Keyboard;

class DebugUI
{
public:
	/// Constructor
						DebugUI(UIManager *inUIManager, const Font *inFont);

	/// Create a new (sub) menu
	UIElement *			CreateMenu();

	/// Add items to the menu
	UIStaticText *		CreateStaticText(UIElement *inMenu, const string_view &inText);
	UITextButton *		CreateTextButton(UIElement *inMenu, const string_view &inName, UITextButton::ClickAction inAction);
	UICheckBox *		CreateCheckBox(UIElement *inMenu, const string_view &inName, bool inInitiallyChecked, UICheckBox::ClickAction inAction);
	UISlider *			CreateSlider(UIElement *inMenu, const string_view &inName, float inInitialValue, float inMinValue, float inMaxValue, float inStepValue, UISlider::ValueChangedAction inAction);
	UIComboBox *		CreateComboBox(UIElement *inMenu, const string_view &inName, const Array<String> &inItems, int inInitialItem, UIComboBox::ItemChangedAction inAction);

	/// Show it
	void				ShowMenu(UIElement *inMenu);

	/// Go back to the main menu
	void				BackToMain();

	/// Show or hide the entire menu
	void				ToggleVisibility();

private:
	UIManager *			mUI;
	RefConst<Font>		mFont;
	RefConst<Texture>	mUITexture;
};
