// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Constraints/SwingTwistConstraint.h>

class SwingTwistConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(SwingTwistConstraintTest)

	virtual void			Initialize() override;

	virtual void			PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	vector<Ref<SwingTwistConstraint>>	mConstraints;

	inline static float		sNormalHalfConeAngle = DegreesToRadians(60);
	inline static float		sPlaneHalfConeAngle = DegreesToRadians(20);
	inline static float		sTwistMinAngle = DegreesToRadians(-10);
	inline static float		sTwistMaxAngle = DegreesToRadians(20);
};
