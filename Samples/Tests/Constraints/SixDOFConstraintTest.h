// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Constraints/SixDOFConstraint.h>

class SixDOFConstraintTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(SixDOFConstraintTest)

	virtual void			Initialize() override;

	virtual void			GetInitialCamera(CameraState &ioState) const override;

	virtual bool			HasSettingsMenu() const override							{ return true; }
	virtual void			CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu) override;

private:
	using SettingsRef = Ref<SixDOFConstraintSettings>;
	using EAxis = SixDOFConstraintSettings::EAxis;

	static float			sLimitMin[EAxis::Num];
	static float			sLimitMax[EAxis::Num];
	static bool				sEnableLimits[EAxis::Num];
	static SettingsRef		sSettings;

	Vec3					mTargetOrientationCS = Vec3::sZero();

	Ref<SixDOFConstraint>	mConstraint;
};
