// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This is the base class for vehicle tests, it will create some sample geometry
class VehicleTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(VehicleTest)

	// See: Test
	virtual void			Initialize() override;
 
	// Optional settings menu
	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	// List of possible scene names
	static const char *		sScenes[];

	// Filename of animation to load for this test
	static const char *		sSceneName;

	void					CreateBridge();
	void					CreateWall();
};
