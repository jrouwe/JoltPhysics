// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Physics/Collision/Shape/SubShapeIDPair.h>
#include <Jolt/Core/StaticArray.h>

JPH_NAMESPACE_BEGIN

class Body;
class CollideShapeResult;

/// Array of contact points
using ContactPoints = StaticArray<Vec3, 64>;

/// Manifold class, describes the contact surface between two bodies
class ContactManifold
{
public:
	/// Swaps shape 1 and 2
	ContactManifold			SwapShapes() const					{ return { -mWorldSpaceNormal, mPenetrationDepth, mSubShapeID2, mSubShapeID1, mWorldSpaceContactPointsOn2, mWorldSpaceContactPointsOn1 }; }

	Vec3					mWorldSpaceNormal;					///< Normal for this manifold, direction along which to move body 2 out of collision along the shortest path
	float					mPenetrationDepth;					///< Penetration depth (move shape 2 by this distance to resolve the collision)
	SubShapeID				mSubShapeID1;						///< Sub shapes that formed this manifold (note that when multiple manifolds are combined because they're coplanar, we lose some information here because we only keep track of one sub shape pair that we encounter)
	SubShapeID				mSubShapeID2;
	ContactPoints			mWorldSpaceContactPointsOn1;		///< Contact points on the surface of shape 1 in world space.
	ContactPoints			mWorldSpaceContactPointsOn2;		///< Contact points on the surface of shape 2 in world space. If there's no penetration, this will be the same as mWorldSpaceContactPointsOn1. If there is penetration they will be different.
};

/// When a contact point is added or persisted, the callback gets a chance to override certain properties of the contact constraint.
/// The values are filled in with their defaults by the system so the callback doesn't need to modify anything, but it can if it wants to.
class ContactSettings
{
public:
	float					mCombinedFriction;					///< Combined friction for the body pair (usually calculated by sCombineFriction)
	float					mCombinedRestitution;				///< Combined restitution for the body pair (usually calculated by sCombineRestitution)
	bool					mIsSensor;							///< If the contact should be treated as a sensor vs body contact (no collision response)
};

/// Return value for the OnContactValidate callback. Determines if the contact is being processed or not.
/// Results are ordered so that the strongest accept has the lowest number and the strongest reject the highest number (which allows for easy combining of results)
enum class ValidateResult
{
	AcceptAllContactsForThisBodyPair,							///< Accept this and any further contact points for this body pair
	AcceptContact,												///< Accept this contact only (and continue calling this callback for every contact manifold for the same body pair)
	RejectContact,												///< Reject this contact only (but process any other contact manifolds for the same body pair)
	RejectAllContactsForThisBodyPair							///< Rejects this and any further contact points for this body pair
};

/// A listener class that receives collision contact events events.
/// It can be registered with the ContactConstraintManager (or PhysicsSystem).
class ContactListener
{
public:
	/// Ensure virtual destructor
	virtual					~ContactListener() = default;

	/// Called after detecting a collision between a body pair, but before calling OnContactAdded and before adding the contact constraint.
	/// If the function returns false, the contact will not be added and any other contacts between this body pair will not be processed.
	/// This function will only be called once per PhysicsSystem::Update per body pair and may not be called again the next update
	/// if a contact persists and no new contact pairs between sub shapes are found.
	/// This is a rather expensive time to reject a contact point since a lot of the collision detection has happened already, make sure you
	/// filter out the majority of undesired body pairs through the ObjectLayerPairFilter that is registered on the PhysicsSystem.
	/// Note that this callback is called when all bodies are locked, so don't use any locking functions!
	/// The order of body 1 and 2 is undefined, but when one of the two bodies is dynamic it will be body 1
	virtual ValidateResult	OnContactValidate(const Body &inBody1, const Body &inBody2, const CollideShapeResult &inCollisionResult) { return ValidateResult::AcceptAllContactsForThisBodyPair; }

	/// Called whenever a new contact point is detected.
	/// Note that this callback is called when all bodies are locked, so don't use any locking functions!
	/// Body 1 and 2 will be sorted such that body 1 ID < body 2 ID, so body 1 may not be dynamic.
	/// Note that only active bodies will report contacts, as soon as a body goes to sleep the contacts between that body and all other
	/// bodies will receive an OnContactRemoved callback, if this is the case then Body::IsActive() will return false during the callback.
	/// When contacts are added, the constraint solver has not run yet, so the collision impulse is unknown at that point.
	/// The velocities of inBody1 and inBody2 are the velocities before the contact has been resolved, so you can use this to
	/// estimate the collision impulse to e.g. determine the volume of the impact sound to play.
	virtual void			OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) { /* Do nothing */ }

	/// Called whenever a contact is detected that was also detected last update.
	/// Note that this callback is called when all bodies are locked, so don't use any locking functions!
	/// Body 1 and 2 will be sorted such that body 1 ID < body 2 ID, so body 1 may not be dynamic.
	virtual void			OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) { /* Do nothing */ }

	/// Called whenever a contact was detected last update but is not detected anymore.
	/// Note that this callback is called when all bodies are locked, so don't use any locking functions!
	/// Note that we're using BodyID's since the bodies may have been removed at the time of callback.
	/// Body 1 and 2 will be sorted such that body 1 ID < body 2 ID, so body 1 may not be dynamic.
	virtual void			OnContactRemoved(const SubShapeIDPair &inSubShapePair) { /* Do nothing */ }
};

JPH_NAMESPACE_END
