// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Body/Body.h>

JPH_NAMESPACE_BEGIN

class PhysicsSystem;

/// Class that does collision detection between wheels and ground
class VehicleCollisionTester : public RefTarget<VehicleCollisionTester>
{
public:
	/// Virtual destructor
	virtual						~VehicleCollisionTester() = default;

	/// Do a collision test with the world
	/// @param inPhysicsSystem The physics system that should be tested against
	/// @param inWheelIndex Index of the wheel that we're testing collision for
	/// @param inOrigin Origin for the test, corresponds to the world space position for the suspension attachment point
	/// @param inDirection Direction for the test (unit vector, world space)
	/// @param inSuspensionMaxLength Length of the suspension at max droop (m)
	/// @param inVehicleBodyID This body should be filtered out during collision detection to avoid self collisions
	/// @param outBody Body that the wheel collided with
	/// @param outSubShapeID Sub shape ID that the wheel collided with
	/// @param outContactPosition Contact point between wheel and floor, in world space
	/// @param outContactNormal Contact normal between wheel and floor, pointing away from the floor
	/// @param outSuspensionLength New length of the suspension [0, inSuspensionMaxLength]
	/// @return True when collision found, false if not
	virtual bool				Collide(PhysicsSystem &inPhysicsSystem, uint inWheelIndex, Vec3Arg inOrigin, Vec3Arg inDirection, float inSuspensionMaxLength, const BodyID &inVehicleBodyID, Body *&outBody, SubShapeID &outSubShapeID, Vec3 &outContactPosition, Vec3 &outContactNormal, float &outSuspensionLength) const = 0;
};

/// Collision tester that tests collision using a raycast
class VehicleCollisionTesterRay : public VehicleCollisionTester
{
public:
	/// Constructor
	/// @param inObjectLayer Object layer to test collision with
	/// @param inUp World space up vector, used to avoid colliding with vertical walls.
	/// @param inMaxSlopeAngle Max angle (rad) that is considered for colliding wheels. This is to avoid colliding with vertical walls.
								VehicleCollisionTesterRay(ObjectLayer inObjectLayer, Vec3Arg inUp = Vec3::sAxisY(), float inMaxSlopeAngle = DegreesToRadians(80.0f)) : mObjectLayer(inObjectLayer), mUp(inUp), mCosMaxSlopeAngle(cos(inMaxSlopeAngle)) { }

	// See: VehicleCollisionTester
	virtual bool				Collide(PhysicsSystem &inPhysicsSystem, uint inWheelIndex, Vec3Arg inOrigin, Vec3Arg inDirection, float inSuspensionMaxLength, const BodyID &inVehicleBodyID, Body *&outBody, SubShapeID &outSubShapeID, Vec3 &outContactPosition, Vec3 &outContactNormal, float &outSuspensionLength) const override;

private:
	ObjectLayer					mObjectLayer;
	Vec3						mUp;
	float						mCosMaxSlopeAngle;
};

/// Collision tester that tests collision using a sphere cast
class VehicleCollisionTesterCastSphere : public VehicleCollisionTester
{
public:
	/// Constructor
	/// @param inObjectLayer Object layer to test collision with
	/// @param inUp World space up vector, used to avoid colliding with vertical walls.
	/// @param inRadius Radius of sphere
	/// @param inMaxSlopeAngle Max angle (rad) that is considered for colliding wheels. This is to avoid colliding with vertical walls.
								VehicleCollisionTesterCastSphere(ObjectLayer inObjectLayer, float inRadius, Vec3Arg inUp = Vec3::sAxisY(), float inMaxSlopeAngle = DegreesToRadians(80.0f)) : mObjectLayer(inObjectLayer), mRadius(inRadius), mUp(inUp), mCosMaxSlopeAngle(cos(inMaxSlopeAngle)) { }

	// See: VehicleCollisionTester
	virtual bool				Collide(PhysicsSystem &inPhysicsSystem, uint inWheelIndex, Vec3Arg inOrigin, Vec3Arg inDirection, float inSuspensionMaxLength, const BodyID &inVehicleBodyID, Body *&outBody, SubShapeID &outSubShapeID, Vec3 &outContactPosition, Vec3 &outContactNormal, float &outSuspensionLength) const override;

private:
	ObjectLayer					mObjectLayer;
	float						mRadius;
	Vec3						mUp;
	float						mCosMaxSlopeAngle;
};

JPH_NAMESPACE_END
