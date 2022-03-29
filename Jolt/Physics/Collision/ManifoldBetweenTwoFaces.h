// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/Shape/ConvexShape.h>
#include <Jolt/Physics/Collision/ContactListener.h>

JPH_NAMESPACE_BEGIN

/// Remove contact points if there are > 4 (no more than 4 are needed for a stable solution)
/// @param inCenterOfMass is the world space center of mass for body 1
/// @param inPenetrationAxis is the world space penetration axis (must be normalized)
/// @param ioContactPointsOn1 The world space contact points on shape 1
/// @param ioContactPointsOn2 The world space contact points on shape 2
/// On output ioContactPointsOn1/2 are reduced to 4 or less points
void PruneContactPoints(Vec3Arg inCenterOfMass, Vec3Arg inPenetrationAxis, ContactPoints &ioContactPointsOn1, ContactPoints &ioContactPointsOn2);

/// Determine contact points between 2 faces of 2 shapes and return them in outContactPoints 1 & 2
/// @param inContactPoint1 The contact point on shape 1 in world space
/// @param inContactPoint2 The contact point on shape 2 in world space
/// @param inPenetrationAxis The local space penetration axis in world space
/// @param inSpeculativeContactDistanceSq Squared speculative contact distance, any contact further apart than this distance will be discarded
/// @param inShape1Face The supporting faces on shape 1 in world space
/// @param inShape2Face The supporting faces on shape 2 in world space
/// @param outContactPoints1 Returns the contact points between the two shapes for shape 1 in world space (any existing points in the output array are left as is)
/// @param outContactPoints2 Returns the contact points between the two shapes for shape 2 in world space (any existing points in the output array are left as is)
void ManifoldBetweenTwoFaces(Vec3Arg inContactPoint1, Vec3Arg inContactPoint2, Vec3Arg inPenetrationAxis, float inSpeculativeContactDistanceSq, const ConvexShape::SupportingFace &inShape1Face, const ConvexShape::SupportingFace &inShape2Face, ContactPoints &outContactPoints1, ContactPoints &outContactPoints2);

JPH_NAMESPACE_END
