// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Constraints/SliderConstraint.h>

class PoweredSliderConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(PoweredSliderConstraintTest)

	// See: Test
	virtual void			Initialize() override;
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	inline static float		sMaxMotorAcceleration = 250.0f;
	inline static float		sMaxFrictionAcceleration = 0.0f;
	inline static float		sFrequency = 2.0f;
	inline static float		sDamping = 1.0f;

	Body *					mBody2 = nullptr;
	SliderConstraint *		mConstraint = nullptr;
};
