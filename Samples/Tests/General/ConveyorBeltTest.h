// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

// This test shows how to create a conveyor belt
class ConveyorBeltTest : public Test, public ContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ConveyorBeltTest)

	// See: Test
	virtual void			Initialize() override;

	// If this test implements a contact listener, it should be returned here
	virtual ContactListener *GetContactListener() override		{ return this; }

	// See: ContactListener
	virtual void			OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;
	virtual void			OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;

private:
	BodyIDVector			mLinearBelts;
	BodyID					mAngularBelt;
};
