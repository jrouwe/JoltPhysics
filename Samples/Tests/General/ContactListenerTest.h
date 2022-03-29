// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Collision/ContactListener.h>

// Tests the contact listener callbacks
class ContactListenerTest : public Test, public ContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(ContactListenerTest)

	// See: Test
	virtual void			Initialize() override;

	// If this test implements a contact listener, it should be returned here
	virtual ContactListener *GetContactListener() override		{ return this; }

	// See: ContactListener
	virtual ValidateResult	OnContactValidate(const Body &inBody1, const Body &inBody2, const CollideShapeResult &inCollisionResult) override;
	virtual void			OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;

private:
	// The 4 bodies that we create
	Body *					mBody[4];
};
