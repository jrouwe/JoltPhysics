// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Physics/Collision/Shape/Shape.h>

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
	static void				sCollideShapeVsShape(const Shape *inShape1, const Shape *inShape2, Vec3Arg inScale1, Vec3Arg inScale2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, const CollideShapeSettings &inCollideShapeSettings, CollideShapeCollector &ioCollector);

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
	static void				sCastShapeVsShape(const ShapeCast &inShapeCast, const ShapeCastSettings &inShapeCastSettings, const Shape *inShape, Vec3Arg inScale, const ShapeFilter &inShapeFilter, Mat44Arg inCenterOfMassTransform2, const SubShapeIDCreator &inSubShapeIDCreator1, const SubShapeIDCreator &inSubShapeIDCreator2, CastShapeCollector &ioCollector);
};

} // JPH