// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test loads a physics scene from 'snapshot.bin' and runs it
class LoadSnapshotTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, LoadSnapshotTest)

	// See: Test
	virtual void		Initialize() override;

	// Optional settings menu
	virtual bool		HasSettingsMenu() const override							{ return true; }
	virtual void		CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	inline static bool	sOverrideLayers = false;
	inline static int	sUpAxis = 1;				// 0 = x, 1 = y, 2 = z
};
