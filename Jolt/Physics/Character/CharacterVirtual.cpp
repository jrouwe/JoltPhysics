// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Character/CharacterVirtual.h>
#include <Jolt/Physics/Body/Body.h>
#include <Jolt/Physics/PhysicsSystem.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Core/QuickSort.h>
#include <Jolt/Geometry/ConvexSupport.h>
#include <Jolt/Geometry/GJKClosestPoint.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

CharacterVirtual::CharacterVirtual(const CharacterVirtualSettings *inSettings, Vec3Arg inPosition, QuatArg inRotation, PhysicsSystem *inSystem) :
	CharacterBase(inSettings, inSystem),
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
void CharacterVirtual::sFillContactProperties(Contact &outContact, const Body &inBody, Vec3Arg inUp, const taCollector &inCollector, const CollideShapeResult &inResult)
{
	outContact.mPosition = inResult.mContactPointOn2;	
	outContact.mLinearVelocity = inBody.GetPointVelocity(inResult.mContactPointOn2);
	outContact.mContactNormal = -inResult.mPenetrationAxis.NormalizedOr(Vec3::sZero());
	outContact.mSurfaceNormal = inCollector.GetContext()->GetWorldSpaceSurfaceNormal(inResult.mSubShapeID2, inResult.mContactPointOn2);
	if (outContact.mContactNormal.Dot(outContact.mSurfaceNormal) < 0.0f)
		outContact.mSurfaceNormal = -outContact.mSurfaceNormal; // Flip surface normal if we're hitting a back face
	if (outContact.mContactNormal.Dot(inUp) > outContact.mSurfaceNormal.Dot(inUp))
		outContact.mSurfaceNormal = outContact.mContactNormal; // Replace surface normal with contact normal if the contact normal is pointing more upwards
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
		sFillContactProperties(contact, body, mUp, *this, inResult);
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
			sFillContactProperties(contact, body, mUp, *this, inResult);
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
	ContactCollector collector(mSystem, mMaxNumHits, mUp, outContacts);
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
					&& contact1.mContactNormal.Dot(contact2.mContactNormal) < 0.0f) // Only opposing normals
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

template <class T>
inline static bool sCorrectFractionForCharacterPadding(const Shape *inShape, Mat44Arg inStart, Vec3Arg inDisplacement, const T &inPolygon, float &ioFraction)
{
	if (inShape->GetType() == EShapeType::Convex)
	{
		// Get the support function for the shape we're casting
		const ConvexShape *convex_shape = static_cast<const ConvexShape *>(inShape);
		ConvexShape::SupportBuffer buffer;
		const ConvexShape::Support *support = convex_shape->GetSupportFunction(ConvexShape::ESupportMode::IncludeConvexRadius, buffer, Vec3::sReplicate(1.0f));

		// Cast the shape against the polygon
		GJKClosestPoint gjk;
		return gjk.CastShape(inStart, inDisplacement, cDefaultCollisionTolerance, *support, inPolygon, ioFraction);
	}
	else if (inShape->GetSubType() == EShapeSubType::RotatedTranslated)
	{
		const RotatedTranslatedShape *rt_shape = static_cast<const RotatedTranslatedShape *>(inShape);
		return sCorrectFractionForCharacterPadding(rt_shape->GetInnerShape(), inStart * Mat44::sRotation(rt_shape->GetRotation()), inDisplacement, inPolygon, ioFraction);
	}
	else
	{
		JPH_ASSERT(false, "Not supported yet!");
		return false;
	}
}

bool CharacterVirtual::GetFirstContactForSweep(Vec3Arg inPosition, Vec3Arg inDisplacement, Contact &outContact, const IgnoredContactList &inIgnoredContacts, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator) const
{
	// Too small distance -> skip checking
	float displacement_len_sq = inDisplacement.LengthSq();
	if (displacement_len_sq < 1.0e-8f)
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
	ContactCastCollector collector(mSystem, inDisplacement, mMaxNumHits, mUp, inIgnoredContacts, contacts);
	ShapeCast shape_cast(mShape, Vec3::sReplicate(1.0f), start, inDisplacement);
	mSystem->GetNarrowPhaseQuery().CastShape(shape_cast, settings, collector, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter);
	if (contacts.empty())
		return false;

	// Sort the contacts on fraction
	QuickSort(contacts.begin(), contacts.end(), [](const Contact &inLHS, const Contact &inRHS) { return inLHS.mFraction < inRHS.mFraction; });

	// Check the first contact that will make us penetrate more than the allowed tolerance
	bool valid_contact = false;
	for (const Contact &c : contacts)
		if (c.mDistance + c.mContactNormal.Dot(inDisplacement) < -mCollisionTolerance
			&& ValidateContact(c))
		{
			outContact = c;
			valid_contact = true;
			break;
		}
	if (!valid_contact)
		return false;

	// Fetch the face we're colliding with
	TransformedShape ts = mSystem->GetBodyInterface().GetTransformedShape(outContact.mBodyB);
	Shape::SupportingFace face;
	ts.GetSupportingFace(outContact.mSubShapeIDB, -outContact.mContactNormal, face);

	bool corrected = false;
	if (face.size() >= 2)
	{
		// Inflate the colliding face by the character padding
		PolygonConvexSupport polygon(face);
		AddConvexRadius add_cvx(polygon, mCharacterPadding);

		// Correct fraction to hit this inflated face instead of the inner shape
		corrected = sCorrectFractionForCharacterPadding(mShape, start, inDisplacement, add_cvx, outContact.mFraction);
	}
	if (!corrected)
	{
		// When there's only a single contact point or when we were unable to correct the fraction,
		// we can just move the fraction back so that the character and its padding don't hit the contact point anymore
		outContact.mFraction = max(0.0f, outContact.mFraction - mCharacterPadding / sqrt(displacement_len_sq));
	}

	return true;
}

void CharacterVirtual::DetermineConstraints(TempContactList &inContacts, ConstraintList &outConstraints) const
{
	for (Contact &c : inContacts)
	{
		Vec3 contact_velocity = c.mLinearVelocity;

		// Penetrating contact: Add a contact velocity that pushes the character out at the desired speed
		if (c.mDistance < 0.0f)
			contact_velocity -= c.mContactNormal * c.mDistance * mPenetrationRecoverySpeed;

		// Convert to a constraint
		outConstraints.emplace_back();
		Constraint &constraint = outConstraints.back();
		constraint.mContact = &c;
		constraint.mLinearVelocity = contact_velocity;
		constraint.mPlane = Plane(c.mContactNormal, c.mDistance);

		// Next check if the angle is too steep and if it is add an additional constraint that holds the character back
		if (IsSlopeTooSteep(c.mSurfaceNormal))
		{
			// Only take planes that point up
			float dot = c.mSurfaceNormal.Dot(mUp);
			if (dot > 0.0f)
			{
				// Make horizontal normal
				Vec3 normal = (c.mSurfaceNormal - dot * mUp).Normalized();

				// Create a secondary constraint that blocks horizontal movement
				outConstraints.emplace_back();
				Constraint &vertical_constraint = outConstraints.back();
				vertical_constraint.mContact = &c;
				vertical_constraint.mLinearVelocity = contact_velocity.Dot(normal) * normal; // Project the contact velocity on the new normal so that both planes push at an equal rate
				vertical_constraint.mPlane = Plane(normal, c.mDistance / normal.Dot(c.mSurfaceNormal)); // Calculate the distance we have to travel horizontally to hit the contact plane
			}
		}
	}
}

bool CharacterVirtual::HandleContact(Vec3Arg inVelocity, Constraint &ioConstraint, float inDeltaTime) const
{
	Contact &contact = *ioConstraint.mContact;

	// Validate the contact point
	if (!ValidateContact(contact))
		return false;

	// Send contact added event
	CharacterContactSettings settings;
	if (mListener != nullptr)
		mListener->OnContactAdded(this, contact.mBodyB, contact.mSubShapeIDB, contact.mPosition, -contact.mContactNormal, settings);
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
	float projected_velocity = relative_velocity.Dot(contact.mContactNormal);
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
	Vec3 jacobian = (contact.mPosition - center_of_mass).Cross(contact.mContactNormal);
	float inv_effective_mass = inverse_inertia.Multiply3x3(jacobian).Dot(jacobian) + inverse_mass;

	// Impulse P = M dv
	float impulse = delta_velocity / inv_effective_mass;

	// Clamp the impulse according to the character strength, character strength is a force in newtons, P = F dt
	float max_impulse = mMaxStrength * inDeltaTime;
	impulse = min(impulse, max_impulse);

	// Calculate the world space impulse to apply
	Vec3 world_impulse = -impulse * contact.mContactNormal;

	// Cancel impulse in down direction (we apply gravity later)
	float impulse_dot_up = world_impulse.Dot(mUp);
	if (impulse_dot_up < 0.0f)
		world_impulse -= impulse_dot_up * mUp;

	// Now apply the impulse (body is already locked so we use the no-lock interface)
	mSystem->GetBodyInterfaceNoLock().AddImpulse(contact.mBodyB, world_impulse, contact.mPosition);
	return true;
}

void CharacterVirtual::SolveConstraints(Vec3Arg inVelocity, float inDeltaTime, float inTimeRemaining, ConstraintList &ioConstraints, IgnoredContactList &ioIgnoredContacts, float &outTimeSimulated, Vec3 &outDisplacement, TempAllocator &inAllocator
#ifdef JPH_DEBUG_RENDERER
	, bool inDrawConstraints
#endif // JPH_DEBUG_RENDERER
	) const
{
	// If there are no constraints we can immediately move to our target
	if (ioConstraints.empty())
	{
		outDisplacement = inVelocity * inTimeRemaining;
		outTimeSimulated = inTimeRemaining;
		return;
	}

	// Create array that holds the constraints in order of time of impact (sort will happen later)
	std::vector<Constraint *, STLTempAllocator<Constraint *>> sorted_constraints(inAllocator);
	sorted_constraints.resize(ioConstraints.size());
	for (size_t index = 0; index < sorted_constraints.size(); index++)
		sorted_constraints[index] = &ioConstraints[index];

	// This is the velocity we use for the displacement, if we hit something it will be shortened
	Vec3 velocity = inVelocity;

	// Keep track of the last velocity that was applied to the character so that we can detect when the velocity reverses
	Vec3 last_velocity = inVelocity;

	// Start with no displacement
	outDisplacement = Vec3::sZero();
	outTimeSimulated = 0.0f;

	// These are the contacts that we hit previously without moving a significant distance
	std::vector<Constraint *, STLTempAllocator<Constraint *>> previous_contacts(inAllocator);
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
		QuickSort(sorted_constraints.begin(), sorted_constraints.end(), [](const Constraint *inLHS, const Constraint *inRHS) {
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
				if (!HandleContact(velocity, *c, inDeltaTime))
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
			new_velocity = velocity_in_slide_dir + perpendicular_velocity + other_perpendicular_velocity;
		}

		// Allow application to modify calculated velocity
		if (mListener != nullptr)
			mListener->OnContactSolve(this, constraint->mContact->mBodyB, constraint->mContact->mSubShapeIDB, constraint->mContact->mPosition, constraint->mContact->mContactNormal, constraint->mContact->mLinearVelocity, constraint->mContact->mMaterial, velocity, new_velocity);

#ifdef JPH_DEBUG_RENDERER
		if (inDrawConstraints)
		{
			// Calculate where to draw
			Vec3 offset = mPosition + Vec3(0, 0, 2.5f * (iteration + 1));

			// Draw constraint plane
			DebugRenderer::sInstance->DrawPlane(offset, constraint->mPlane.GetNormal(), Color::sCyan, 1.0f);

			// Draw 2nd constraint plane
			if (other_constraint != nullptr)
				DebugRenderer::sInstance->DrawPlane(offset, other_constraint->mPlane.GetNormal(), Color::sBlue, 1.0f);

			// Draw starting velocity
			DebugRenderer::sInstance->DrawArrow(offset, offset + velocity, Color::sGreen, 0.05f);

			// Draw resulting velocity
			DebugRenderer::sInstance->DrawArrow(offset, offset + new_velocity, Color::sRed, 0.05f);
		}
#endif // JPH_DEBUG_RENDERER

		// Update the velocity
		velocity = new_velocity;

		// Add the contact to the list so that next iteration we can avoid violating it again
		previous_contacts[num_previous_contacts] = constraint;
		num_previous_contacts++;

		// Check early out
		if (constraint->mProjectedVelocity < 1.0e-8f // Constraint should not be pushing, otherwise there may be other constraints that are pushing us
			&& velocity.LengthSq() < 1.0e-8f) // There's not enough velocity left
			return;

		// If the constraint has velocity we accept the new velocity, otherwise check that we didn't reverse velocity
		if (!constraint->mLinearVelocity.IsNearZero(1.0e-8f))
			last_velocity = constraint->mLinearVelocity;
		else if (velocity.Dot(last_velocity) < 0.0f)
			return;
	}
}

void CharacterVirtual::UpdateSupportingContact(bool inSkipContactVelocityCheck, TempAllocator &inAllocator)
{
	// Flag contacts as having a collision if they're close enough but ignore contacts we're moving away from.
	// Note that if we did MoveShape before we want to preserve any contacts that it marked as colliding
	for (Contact &c : mActiveContacts)
		if (!c.mWasDiscarded)
			c.mHadCollision |= c.mDistance < mCollisionTolerance
								&& (inSkipContactVelocityCheck || c.mSurfaceNormal.Dot(mLinearVelocity - c.mLinearVelocity) <= 0.0f);

	// Calculate transform that takes us to character local space
	Mat44 inv_transform = Mat44::sInverseRotationTranslation(mRotation, mPosition);

	// Determine if we're supported or not
	int num_supported = 0;
	int num_sliding = 0;
	int num_avg_normal = 0;
	Vec3 avg_normal = Vec3::sZero();
	Vec3 avg_velocity = Vec3::sZero();
	const Contact *supporting_contact = nullptr;
	float max_cos_angle = -FLT_MAX;
	const Contact *deepest_contact = nullptr;
	float smallest_distance = FLT_MAX;
	for (const Contact &c : mActiveContacts)
		if (c.mHadCollision)
		{
			// Calculate the angle between the plane normal and the up direction
			float cos_angle = c.mSurfaceNormal.Dot(mUp);

			// Find the deepest contact
			if (c.mDistance < smallest_distance)
			{
				deepest_contact = &c;
				smallest_distance = c.mDistance;
			}

			// If this contact is in front of our plane, we cannot be supported by it
			if (mSupportingVolume.SignedDistance(inv_transform * c.mPosition) > 0.0f)
				continue;

			// Find the contact with the normal that is pointing most upwards and store it
			if (max_cos_angle < cos_angle)
			{
				supporting_contact = &c;
				max_cos_angle = cos_angle;
			}

			// Check if this is a sliding or supported contact
			bool is_supported = mCosMaxSlopeAngle > cNoMaxSlopeAngle || cos_angle >= mCosMaxSlopeAngle;
			if (is_supported)
				num_supported++;
			else
				num_sliding++;

			// If the angle between the two is less than 85 degrees we also use it to calculate the average normal
			if (cos_angle >= 0.08f)
			{
				avg_normal += c.mSurfaceNormal;
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

	// Take either the most supporting contact or the deepest contact
	const Contact *best_contact = supporting_contact != nullptr? supporting_contact : deepest_contact;

	// Calculate average normal and velocity
	if (num_avg_normal >= 1)
	{
		mGroundNormal = avg_normal.Normalized();
		mGroundVelocity = avg_velocity / float(num_avg_normal);
	}
	else if (best_contact != nullptr)
	{
		mGroundNormal = best_contact->mSurfaceNormal;
		mGroundVelocity = best_contact->mLinearVelocity;
	}
	else
	{
		mGroundNormal = Vec3::sZero();
		mGroundVelocity = Vec3::sZero();
	}

	// Copy contact properties
	if (best_contact != nullptr)
	{
		mGroundBodyID = best_contact->mBodyB;
		mGroundBodySubShapeID = best_contact->mSubShapeIDB;
		mGroundPosition = best_contact->mPosition;
		mGroundMaterial = best_contact->mMaterial;
		mGroundUserData = best_contact->mUserData;
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
		SolveConstraints(-mUp, 1.0f, 1.0f, constraints, ignored_contacts, time_simulated, displacement, inAllocator);

		// If we're blocked then we're supported, otherwise we're sliding
		float min_required_displacement_sq = Square(0.6f * mLastDeltaTime);
		if (time_simulated < 0.001f || displacement.LengthSq() < min_required_displacement_sq)
			mGroundState = EGroundState::OnGround;
		else
			mGroundState = EGroundState::OnSteepGround;
	}
	else
	{
		// Not supported by anything
		mGroundState = best_contact != nullptr? EGroundState::NotSupported : EGroundState::InAir;
	}
}

void CharacterVirtual::StoreActiveContacts(const TempContactList &inContacts, TempAllocator &inAllocator)
{
	mActiveContacts.assign(inContacts.begin(), inContacts.end());

	UpdateSupportingContact(true, inAllocator);
}

void CharacterVirtual::MoveShape(Vec3 &ioPosition, Vec3Arg inVelocity, float inDeltaTime, ContactList *outActiveContacts, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator
#ifdef JPH_DEBUG_RENDERER
	, bool inDrawConstraints
#endif // JPH_DEBUG_RENDERER
	) const
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
		bool draw_constraints = inDrawConstraints && iteration == 0;
		if (draw_constraints)
		{
			for (const Constraint &c : constraints)
			{
				// Draw contact point
				DebugRenderer::sInstance->DrawMarker(c.mContact->mPosition, Color::sYellow, 0.05f);
				Vec3 dist_to_plane = -c.mPlane.GetConstant() * c.mPlane.GetNormal();

				// Draw arrow towards surface that we're hitting
				DebugRenderer::sInstance->DrawArrow(c.mContact->mPosition, c.mContact->mPosition - dist_to_plane, Color::sYellow, 0.05f);

				// Draw plane around the player position indicating the space that we can move
				DebugRenderer::sInstance->DrawPlane(mPosition + dist_to_plane, c.mPlane.GetNormal(), Color::sCyan, 1.0f);
				DebugRenderer::sInstance->DrawArrow(mPosition + dist_to_plane, mPosition + dist_to_plane + c.mContact->mSurfaceNormal, Color::sRed, 0.05f);
			}
		}
#endif // JPH_DEBUG_RENDERER

		// Solve the displacement using these constraints
		Vec3 displacement;
		float time_simulated;
		SolveConstraints(inVelocity, inDeltaTime, time_remaining, constraints, ignored_contacts, time_simulated, displacement, inAllocator
		#ifdef JPH_DEBUG_RENDERER
			, draw_constraints
		#endif // JPH_DEBUG_RENDERER
			);

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

Vec3 CharacterVirtual::CancelVelocityTowardsSteepSlopes(Vec3Arg inDesiredVelocity) const
{
	// If we're not pushing against a steep slope, return the desired velocity
	// Note: This is important as WalkStairs overrides the ground state to OnGround when its first check fails but the second succeeds
	if (mGroundState == CharacterVirtual::EGroundState::OnGround
		|| mGroundState == CharacterVirtual::EGroundState::InAir)
		return inDesiredVelocity;
		
	Vec3 desired_velocity = inDesiredVelocity;
	for (const Contact &c : mActiveContacts)
		if (c.mHadCollision
			&& IsSlopeTooSteep(c.mSurfaceNormal))
		{
			Vec3 normal = c.mSurfaceNormal;

			// Remove normal vertical component
			normal -= normal.Dot(mUp) * mUp;

			// Cancel horizontal movement in opposite direction
			float dot = normal.Dot(desired_velocity);
			if (dot < 0.0f)
				desired_velocity -= (dot * normal) / normal.LengthSq();
		}
	return desired_velocity;
}

void CharacterVirtual::Update(float inDeltaTime, Vec3Arg inGravity, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator)
{
	// If there's no delta time, we don't need to do anything
	if (inDeltaTime <= 0.0f)
		return;

	// Remember delta time for checking if we're supported by the ground
	mLastDeltaTime = inDeltaTime;

	// Slide the shape through the world
	MoveShape(mPosition, mLinearVelocity, inDeltaTime, &mActiveContacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator
	#ifdef JPH_DEBUG_RENDERER
		, sDrawConstraints
	#endif // JPH_DEBUG_RENDERER
		);

	// Determine the object that we're standing on
	UpdateSupportingContact(false, inAllocator);

	// If we're on the ground
	if (!mGroundBodyID.IsInvalid() && mMass > 0.0f)
	{
		// Add the impulse to the ground due to gravity: P = F dt = M g dt
		float normal_dot_gravity = mGroundNormal.Dot(inGravity);
		if (normal_dot_gravity < 0.0f)
		{
			Vec3 world_impulse = -(mMass * normal_dot_gravity / inGravity.Length() * inDeltaTime) * inGravity;
			mSystem->GetBodyInterface().AddImpulse(mGroundBodyID, world_impulse, mGroundPosition);
		}
	}
}

void CharacterVirtual::RefreshContacts(const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator)
{
	// Determine the contacts
	TempContactList contacts(inAllocator);
	contacts.reserve(mMaxNumHits);
	GetContactsAtPosition(mPosition, mLinearVelocity.NormalizedOr(Vec3::sZero()), mShape, contacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter);

	StoreActiveContacts(contacts, inAllocator);
}

void CharacterVirtual::MoveToContact(Vec3Arg inPosition, const Contact &inContact, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator)
{
	// Set the new position
	SetPosition(inPosition);

	// Determine the contacts
	TempContactList contacts(inAllocator);
	contacts.reserve(mMaxNumHits + 1); // +1 because we can add one extra below
	GetContactsAtPosition(mPosition, mLinearVelocity.NormalizedOr(Vec3::sZero()), mShape, contacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter);

	// Ensure that we mark inContact as colliding
	bool found_contact = false;
	for (Contact &c : contacts)
		if (c.mBodyB == inContact.mBodyB
			&& c.mSubShapeIDB == inContact.mSubShapeIDB)
		{
			c.mHadCollision = true;
			found_contact = true;
		}
	if (!found_contact)
	{
		contacts.push_back(inContact);

		Contact &copy = contacts.back();
		copy.mHadCollision = true;
	}

	StoreActiveContacts(contacts, inAllocator);
	JPH_ASSERT(mGroundState != EGroundState::InAir);
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

bool CharacterVirtual::CanWalkStairs(Vec3Arg inLinearVelocity) const
{
	// We can only walk stairs if we're supported
	if (!IsSupported())
		return false;

	// Check if there's enough horizontal velocity to trigger a stair walk
	Vec3 horizontal_velocity = inLinearVelocity - inLinearVelocity.Dot(mUp) * mUp;
	if (horizontal_velocity.IsNearZero(1.0e-6f))
		return false;

	// Check contacts for steep slopes
	for (const Contact &c : mActiveContacts)
		if (c.mHadCollision
			&& c.mSurfaceNormal.Dot(horizontal_velocity - c.mLinearVelocity) < 0.0f // Pushing into the contact
			&& IsSlopeTooSteep(c.mSurfaceNormal)) // Slope too steep
			return true;

	return false;
}

bool CharacterVirtual::WalkStairs(float inDeltaTime, Vec3Arg inStepUp, Vec3Arg inStepForward, Vec3Arg inStepForwardTest, Vec3Arg inStepDownExtra, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator)
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
		DebugRenderer::sInstance->DrawArrow(mPosition, up_position, Color::sWhite, 0.01f);
#endif // JPH_DEBUG_RENDERER

	// Horizontal movement
	Vec3 new_position = up_position;
	MoveShape(new_position, inStepForward / inDeltaTime, inDeltaTime, nullptr, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator);
	float horizontal_movement_sq = (new_position - up_position).LengthSq();
	if (horizontal_movement_sq < 1.0e-8f)
		return false; // No movement, cancel

#ifdef JPH_DEBUG_RENDERER
	// Draw horizontal sweep
	if (sDrawWalkStairs)
		DebugRenderer::sInstance->DrawArrow(up_position, new_position, Color::sWhite, 0.01f);
#endif // JPH_DEBUG_RENDERER

	// Move down towards the floor.
	// Note that we travel the same amount down as we travelled up with the character padding and the specified extra
	// If we don't add the character padding, we may miss the floor (note that GetFirstContactForSweep will subtract the padding when it finds a hit)
	Vec3 down = -up - mCharacterPadding * mUp + inStepDownExtra;
	if (!GetFirstContactForSweep(new_position, down, contact, dummy_ignored_contacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator))
		return false; // No floor found, we're in mid air, cancel stair walk

#ifdef JPH_DEBUG_RENDERER
	// Draw sweep down
	if (sDrawWalkStairs)
	{
		Vec3 debug_pos = new_position + contact.mFraction * down; 
		DebugRenderer::sInstance->DrawArrow(new_position, debug_pos, Color::sWhite, 0.01f);
		DebugRenderer::sInstance->DrawArrow(contact.mPosition, contact.mPosition + contact.mSurfaceNormal, Color::sWhite, 0.01f);
		mShape->Draw(DebugRenderer::sInstance, GetCenterOfMassTransform(debug_pos, mRotation, mShape), Vec3::sReplicate(1.0f), Color::sWhite, false, true);
	}
#endif // JPH_DEBUG_RENDERER

	// Test for floor that will support the character
	if (IsSlopeTooSteep(contact.mSurfaceNormal))
	{
		// If no test position was provided, we cancel the stair walk
		if (inStepForwardTest.IsNearZero())
			return false;

		// Delta time may be very small, so it may be that we hit the edge of a step and the normal is too horizontal.
		// In order to judge if the floor is flat further along the sweep, we test again for a floor at inStepForwardTest
		// and check if the normal is valid there.
		Vec3 test_position = up_position;
		MoveShape(test_position, inStepForwardTest / inDeltaTime, inDeltaTime, nullptr, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator);
		float test_horizontal_movement_sq = (test_position - up_position).LengthSq();
		if (test_horizontal_movement_sq <= horizontal_movement_sq + 1.0e-8f)
			return false; // We didn't move any further than in the previous test

	#ifdef JPH_DEBUG_RENDERER
		// Draw 2nd sweep horizontal
		if (sDrawWalkStairs)
			DebugRenderer::sInstance->DrawArrow(up_position, test_position, Color::sCyan, 0.01f);
	#endif // JPH_DEBUG_RENDERER

		// Then sweep down
		Contact test_contact;
		if (!GetFirstContactForSweep(test_position, down, test_contact, dummy_ignored_contacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator))
			return false;

	#ifdef JPH_DEBUG_RENDERER
		// Draw 2nd sweep down
		if (sDrawWalkStairs)
		{
			Vec3 debug_pos = test_position + test_contact.mFraction * down; 
			DebugRenderer::sInstance->DrawArrow(test_position, debug_pos, Color::sCyan, 0.01f);
			DebugRenderer::sInstance->DrawArrow(test_contact.mPosition, test_contact.mPosition + test_contact.mSurfaceNormal, Color::sCyan, 0.01f);
			mShape->Draw(DebugRenderer::sInstance, GetCenterOfMassTransform(debug_pos, mRotation, mShape), Vec3::sReplicate(1.0f), Color::sCyan, false, true);
		}
	#endif // JPH_DEBUG_RENDERER

		if (IsSlopeTooSteep(test_contact.mSurfaceNormal))
			return false;
	}

	// Calculate new down position
	down *= contact.mFraction;
	new_position += down;

	// Move the character to the new location
	MoveToContact(new_position, contact, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator);

	// Override ground state to 'on ground', it is possible that the contact normal is too steep, but in this case the inStepForwardTest has found a contact normal that is not too steep
	mGroundState = EGroundState::OnGround;

	return true;
}

bool CharacterVirtual::StickToFloor(Vec3Arg inStepDown, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator)
{
	// Try to find the floor
	Contact contact;
	IgnoredContactList dummy_ignored_contacts(inAllocator);
	if (!GetFirstContactForSweep(mPosition, inStepDown, contact, dummy_ignored_contacts, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator))
		return false; // If no floor found, don't update our position

	// Calculate new position
	Vec3 new_position = mPosition + contact.mFraction * inStepDown;

#ifdef JPH_DEBUG_RENDERER
	// Draw sweep down
	if (sDrawStickToFloor)
	{
		DebugRenderer::sInstance->DrawArrow(mPosition, new_position, Color::sOrange, 0.01f);
		mShape->Draw(DebugRenderer::sInstance, GetCenterOfMassTransform(new_position, mRotation, mShape), Vec3::sReplicate(1.0f), Color::sOrange, false, true);
	}
#endif // JPH_DEBUG_RENDERER

	// Move the character to the new location
	MoveToContact(new_position, contact, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator);
	return true;
}

void CharacterVirtual::ExtendedUpdate(float inDeltaTime, Vec3Arg inGravity, const ExtendedUpdateSettings &inSettings, const BroadPhaseLayerFilter &inBroadPhaseLayerFilter, const ObjectLayerFilter &inObjectLayerFilter, const BodyFilter &inBodyFilter, TempAllocator &inAllocator)
{
	// Update the velocity
	Vec3 desired_velocity = mLinearVelocity;
	mLinearVelocity = CancelVelocityTowardsSteepSlopes(desired_velocity);
	
	// Remember old position
	Vec3 old_position = mPosition;

	// Track if on ground before the update
	bool ground_to_air = IsSupported();

	// Update the character position (instant, do not have to wait for physics update)
	Update(inDeltaTime, inGravity, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator);

	// ... and that we got into air after
	if (IsSupported())
		ground_to_air = false;

	// If stick to floor enabled and we're going from supported to not supported
	if (ground_to_air && !inSettings.mStickToFloorStepDown.IsNearZero())
	{
		// If we're not moving up, stick to the floor
		float velocity = (mPosition - old_position).Dot(mUp) / inDeltaTime;
		if (velocity <= 1.0e-6f)
			StickToFloor(inSettings.mStickToFloorStepDown, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator);
	}

	// If walk stairs enabled
	if (!inSettings.mWalkStairsStepUp.IsNearZero())
	{
		// Calculate how much we wanted to move horizontally
		Vec3 desired_horizontal_step = desired_velocity * inDeltaTime;
		desired_horizontal_step -= desired_horizontal_step.Dot(mUp) * mUp;
		float desired_horizontal_step_len = desired_horizontal_step.Length();
		if (desired_horizontal_step_len > 0.0f)
		{
			// Calculate how much we moved horizontally
			Vec3 achieved_horizontal_step = mPosition - old_position;
			achieved_horizontal_step -= achieved_horizontal_step.Dot(mUp) * mUp;

			// Only count movement in the direction of the desired movement
			// (otherwise we find it ok if we're sliding downhill while we're trying to climb uphill)
			Vec3 step_forward_normalized = desired_horizontal_step / desired_horizontal_step_len;
			achieved_horizontal_step = max(0.0f, achieved_horizontal_step.Dot(step_forward_normalized)) * step_forward_normalized;
			float achieved_horizontal_step_len = achieved_horizontal_step.Length();

			// If we didn't move as far as we wanted and we're against a slope that's too steep
			if (achieved_horizontal_step_len + 1.0e-4f < desired_horizontal_step_len
				&& CanWalkStairs(desired_velocity))
			{
				// Calculate how much we should step forward
				// Note that we clamp the step forward to a minimum distance. This is done because at very high frame rates the delta time
				// may be very small, causing a very small step forward. If the step becomes small enough, we may not move far enough
				// horizontally to actually end up at the top of the step.
				Vec3 step_forward = step_forward_normalized * max(inSettings.mWalkStairsMinStepForward, desired_horizontal_step_len - achieved_horizontal_step_len);

				// Calculate how far to scan ahead for a floor. This is only used in case the floor normal at step_forward is too steep.
				// In that case an additional check will be performed at this distance to check if that normal is not too steep.
				// Start with the ground normal in the horizontal plane and normalizing it
				Vec3 step_forward_test = -mGroundNormal;
				step_forward_test -= step_forward_test.Dot(mUp) * mUp;
				step_forward_test = step_forward_test.NormalizedOr(step_forward_normalized);

				// If this normalized vector and the character forward vector is bigger than a preset angle, we use the character forward vector instead of the ground normal
				// to do our forward test
				if (step_forward_test.Dot(step_forward_normalized) < inSettings.mWalkStairsCosAngleForwardContact)
					step_forward_test = step_forward_normalized;

				// Calculate the correct magnitude for the test vector
				step_forward_test *= inSettings.mWalkStairsStepForwardTest;

				WalkStairs(inDeltaTime, inSettings.mWalkStairsStepUp, step_forward, step_forward_test, inSettings.mWalkStairsStepDownExtra, inBroadPhaseLayerFilter, inObjectLayerFilter, inBodyFilter, inAllocator);
			}
		}
	}
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
