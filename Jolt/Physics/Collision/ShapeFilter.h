// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Body/BodyID.h>
#include <Jolt/Core/NonCopyable.h>

JPH_NAMESPACE_BEGIN

class SubShapeID;

/// Filter class
class ShapeFilter : public NonCopyable
{
public:
	/// Destructor
	virtual					~ShapeFilter() = default;

	/// Filter function to determine if two shapes should collide. Returns true if the filter passes.
	virtual bool			ShouldCollide(const SubShapeID &inSubShapeID1, const SubShapeID &inSubShapeID2) const
	{
		return true;
	}

	/// Set by the collision detection functions to the body ID of the body that we're colliding against before calling the ShouldCollide function
	mutable BodyID			mBodyID2;
};

JPH_NAMESPACE_END
