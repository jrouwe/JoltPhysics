// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>

class PoweredHingeConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(PoweredHingeConstraintTest)

	virtual void			Initialize() override;
	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	inline static float		sMaxAngularAcceleration = DegreesToRadians(3600.0f);
	inline static float		sMaxFrictionAngularAcceleration = 0.0f;
	inline static float		sFrequency = 2.0f;
	inline static float		sDamping = 1.0f;

	HingeConstraint *		mConstraint = nullptr;
	float					mInertiaBody2AsSeenFromConstraint;
};
