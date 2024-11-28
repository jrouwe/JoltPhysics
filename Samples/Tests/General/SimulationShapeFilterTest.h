// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test shows how to use a shape filter during the simulation to disable contacts between certain sub shapes
class SimulationShapeFilterTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SimulationShapeFilterTest)

	// Destructor
	virtual				~SimulationShapeFilterTest() override;

	// See: Test
	virtual void		Initialize() override;

private:
	// A demo of the shape filter
	class Filter : public ShapeFilter
	{
	public:
		virtual bool	ShouldCollide(const Shape *inShape1, const SubShapeID &inSubShapeIDOfShape1, const Shape *inShape2, const SubShapeID &inSubShapeIDOfShape2) const override;

		// We're not interested in the other overload as it is only used by collision queries and not by the simulation
		using ShapeFilter::ShouldCollide;

		BodyID			mPlatformID;
		BodyID			mCompoundID;
	};

	Filter				mShapeFilter;
};
