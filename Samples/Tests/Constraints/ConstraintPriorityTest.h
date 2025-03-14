// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Constraints/FixedConstraint.h>

class ConstraintPriorityTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ConstraintPriorityTest)

	// Description of the test
	virtual const char *		GetDescription() const override
	{
		return	"Tests constraint priority system to demonstrate that the order of solving can have an effect on the simulation.\n"
				"Solving the root first will make the system stiffer.";
	}

	// See: Test
	virtual void				Initialize() override;
	virtual void				PostPhysicsUpdate(float inDeltaTime) override;

private:
	Array<Ref<FixedConstraint>>	mConstraints;
};
