// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Core/NonCopyable.h>

JPH_NAMESPACE_BEGIN

class Shape;
class SubShapeID;

/// Filter class
class ShapeFilter : public NonCopyable
{
public:
	/// Destructor
	virtual					~ShapeFilter() = default;

	/// Filter function to determine if we should collide with a shape. Returns true if the filter passes.
	/// This overload is called when the query doesn't have a source shape (e.g. ray cast / collide point)
	virtual bool			ShouldCollide(const SubShapeID &inSubShapeID2) const
	{
		return true;
	}

	/// Filter function to determine if two shapes should collide. Returns true if the filter passes.
	/// This overload is called when querying a shape vs a shape (e.g. collide object / cast object).
	/// It is called at each level of the shape hierarchy, so if you have a compound shape with a box, this function will be called twice.
	/// @param inShape1 1st shape that is colliding
	/// @param inSubShapeID1 Sub shape ID of the 1st shape (note that this is the sub shape ID from the top of the hierarchy,
	/// so unless inShape1 happens to be the top it will not be valid for that shape)
	/// @param inShape2 2nd shape that is colliding
	/// @param inSubShapeID2 Sub shape ID of the 2nd shape (note that this is the sub shape ID from the top of the hierarchy,
	/// so unless inShape2 happens to be the top it will not be valid for that shape, use mBodyID2 to get the top of the hierarchy shape)
	virtual bool			ShouldCollide(const Shape *inShape1, const SubShapeID &inSubShapeID1, const Shape *inShape2, const SubShapeID &inSubShapeID2) const
	{
		return true;
	}

	/// Set by the collision detection functions to the body ID of the body that we're colliding against before calling the ShouldCollide function
	mutable BodyID			mBodyID2;
};

JPH_NAMESPACE_END
