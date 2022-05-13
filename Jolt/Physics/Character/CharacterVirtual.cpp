// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CollideShape.h>

JPH_NAMESPACE_BEGIN

CharacterVirtual::CharacterVirtual(const CharacterVirtualSettings *inSettings, Vec3Arg inPosition, QuatArg inRotation, PhysicsSystem *inSystem) :
	CharacterBase(inSettings, inSystem),
	mUp(inSettings->mUp),
	mPredictiveContactDistance(inSettings->mPredictiveContactDistance),
	mMaxCollisionIterations(inSettings->mMaxCollisionIterations),
	mMaxConstraintIterations(inSettings->mMaxConstraintIterations),
	mMinTimeRemaining(inSettings->mMinTimeRemaining),
	mCollisionTolerance(inSettings->mCollisionTolerance),
	mCharacterPadding(inSettings->mCharacterPadding),
	mMaxNumHits(inSettings->mMaxNumHits),
	mPenetrationRecoverySpeed(inSettings->mPenetrationRecoverySpeed),
	mPosition(inPosition),
	mRotation(inRotation)
{
	// Copy settings
	SetMaxStrength(inSettings->mMaxStrength);
	SetMass(inSettings->mMass);
}

template <class taCollector>
void CharacterVirtual::sFillContactProperties(Contact &outContact, const Body &inBody, const taCollector &inCollector, const CollideShapeResult &inResult)
{
	outContact.mPosition = inResult.mContactPointOn2;	
	outContact.mLinearVelocity = inBody.GetPointVelocity(inResult.mContactPointOn2);
	outContact.mNormal = -inResult.mPenetrationAxis.NormalizedOr(Vec3::sZero());
	outContact.mDistance = -inResult.mPenetrationDepth;
	outContact.mBodyB = inResult.mBodyID2;
	outContact.mSubShapeIDB = inResult.mSubShapeID2;
	outContact.mMotionTypeB = inBody.GetMotionType();
	outContact.mUserData = inBody.GetUserData();
	outContact.mMaterial = inCollector.GetContext()->GetMaterial(inResult.mSubShapeID2);
}

void CharacterVirtual::ContactCollector::AddHit(const CollideShapeResult &inResult)
{
	BodyLockRead lock(mSystem->GetBodyLockInterface(), inResult.mBodyID2);
	if (lock.SucceededAndIsInBroadPhase())
	{
		const Body &body = lock.GetBody();

		mContacts.emplace_back();
		Contact &contact = mContacts.back();
		sFillContactProperties(contact, body, *this, inResult);
		contact.mFraction = 0.0f;

		// Protection from excess of contact points
		if (mContacts.size() == mMaxHits)
			ForceEarlyOut();
	}
}

void CharacterVirtual::ContactCastCollector::AddHit(const ShapeCastResult &inResult)
{	
	if (inResult.mFraction > 0.0f // Ignore collisions at fraction = 0
		&& inResult.mPenetrationAxis.Dot(mDisplacement) > 0.0f) // Ignore penetrations that we're moving away from
	{
		// Test if this contact should be ignored
		for (const IgnoredContact &c : mIgnoredContacts)
			if (c.mBodyID == inResult.mBodyID2 && c.mSubShapeID == inResult.mSubShapeID2)
				return;

		BodyLockRead lock(mSystem->GetBodyLockInterface(), inResult.mBodyID2);
		if (lock.SucceededAndIsInBroadPhase())
		{
			const Body &body = lock.GetBody();

			mContacts.emplace_back();
			Contact &contact = mContacts.back();
			sFillContactProperties(contact, body, *this, inResult);
			contact.mFraction = inResult.mFraction;

			// Protection from excess of contact points
			if (mContacts.size() == mMaxHits)
				ForceEarlyOut();
		}
	}
}

void CharacterVirtual::CheckCollision(Vec3Arg inPosition, QuatArg inRotation, Vec3Arg inMovementDirection, float inMaxSeparationDistance, const Shape *inShape, CollideShapeCollector &ioCollector, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter) const
{
	// Query shape transform
	Mat44 transform = GetCenterOfMassTransform(inPosition, inRotation, inShape);

	// Settings for collide shape
	CollideShapeSettings settings;
	settings.mActiveEdgeMode = EActiveEdgeMode::CollideOnlyWithActive;
	settings.mBackFaceMode = EBackFaceMode::CollideWithBackFaces;
	settings.mActiveEdgeMovementDirection = inMovementDirection;
	settings.mMaxSeparationDistance = mCharacterPadding + inMaxSeparationDistance;

	// Collide shape
	mSystem->GetNarrowPhaseQuery().CollideShape(inShape, Vec3::sReplicate(1.0f), transform, settings, ioCollector, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter);
}

void CharacterVirtual::GetContactsAtPosition(Vec3Arg inPosition, Vec3Arg inMovementDirection, const Shape *inShape, TempContactList &outContacts, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter) const
{
	// Remove previous results
	outContacts.clear();

	// Collide shape
	ContactCollector collector(mSystem, mMaxNumHits, outContacts);
	CheckCollision(inPosition, mRotation, inMovementDirection, mPredictiveContactDistance, inShape, collector, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter);

	// Reduce distance to contact by padding to ensure we stay away from the object by a little margin
	// (this will make collision detection cheaper - especially for sweep tests as they won't hit the surface if we're properly sliding)
	for (Contact &c : outContacts)
		c.mDistance -= mCharacterPadding;
}

void CharacterVirtual::RemoveConflictingContacts(TempContactList &ioContacts, IgnoredContactList &outIgnoredContacts) const
{
	// Only use this algorithm if we're penetrating further than this (due to numerical precision issues we can always penetrate a little bit and we don't want to discard contacts if they just have a tiny penetration)
	// We do need to account for padding (see GetContactsAtPosition) that is removed from the contact distances, to compensate we add it to the cMinRequiredPenetration
	const float cMinRequiredPenetration = 1.25f * mCharacterPadding;

	// Discard conflicting penetrating contacts
	for (size_t c1 = 0; c1 < ioContacts.size(); c1++)
	{
		Contact &contact1 = ioContacts[c1];
		if (contact1.mDistance <= -cMinRequiredPenetration) // Only for penetrations
			for (size_t c2 = c1 + 1; c2 < ioContacts.size(); c2++)
			{
				Contact &contact2 = ioContacts[c2];
				if (contact1.mBodyB == contact2.mBodyB // Only same body
					&& contact2.mDistance <= -cMinRequiredPenetration // Only for penetrations
					&& contact1.mNormal.Dot(contact2.mNormal) < 0.0f) // Only opposing normals
				{
					// Discard contacts with the least amount of penetration
					if (contact1.mDistance < contact2.mDistance)
					{
						// Discard the 2nd contact
						outIgnoredContacts.emplace_back(contact2.mBodyB, contact2.mSubShapeIDB);
						ioContacts.erase(ioContacts.begin() + c2);
						c2--;
					}
					else
					{
						// Discard the first contact
						outIgnoredContacts.emplace_back(contact1.mBodyB, contact1.mSubShapeIDB);
						ioContacts.erase(ioContacts.begin() + c1);
						c1--;
						break;
					}
				}
			}
	}
}

bool CharacterVirtual::ValidateContact(const Contact &inContact) const
{
	if (mListener == nullptr)
		return true;

	return mListener->OnContactValidate(this, inContact.mBodyB, inContact.mSubShapeIDB);
}

bool CharacterVirtual::GetFirstContactForSweep(Vec3Arg inPosition, Vec3Arg inDisplacement, Contact &outContact, const IgnoredContactList &inIgnoredContacts, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator) const
{
	// Too small distance -> skip checking
	if (inDisplacement.LengthSq() < 1.0e-8f)
		return false;

	// Calculate start transform
	Mat44 start = GetCenterOfMassTransform(inPosition, mRotation, mShape);

	// Settings for the cast
	ShapeCastSettings settings;
	settings.mBackFaceModeTriangles = EBackFaceMode::CollideWithBackFaces;
	settings.mBackFaceModeConvex = EBackFaceMode::IgnoreBackFaces;
	settings.mActiveEdgeMode = EActiveEdgeMode::CollideOnlyWithActive;
	settings.mUseShrunkenShapeAndConvexRadius = true;
	settings.mReturnDeepestPoint = false;

	// Cast shape
	TempContactList contacts(inAllocator);
	contacts.reserve(mMaxNumHits);
	ContactCastCollector collector(mSystem, inDisplacement, mMaxNumHits, inIgnoredContacts, contacts);
	ShapeCast shape_cast(mShape, Vec3::sReplicate(1.0f), start, inDisplacement);
	mSystem->GetNarrowPhaseQuery().CastShape(shape_cast, settings, collector, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter);
	if (contacts.empty())
		return false;

	// Sort the contacts on fraction
	sort(contacts.begin(), contacts.end(), [](const Contact &inLHS, const Contact &inRHS) { return inLHS.mFraction < inRHS.mFraction; });

	// Check the first contact that will make us penetrate more than the allowed tolerance
	bool valid_contact = false;
	for (const Contact &c : contacts)
		if (c.mDistance + c.mNormal.Dot(inDisplacement) < -mCollisionTolerance
			&& ValidateContact(c))
		{
			outContact = c;
			valid_contact = true;
			break;
		}
	if (!valid_contact)
		return false;

	// Correct fraction for the padding that we want to keep from geometry
	// We want to maintain distance of cCharacterPadding (p) along plane normal outContact.mNormal (n) to the capsule by moving back along inDisplacement (d) by amount d'
	// cos(angle between d and -n) = -n dot d / |d| = p / d'
	// <=> d' = -p |d| / n dot d
	// The new fraction of collision is then:
	// f' = f - d' / |d| = f + p / n dot d
	float dot = outContact.mNormal.Dot(inDisplacement);
	if (dot < 0.0f) // We should not divide by zero and we should only update the fraction if normal is pointing towards displacement
		outContact.mFraction = max(0.0f, outContact.mFraction + mCharacterPadding / dot);
	return true;
}

void CharacterVirtual::DetermineConstraints(TempContactList &inContacts, ConstraintList &outConstraints) const
{
	for (Contact &c : inContacts)
	{
		Vec3 contact_velocity = c.mLinearVelocity;

		// Penetrating contact: Add a contact velocity that pushes the character out at the desired speed
		if (c.mDistance < 0.0f)
			contact_velocity -= c.mNormal * c.mDistance * mPenetrationRecoverySpeed;

		// Convert to a constraint
		outConstraints.emplace_back();
		Constraint &constraint = outConstraints.back();
		constraint.mContact = &c;
		constraint.mLinearVelocity = contact_velocity;
		constraint.mPlane = Plane(c.mNormal, c.mDistance);

		// Next check if the angle is too steep and if it is add an additional constraint that holds the character back
		if (mCosMaxSlopeAngle < 0.999f) // If cos(slope angle) is close to 1 then there's no limit
		{
			float dot = c.mNormal.Dot(mUp);
			if (dot > 0.0f && dot < mCosMaxSlopeAngle)
			{
				// Make horizontal normal
				Vec3 normal = (c.mNormal - dot * mUp).Normalized();

				// Create a secondary constraint that blocks horizontal movement
				outConstraints.emplace_back();
				Constraint &vertical_constraint = outConstraints.back();
				vertical_constraint.mContact = &c;
				vertical_constraint.mLinearVelocity = contact_velocity.Dot(normal) * normal; // Project the contact velocity on the new normal so that both planes push at an equal rate
				vertical_constraint.mPlane = Plane(normal, c.mDistance / normal.Dot(c.mNormal)); // Calculate the distance we have to travel horizontally to hit the contact plane
			}
		}
	}
}

bool CharacterVirtual::HandleContact(Vec3Arg inVelocity, Constraint &ioConstraint, Vec3Arg inGravity, float inDeltaTime) const
{
	Contact &contact = *ioConstraint.mContact;

	// Validate the contact point
	if (!ValidateContact(contact))
		return false;

	// Send contact added event
	CharacterContactSettings settings;
	if (mListener != nullptr)
		mListener->OnContactAdded(this, contact.mBodyB, contact.mSubShapeIDB, contact.mPosition, -contact.mNormal, settings);
	contact.mCanPushCharacter = settings.mCanPushCharacter;

	// If body B cannot receive an impulse, we're done
	if (!settings.mCanReceiveImpulses || contact.mMotionTypeB != EMotionType::Dynamic)
		return true;

	// Lock the body we're colliding with
	BodyLockWrite lock(mSystem->GetBodyLockInterface(), contact.mBodyB);
	if (!lock.SucceededAndIsInBroadPhase())
		return false; // Body has been removed, we should not collide with it anymore
	const Body &body = lock.GetBody();

	// Calculate the velocity that we want to apply at B so that it will start moving at the character's speed at the contact point
	constexpr float cDamping = 0.9f;
	constexpr float cPenetrationResolution = 0.4f;
	Vec3 relative_velocity = inVelocity - contact.mLinearVelocity;
	float projected_velocity = relative_velocity.Dot(contact.mNormal);
	float delta_velocity = -projected_velocity * cDamping - min(contact.mDistance, 0.0f) * cPenetrationResolution / inDeltaTime;

	// Don't apply impulses if we're separating
	if (delta_velocity < 0.0f)
		return true;

	// Determine mass properties of the body we're colliding with
	const MotionProperties *motion_properties = body.GetMotionProperties();
	Vec3 center_of_mass = body.GetCenterOfMassPosition();
	Mat44 inverse_inertia = body.GetInverseInertia();
	float inverse_mass = motion_properties->GetInverseMass();

	// Calculate the inverse of the mass of body B as seen at the contact point in the direction of the contact normal
	Vec3 jacobian = (contact.mPosition - center_of_mass).Cross(contact.mNormal);
	float inv_effective_mass = inverse_inertia.Multiply3x3(jacobian).Dot(jacobian) + inverse_mass;

	// Impulse P = M dv
	float impulse = delta_velocity / inv_effective_mass;

	// Clamp the impulse according to the character strength, character strength is a force in newtons, P = F dt
	float max_impulse = mMaxStrength * inDeltaTime;
	impulse = min(impulse, max_impulse);

	// Calculate the world space impulse to apply
	Vec3 world_impulse = -impulse * contact.mNormal;

	// Add the impulse due to gravity working on the player: P = F dt = M g dt
	float normal_dot_gravity = contact.mNormal.Dot(inGravity);
	if (normal_dot_gravity < 0.0f)
		world_impulse -= (mMass * normal_dot_gravity / inGravity.Length() * inDeltaTime) * inGravity;

	// Now apply the impulse (body is already locked so we use the no-lock interface)
	mSystem->GetBodyInterfaceNoLock().AddImpulse(contact.mBodyB, world_impulse, contact.mPosition);
	return true;
}

void CharacterVirtual::SolveConstraints(Vec3Arg inVelocity, Vec3Arg inGravity, float inDeltaTime, float inTimeRemaining, ConstraintList &ioConstraints, IgnoredContactList &ioIgnoredContacts, float &outTimeSimulated, Vec3 &outDisplacement, TempAllocator &inAllocator) const
{
	// If there are no constraints we can immediately move to our target
	if (ioConstraints.empty())
	{
		outDisplacement = inVelocity * inTimeRemaining;
		outTimeSimulated = inTimeRemaining;
		return;
	}

	// Create array that holds the constraints in order of time of impact (sort will happen later)
	vector<Constraint *, STLTempAllocator<Constraint *>> sorted_constraints(inAllocator);
	sorted_constraints.resize(ioConstraints.size());
	for (size_t index = 0; index < sorted_constraints.size(); index++)
		sorted_constraints[index] = &ioConstraints[index];

	// This is the velocity we use for the displacement, if we hit something it will be shortened
	Vec3 velocity = inVelocity;

	// Start with no displacement
	outDisplacement = Vec3::sZero();
	outTimeSimulated = 0.0f;

	// These are the contacts that we hit previously without moving a significant distance
	vector<Constraint *, STLTempAllocator<Constraint *>> previous_contacts(inAllocator);
	previous_contacts.resize(mMaxConstraintIterations);
	int num_previous_contacts = 0;

	// Loop for a max amount of iterations
	for (uint iteration = 0; iteration < mMaxConstraintIterations; iteration++)
	{
		// Calculate time of impact for all constraints
		for (Constraint &c : ioConstraints)
		{
			// Project velocity on plane direction
			c.mProjectedVelocity = c.mPlane.GetNormal().Dot(c.mLinearVelocity - velocity);
			if (c.mProjectedVelocity < 1.0e-6f)
			{
				c.mTOI = FLT_MAX;
			}
			else
			{
				// Distance to plane
				float dist = c.mPlane.SignedDistance(outDisplacement);

				if (dist - c.mProjectedVelocity * inTimeRemaining > -1.0e-4f)
				{
					// Too little penetration, accept the movement
					c.mTOI = FLT_MAX;
				}
				else
				{
					// Calculate time of impact
					c.mTOI = max(0.0f, dist / c.mProjectedVelocity);
				}
			}
		}
				
		// Sort constraints on proximity
		sort(sorted_constraints.begin(), sorted_constraints.end(), [](const Constraint *inLHS, const Constraint *inRHS) {
				// If both constraints hit at t = 0 then order the one that will push the character furthest first
				// Note that because we add velocity to penetrating contacts, this will also resolve contacts that penetrate the most
				if (inLHS->mTOI <= 0.0f && inRHS->mTOI <= 0.0f)
					return inLHS->mProjectedVelocity > inRHS->mProjectedVelocity;

				// Then sort on time of impact
				if (inLHS->mTOI != inRHS->mTOI)
					return inLHS->mTOI < inRHS->mTOI;

				// As a tie breaker sort static first so it has the most influence
				return inLHS->mContact->mMotionTypeB > inRHS->mContact->mMotionTypeB;
			});

		// Find the first valid constraint
		Constraint *constraint = nullptr;
		for (Constraint *c : sorted_constraints)
		{
			// Take the first contact and see if we can reach it
			if (c->mTOI >= inTimeRemaining)
			{
				// We can reach our goal!
				outDisplacement += velocity * inTimeRemaining;
				outTimeSimulated += inTimeRemaining;
				return;
			}

			// Test if this contact was discarded by the contact callback before
			if (c->mContact->mWasDiscarded)
				continue;

			// Check if we made contact with this before
			if (!c->mContact->mHadCollision)
			{
				// Handle the contact
				if (!HandleContact(velocity, *c, inGravity, inDeltaTime))
				{
					// Constraint should be ignored, remove it from the list
					c->mContact->mWasDiscarded = true;

					// Mark it as ignored for GetFirstContactForSweep
					ioIgnoredContacts.emplace_back(c->mContact->mBodyB, c->mContact->mSubShapeIDB);
					continue;
				}

				c->mContact->mHadCollision = true;
			}

			// Cancel velocity of constraint if it cannot push the character
			if (!c->mContact->mCanPushCharacter)
				c->mLinearVelocity = Vec3::sZero();

			// We found the first constraint that we want to collide with
			constraint = c;
			break;
		}

		if (constraint == nullptr)
		{
			// All constraints were discarded, we can reach our goal!
			outDisplacement += velocity * inTimeRemaining;
			outTimeSimulated += inTimeRemaining;
			return;
		}

		// Move to the contact
		outDisplacement += velocity * constraint->mTOI;
		inTimeRemaining -= constraint->mTOI;
		outTimeSimulated += constraint->mTOI;

		// If there's not enough time left to be simulated, bail
		if (inTimeRemaining < mMinTimeRemaining)
			return;

		// If we've moved significantly, clear all previous contacts
		if (constraint->mTOI > 1.0e-4f)
			num_previous_contacts = 0;

		// Get the normal of the plane we're hitting
		Vec3 plane_normal = constraint->mPlane.GetNormal();

		// Get the relative velocity between the character and the constraint
		Vec3 relative_velocity = velocity - constraint->mLinearVelocity;

		// Calculate new velocity if we cancel the relative velocity in the normal direction
		Vec3 new_velocity = velocity - relative_velocity.Dot(plane_normal) * plane_normal;

		// Find the normal of the previous contact that we will violate the most if we move in this new direction
		float highest_penetration = 0.0f;
		Constraint *other_constraint = nullptr;
		for (Constraint **c = previous_contacts.data(); c < previous_contacts.data() + num_previous_contacts; ++c)
			if (*c != constraint)
			{
				// Calculate how much we will penetrate if we move in this direction
				Vec3 other_normal = (*c)->mPlane.GetNormal();
				float penetration = ((*c)->mLinearVelocity - new_velocity).Dot(other_normal);
				if (penetration > highest_penetration)
				{
					// We don't want parallel or anti-parallel normals as that will cause our cross product below to become zero. Slack is approx 10 degrees.
					float dot = other_normal.Dot(plane_normal);
					if (dot < 0.984f && dot > -0.984f) 
					{
						highest_penetration = penetration;
						other_constraint = *c;
					}
				}
			}

		// Check if we found a 2nd constraint
		if (other_constraint != nullptr)
		{
			// Calculate the sliding direction and project the new velocity onto that sliding direction
			Vec3 other_normal = other_constraint->mPlane.GetNormal();
			Vec3 slide_dir = plane_normal.Cross(other_normal).Normalized();
			Vec3 velocity_in_slide_dir = new_velocity.Dot(slide_dir) * slide_dir;

			// Cancel the constraint velocity in the other constraint plane's direction so that we won't try to apply it again and keep ping ponging between planes
			constraint->mLinearVelocity -= min(0.0f, constraint->mLinearVelocity.Dot(other_normal)) * other_normal;

			// Cancel the other constraints velocity in this constraint plane's direction so that we won't try to apply it again and keep ping ponging between planes
			other_constraint->mLinearVelocity -= min(0.0f, other_constraint->mLinearVelocity.Dot(plane_normal)) * plane_normal;

			// Calculate the velocity of this constraint perpendicular to the slide direction
			Vec3 perpendicular_velocity = constraint->mLinearVelocity - constraint->mLinearVelocity.Dot(slide_dir) * slide_dir;

			// Calculate the velocity of the other constraint perpendicular to the slide direction
			Vec3 other_perpendicular_velocity = other_constraint->mLinearVelocity - other_constraint->mLinearVelocity.Dot(slide_dir) * slide_dir;

			// Add all components together
			velocity = velocity_in_slide_dir + perpendicular_velocity + other_perpendicular_velocity;
		}			
		else
		{
			// Update the velocity
			velocity = new_velocity;
		}

		// Add the contact to the list so that next iteration we can avoid violating it again
		previous_contacts[num_previous_contacts] = constraint;
		num_previous_contacts++;

		// If there's not enough velocity left, bail
		if (velocity.LengthSq() < 1.0e-8f)
			return;
	}
}

void CharacterVirtual::UpdateSupportingContact(TempAllocator &inAllocator)
{
	// Flag contacts as having a collision if they're close enough.
	// Note that if we did MoveShape before we want to preserve any contacts that it marked as colliding
	for (Contact &c : mActiveContacts)
		if (!c.mWasDiscarded)
			c.mHadCollision |= c.mDistance < mCollisionTolerance;

	// Determine if we're supported or not
	int num_supported = 0;
	int num_sliding = 0;
	int num_avg_normal = 0;
	Vec3 avg_normal = Vec3::sZero();
	Vec3 avg_velocity = Vec3::sZero();
	const Contact *supporting_contact = nullptr;
	float max_cos_angle = -FLT_MAX;
	for (const Contact &c : mActiveContacts)
		if (c.mHadCollision)
		{
			// Calculate the angle between the plane normal and the up direction
			float cos_angle = c.mNormal.Dot(mUp);

			// Find the contact with the normal that is pointing most upwards and store it in mSupportingContact
			if (max_cos_angle < cos_angle)
			{
				supporting_contact = &c;
				max_cos_angle = cos_angle;
			}

			// Check if this is a sliding or supported contact
			bool is_supported = cos_angle >= mCosMaxSlopeAngle;
			if (is_supported)
				num_supported++;
			else
				num_sliding++;

			// If the angle between the two is less than 85 degrees we also use it to calculate the average normal
			if (cos_angle >= 0.08f)
			{
				avg_normal += c.mNormal;
				num_avg_normal++;

				// For static or dynamic objects or for contacts that don't support us just take the contact velocity
				if (c.mMotionTypeB != EMotionType::Kinematic || !is_supported)
					avg_velocity += c.mLinearVelocity;
				else
				{
					// For keyframed objects that support us calculate the velocity at our position rather than at the contact position so that we properly follow the object
					// Note that we don't just take the point velocity because a point on an object with angular velocity traces an arc, 
					// so if you just take point velocity * delta time you get an error that accumulates over time

					// Determine center of mass and angular velocity
					Vec3 angular_velocity, com;
					{
						BodyLockRead lock(mSystem->GetBodyLockInterface(), c.mBodyB);
						if (lock.SucceededAndIsInBroadPhase())
						{
							const Body &body = lock.GetBody();

							// Add the linear velocity to the average velocity
							avg_velocity += body.GetLinearVelocity();

							angular_velocity = body.GetAngularVelocity();
							com = body.GetCenterOfMassPosition();
						}
						else
						{
							angular_velocity = Vec3::sZero();
							com = Vec3::sZero();
						}
					}

					// Get angular velocity
					float angular_velocity_len_sq = angular_velocity.LengthSq();
					if (angular_velocity_len_sq > 1.0e-12f)
					{
						float angular_velocity_len = sqrt(angular_velocity_len_sq);

						// Calculate the rotation that the object will make in the time step
						Quat rotation = Quat::sRotation(angular_velocity / angular_velocity_len, angular_velocity_len * mLastDeltaTime);

						// Calculate where the new contact position will be
						Vec3 new_position = com + rotation * (mPosition - com);

						// Calculate the velocity
						avg_velocity += (new_position - mPosition) / mLastDeltaTime;
					}
				}
			}
		}

	// Calculate average normal and velocity
	if (num_avg_normal >= 1)
	{
		mGroundNormal = avg_normal.Normalized();
		mGroundVelocity = avg_velocity / float(num_avg_normal);
	}
	else
	{
		mGroundNormal = Vec3::sZero();
		mGroundVelocity = Vec3::sZero();
	}

	// Copy supporting contact properties
	if (supporting_contact != nullptr)
	{
		mGroundBodyID = supporting_contact->mBodyB;
		mGroundBodySubShapeID = supporting_contact->mSubShapeIDB;
		mGroundPosition = supporting_contact->mPosition;
		mGroundMaterial = supporting_contact->mMaterial;
		mGroundUserData = supporting_contact->mUserData;
	}
	else
	{
		mGroundBodyID = BodyID();
		mGroundBodySubShapeID = SubShapeID();
		mGroundPosition = Vec3::sZero();
		mGroundMaterial = PhysicsMaterial::sDefault;
		mGroundUserData = 0;
	}

	// Determine ground state
	if (num_supported > 0)
	{
		// We made contact with something that supports us
		mGroundState = EGroundState::OnGround;
	}
	else if (num_sliding > 0)
	{
		// If we're sliding we may actually be standing on multiple sliding contacts in such a way that we can't slide off, in this case we're also supported

		// Convert the contacts into constraints
		TempContactList contacts(mActiveContacts.begin(), mActiveContacts.end(), inAllocator);
		ConstraintList constraints(inAllocator);
		constraints.reserve(contacts.size() * 2);
		DetermineConstraints(contacts, constraints);

		// Solve the displacement using these constraints, this is used to check if we didn't move at all because we are supported
		Vec3 displacement;
		float time_simulated;
		IgnoredContactList ignored_contacts(inAllocator);
		ignored_contacts.reserve(contacts.size());
		SolveConstraints(-mUp, mSystem->GetGravity(), 1.0f, 1.0f, constraints, ignored_contacts, time_simulated, displacement, inAllocator);

		// If we're blocked then we're supported, otherwise we're sliding
		constexpr float cMinRequiredDisplacementSquared = Square(0.01f);
		if (time_simulated < 0.001f || displacement.LengthSq() < cMinRequiredDisplacementSquared)
			mGroundState = EGroundState::OnGround;
		else
			mGroundState = EGroundState::Sliding;
	}
	else
	{
		// Not in contact with anything
		mGroundState = EGroundState::InAir;
	}
}

void CharacterVirtual::StoreActiveContacts(const TempContactList &inContacts, TempAllocator &inAllocator)
{
	mActiveContacts.assign(inContacts.begin(), inContacts.end());

	UpdateSupportingContact(inAllocator);
}

void CharacterVirtual::MoveShape(Vec3 &ioPosition, Vec3Arg inVelocity, Vec3Arg inGravity, float inDeltaTime, ContactList *outActiveContacts, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator) const
{
	Vec3 movement_direction = inVelocity.NormalizedOr(Vec3::sZero());

	float time_remaining = inDeltaTime;
	for (uint iteration = 0; iteration < mMaxCollisionIterations && time_remaining >= mMinTimeRemaining; iteration++)
	{
		// Determine contacts in the neighborhood
		TempContactList contacts(inAllocator);
		contacts.reserve(mMaxNumHits);
		GetContactsAtPosition(ioPosition, movement_direction, mShape, contacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter);

		// Remove contacts with the same body that have conflicting normals
		IgnoredContactList ignored_contacts(inAllocator);
		ignored_contacts.reserve(contacts.size());
		RemoveConflictingContacts(contacts, ignored_contacts);

		// Convert contacts into constraints
		ConstraintList constraints(inAllocator);
		constraints.reserve(contacts.size() * 2);
		DetermineConstraints(contacts, constraints);

#ifdef JPH_DEBUG_RENDERER
		if (sDrawConstraints && iteration == 0)
		{
			for (const Constraint &c : constraints)
			{
				// Draw contact point
				DebugRenderer::sInstance->DrawMarker(c.mContact->mPosition, Color::sYellow, 0.05f);
				Vec3 dist_to_plane = -c.mPlane.GetConstant() * c.mPlane.GetNormal();

				// Draw arrow towards surface that we're hitting
				DebugRenderer::sInstance->DrawArrow(c.mContact->mPosition, c.mContact->mPosition - dist_to_plane, Color::sYellow, 0.05f);

				// Draw plane around the player posiiton indicating the space that we can move
				DebugRenderer::sInstance->DrawPlane(mPosition + dist_to_plane, c.mPlane.GetNormal(), Color::sCyan, 1.0f);
			}
		}
#endif // JPH_DEBUG_RENDERER

		// Solve the displacement using these constraints
		Vec3 displacement;
		float time_simulated;
		SolveConstraints(inVelocity, inGravity, inDeltaTime, time_remaining, constraints, ignored_contacts, time_simulated, displacement, inAllocator);

		// Store the contacts now that the colliding ones have been marked
		if (outActiveContacts != nullptr)
			outActiveContacts->assign(contacts.begin(), contacts.end());

		// Do a sweep to test if the path is really unobstructed
		Contact cast_contact;
		if (GetFirstContactForSweep(ioPosition, displacement, cast_contact, ignored_contacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator))
		{
			displacement *= cast_contact.mFraction;
			time_simulated *= cast_contact.mFraction;
		}

		// Update the position
		ioPosition += displacement;
		time_remaining -= time_simulated;

		// If the displacement during this iteration was too small we assume we cannot further progress this update
		if (displacement.LengthSq() < 1.0e-8f)
			break;
	}
}

void CharacterVirtual::Update(float inDeltaTime, Vec3Arg inGravity, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator)
{
	// If there's no delta time, we don't need to do anything
	if (inDeltaTime <= 0.0f)
		return;

	// Remember delta time for checking if we're supported by the ground
	mLastDeltaTime = inDeltaTime;

	// Slide the shape through the world
	MoveShape(mPosition, mLinearVelocity, inGravity, inDeltaTime, &mActiveContacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator);

	// Determine the object that we're standing on
	UpdateSupportingContact(inAllocator);
}

void CharacterVirtual::RefreshContacts(const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator)
{
	// Determine the contacts
	TempContactList contacts(inAllocator);
	contacts.reserve(mMaxNumHits);
	GetContactsAtPosition(mPosition, mLinearVelocity.NormalizedOr(Vec3::sZero()), mShape, contacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter);

	StoreActiveContacts(contacts, inAllocator);
}

bool CharacterVirtual::SetShape(const Shape *inShape, float inMaxPenetrationDepth, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator)
{
	if (mShape == nullptr || mSystem == nullptr)
	{
		// It hasn't been initialized yet
		mShape = inShape;
		return true;
	}

	if (inShape != mShape && inShape != nullptr)
	{
		if (inMaxPenetrationDepth < FLT_MAX)
		{
			// Check collision around the new shape
			TempContactList contacts(inAllocator);
			contacts.reserve(mMaxNumHits);
			GetContactsAtPosition(mPosition, mLinearVelocity.NormalizedOr(Vec3::sZero()), inShape, contacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter);

			// Test if this results in penetration, if so cancel the transition
			for (const Contact &c : contacts)
				if (c.mDistance < -inMaxPenetrationDepth)
					return false;

			StoreActiveContacts(contacts, inAllocator);
		}

		// Set new shape
		mShape = inShape;
	}

	return mShape == inShape;
}

bool CharacterVirtual::CanWalkStairs() const
{
	// Check if there's enough horizontal velocity to trigger a stair walk
	Vec3 horizontal_velocity = mLinearVelocity - mLinearVelocity.Dot(mUp) * mUp;
	if (horizontal_velocity.IsNearZero(1.0e-6f))
		return false;

	// Check contacts for steep slopes
	for (const Contact &c : mActiveContacts)
		if (c.mHadCollision
			&& c.mNormal.Dot(horizontal_velocity - c.mLinearVelocity) < 0.0f // Pushing into the contact
			&& c.mNormal.Dot(mUp) < mCosMaxSlopeAngle) // Slope too steep
			return true;

	return false;
}

bool CharacterVirtual::WalkStairs(float inDeltaTime, Vec3Arg inGravity, Vec3Arg inStepUp, Vec3Arg inStepForward, Vec3Arg inStepForwardTest, Vec3Arg inStepDownExtra, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator)
{
	// Move up
	Vec3 up = inStepUp;
	Contact contact;
	IgnoredContactList dummy_ignored_contacts(inAllocator);
	if (GetFirstContactForSweep(mPosition, up, contact, dummy_ignored_contacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator))
	{
		if (contact.mFraction < 1.0e-6f)
			return false; // No movement, cancel

		// Limit up movement to the first contact point
		up *= contact.mFraction;
	}
	Vec3 up_position = mPosition + up;

#ifdef JPH_DEBUG_RENDERER
	// Draw sweep up
	if (sDrawWalkStairs)
		DebugRenderer::sInstance->DrawArrow(mPosition, up_position, Color::sGrey, 0.01f);
#endif // JPH_DEBUG_RENDERER

	// Horizontal movement
	Vec3 new_position = up_position;
	MoveShape(new_position, inStepForward / inDeltaTime, inGravity, inDeltaTime, nullptr, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator);
	if (new_position.IsClose(up_position, 1.0e-8f))
		return false; // No movement, cancel

#ifdef JPH_DEBUG_RENDERER
	// Draw horizontal sweep
	if (sDrawWalkStairs)
		DebugRenderer::sInstance->DrawArrow(up_position, new_position, Color::sGrey, 0.01f);
#endif // JPH_DEBUG_RENDERER

	// Move down towards the floor.
	// Note that we travel the same amount down as we travelled up with the specified extra
	Vec3 down = -up + inStepDownExtra;
	if (!GetFirstContactForSweep(new_position, down, contact, dummy_ignored_contacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator))
		return false; // No floor found, we're in mid air, cancel stair walk

#ifdef JPH_DEBUG_RENDERER
	// Draw sweep down
	if (sDrawWalkStairs)
	{
		Vec3 debug_pos = new_position + contact.mFraction * down; 
		DebugRenderer::sInstance->DrawArrow(new_position, debug_pos, Color::sYellow, 0.01f);
		DebugRenderer::sInstance->DrawArrow(contact.mPosition, contact.mPosition + contact.mNormal, Color::sYellow, 0.01f);
		mShape->Draw(DebugRenderer::sInstance, GetCenterOfMassTransform(debug_pos, mRotation, mShape), Vec3::sReplicate(1.0f), Color::sYellow, false, true);
	}
#endif // JPH_DEBUG_RENDERER

	// Test for floor that will support the character
	if (mCosMaxSlopeAngle < 0.999f // If cos(slope angle) is close to 1 then there's no limit
		&& contact.mNormal.Dot(mUp) < mCosMaxSlopeAngle) // Check slope angle
	{
		// If no test position was provided, we cancel the stair walk
		if (inStepForwardTest.IsNearZero())
			return false;

		// Delta time may be very small, so it may be that we hit the edge of a step and the normal is too horizontal.
		// In order to judge if the floor is flat further along the sweep, we test again for a floor at inStepForwardTest
		// and check if the normal is valid there.
		Vec3 test_position = up_position + inStepForwardTest;
		Contact test_contact;
		bool hit = GetFirstContactForSweep(test_position, down, test_contact, dummy_ignored_contacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator);
		if (!hit)
			return false;

	#ifdef JPH_DEBUG_RENDERER
		// Draw 2nd sweep down
		if (sDrawWalkStairs)
		{
			Vec3 debug_pos = test_position + test_contact.mFraction * down; 
			DebugRenderer::sInstance->DrawArrow(test_position, debug_pos, Color::sCyan, 0.01f);
			DebugRenderer::sInstance->DrawArrow(test_contact.mPosition, test_contact.mPosition + test_contact.mNormal, Color::sCyan, 0.01f);
			mShape->Draw(DebugRenderer::sInstance, GetCenterOfMassTransform(debug_pos, mRotation, mShape), Vec3::sReplicate(1.0f), Color::sCyan, false, true);
		}
	#endif // JPH_DEBUG_RENDERER

		if (test_contact.mNormal.Dot(mUp) < mCosMaxSlopeAngle)
			return false;
	}

	// Calculate new down position
	down *= contact.mFraction;
	new_position += down;

	// Move the character to the new location
	SetPosition(new_position);
	RefreshContacts(inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator);
	return true;
}

void CharacterVirtual::SaveState(StateRecorder &inStream) const
{
	CharacterBase::SaveState(inStream);

	inStream.Write(mPosition);
	inStream.Write(mRotation);
	inStream.Write(mLinearVelocity);
}

void CharacterVirtual::RestoreState(StateRecorder &inStream)
{
	CharacterBase::RestoreState(inStream);

	inStream.Read(mPosition);
	inStream.Read(mRotation);
	inStream.Read(mLinearVelocity);
}

JPH_NAMESPACE_END
