// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Collision/SimShapeFilter.h>

// This test shows how to use a shape filter during the simulation to disable contacts between certain sub shapes
class SimShapeFilterTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SimShapeFilterTest)

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
