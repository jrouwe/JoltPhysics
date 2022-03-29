// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Constraints/PathConstraint.h>

class PathConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(PathConstraintTest)

	virtual void						Initialize() override;

	virtual void						PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	virtual bool						HasSettingsMenu() const override							{ return true; }
	virtual void						CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	Ref<PathConstraintPath>				mPaths[2];
	Ref<PathConstraint>					mConstraints[2];

	inline static float					sMaxMotorAcceleration = 20.0f;
	inline static float					sMaxFrictionAcceleration = 0.0f;
	inline static float					sFrequency = 2.0f;
	inline static float					sDamping = 1.0f;
	static EPathRotationConstraintType	sOrientationType;					///< The orientation constraint type for the path constraint
};
