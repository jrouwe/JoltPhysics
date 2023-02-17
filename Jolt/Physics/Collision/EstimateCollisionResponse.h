// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/ContactListener.h>

JPH_NAMESPACE_BEGIN

using ContactImpulses = StaticArray<float, ContactPoints::capacity()>;

/// This function estimates the contact impulses and body velocity changes as a result of a collision.
/// It can be used in the ContactListener::OnContactAdded to determine the strength of the collision to e.g. play a sound or trigger a particle system.
/// It only uses the contact points and restitution to estimate the velocity changes and will ignore friction.
/// This function is accurate when two bodies collide but will not be accurate when more than 2 bodies collide at the same time as it does not know about these other collisions.
/// 
/// @param inBody1 Colliding body 1
/// @param inBody2 Colliding body 2
/// @param inManifold The collision manifold
/// @param outLinearVelocity1 Outputs the estimated linear velocity of body 1 after collision
/// @param outAngularVelocity1 Outputs the estimated angular velocity of body 1 after collision
/// @param outLinearVelocity2 Outputs the estimated linear velocity of body 2 after collision
/// @param outAngularVelocity2 Outputs the estimated angular velocity of body 2 after collision
/// @param outContactImpulses An array that will contain the estimated contact impulses when the function returns
/// @param inCombinedRestitution The combined restitution of body 1 and body 2 (see ContactSettings::mCombinedRestitution)
/// @param inMinVelocityForRestitution Minimal velocity required for restitution to be applied (see PhysicsSettings::mMinVelocityForRestitution)
/// @param inNumIterations Number of iterations to use for the impulse estimation (default = 4)
void EstimateCollisionResponse(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, Vec3 &outLinearVelocity1, Vec3 &outAngularVelocity1, Vec3 &outLinearVelocity2, Vec3 &outAngularVelocity2, ContactImpulses &outContactImpulses, float inCombinedRestitution, float inMinVelocityForRestitution = 1.0f, uint inNumIterations = 10);

JPH_NAMESPACE_END
