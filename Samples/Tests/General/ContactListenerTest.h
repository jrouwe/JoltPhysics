// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Collision/ContactListener.h>

class ContactListenerTest : public Test, public ContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ContactListenerTest)

	// Description of the test
	virtual const char *	GetDescription() const override
	{
		return	"Demonstrates how to listen for contact events.\n"
				"Leftmost box ignores contacts with the 2nd box and overrides the restitution to 1 for non-persisted contacts.\n"
				"Rightmost box contains an inner and an outer shape, the outer shape acts as a sensor.\n"
				"The TTY will output estimated post collision velocities.";
	}

	// See: Test
	virtual void			Initialize() override;
	virtual void			PostPhysicsUpdate(float inDeltaTime) override;

	// If this test implements a contact listener, it should be returned here
	virtual ContactListener *GetContactListener() override		{ return this; }

	// See: ContactListener
	virtual ValidateResult	OnContactValidate(const Body &inBody1, const Body &inBody2, RVec3Arg inBaseOffset, const CollideShapeResult &inCollisionResult) override;
	virtual void			OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;
	virtual void			OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;

private:
	void					MakeBody5PartialSensor(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings);

	// The 5 bodies that we create
	Body *					mBody[5];

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
