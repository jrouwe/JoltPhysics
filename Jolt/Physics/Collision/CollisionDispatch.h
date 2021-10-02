// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Physics/Collision/Shape/Shape.h>
#include <Physics/Collision/Shape/SubShapeID.h>
#include <Physics/Collision/ShapeCast.h>
#include <Physics/Collision/ShapeFilter.h>

namespace JPH {

class CollideShapeSettings;

/// Dispatch function, main function to handle collisions between shapes
class CollisionDispatch
{
public:
	/// Collide 2 shapes and pass any collision on to ioCollector
	/// @param inShape1 The first shape
	/// @param inShape2 The second shape
	/// @param inScale1 Local space scale of shape 1
	/// @param inScale2 Local space scale of shape 2
	/// @param inCenterOfMassTransform1 Transform to transform center of mass of shape 1 into world space
	/// @param inCenterOfMassTransform2 Transform to transform center of mass of shape 2 into world space
	/// @param inSubShapeIDCreator1 Class that tracks the current sub shape ID for shape 1
	/// @param inSubShapeIDCreator2 Class that tracks the current sub shape ID for shape 2
	/// @param inCollideShapeSettings Options for the CollideShape test
	/// @param ioCollector The collector that receives the results.
	static inline void		sCollideShapeVsShape(const Shape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector)
	{
		sCollideShape[(int)inShape1->GetSubType()][(int)inShape2->GetSubType()](inShape1, inShape2, inScale1, inScale2, inCenterOfMassTransform1, inCenterOfMassTransform2, inSubShapeIDCreator1, inSubShapeIDCreator2, inCollideShapeSettings, ioCollector);
	}

	/// Cast a shape againt this shape, passes any hits found to ioCollector.
	/// @param inShapeCast The shape to cast against the other shape and its start and direction
	/// @param inShapeCastSettings Settings for performing the cast
	/// @param inShape The shape to cast against.
	/// @param inScale Local space scale for the shape to cast against.
	/// @param inShapeFilter Determines if sub shapes of the shape can collide
	/// @param inCenterOfMassTransform2 Is the center of mass transform of shape 2 (excluding scale), this is used to provide a transform to the shape cast result so that local quantities can be transformed into world space.
	/// @param inSubShapeIDCreator1 Class that tracks the current sub shape ID for the casting shape
	/// @param inSubShapeIDCreator2 Class that tracks the current sub shape ID for the shape we're casting against
	/// @param ioCollector The collector that receives the results.
	static inline void		sCastShapeVsShape(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, const Shape *inShape, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector)
	{
		// Only test shape if it passes the shape filter
		if (inShapeFilter.ShouldCollide(inSubShapeIDCreator1.GetID(), inSubShapeIDCreator2.GetID()))
			sCastShape[(int)inShapeCast.mShape->GetSubType()](inShapeCast, inShapeCastSettings, inShape, inScale, inShapeFilter, inCenterOfMassTransform2, inSubShapeIDCreator1, inSubShapeIDCreator2, ioCollector);
	}

	/// Function that collides 2 shapes (see sCollideShapeVsShape) 
	using CollideShape = void (*)(const Shape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector);

	/// Function that casts a shape vs another shape (see sCastShapeVsShape)
	using CastShape = void (*)(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, const Shape *inShape, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector);

	/// Register a collide shape function in the collision table
	static void				sRegisterCollideShape(EShapeSubType inType1, EShapeSubType inType2, CollideShape inFunction)	{ sCollideShape[(int)inType1][(int)inType2] = inFunction; }

	/// Register a cast shape function in the collision table
	static void				sRegisterCastShape(EShapeSubType inType1, CastShape inFunction)									{ sCastShape[(int)inType1] = inFunction; }

private:
	static CollideShape		sCollideShape[NumSubShapeTypes][NumSubShapeTypes];
	static CastShape		sCastShape[NumSubShapeTypes];
};

} // JPH