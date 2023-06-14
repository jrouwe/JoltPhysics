// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Collision/ContactListener.h>

// Tests the contact listener callbacks
class ContactListenerTest : public Test, public ContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ContactListenerTest)

	// See: Test
	virtual void			Initialize() override;
	virtual void			PostPhysicsUpdate(float inDeltaTime) override;

	// If this test implements a contact listener, it should be returned here
	virtual ContactListener *GetContactListener() override		{ return this; }

	// See: ContactListener
	virtual ValidateResult	OnContactValidate(const Body &inBody1, const Body &inBody2, RVec3Arg inBaseOffset, const CollideShapeResult &inCollisionResult) override;
	virtual void			OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;

private:
	// The 4 bodies that we create
	Body *					mBody[4];

	// Tracks predicted velocities so we can compare them with the actual velocities after time step
	struct PredictedVelocity
	{
		BodyID				mBodyID;
		Vec3				mLinearVelocity;
		Vec3				mAngularVelocity;
	};
	Mutex					mPredictedVelocitiesMutex;
	Array<PredictedVelocity> mPredictedVelocities;
};
