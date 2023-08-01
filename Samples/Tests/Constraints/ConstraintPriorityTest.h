// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Constraints/FixedConstraint.h>

// Tests constraint priority system to demonstrate that the order of solving can have an effect on the stiffness of the system
class ConstraintPriorityTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ConstraintPriorityTest)

	// See: Test
	virtual void				Initialize() override;
	virtual void				PostPhysicsUpdate(float inDeltaTime) override;

private:
	Array<Ref<FixedConstraint>>	mConstraints;
};
