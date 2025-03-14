// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Collision/SimShapeFilter.h>

class SimShapeFilterTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SimShapeFilterTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return	"Shows how to use a shape filter during the simulation to disable contacts between certain sub shapes.\n"
				"The rod and sphere of the dynamic bodies only collide with the floor.";
	}

	// Destructor
	virtual				~SimShapeFilterTest() override;

	// See: Test
	virtual void		Initialize() override;

private:
	// A simulation shape filter
	class Filter : public SimShapeFilter
	{
	public:
		virtual bool	ShouldCollide(const Body &inBody1, const Shape *inShape1, const SubShapeID &inSubShapeIDOfShape1, const Body &inBody2, const Shape *inShape2, const SubShapeID &inSubShapeIDOfShape2) const override;

		BodyID			mPlatformID;
		BodyID			mClothID;
		BodyID			mCompoundID[2];
	};

	Filter				mShapeFilter;
};
