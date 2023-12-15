// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Collision/ContactListener.h>

// Tests modifying mass from a contact listener
class ModifyMassTest : public Test, public ContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ModifyMassTest)

	// See: Test
	virtual void				Initialize() override;
	virtual void				PrePhysicsUpdate(const PreUpdateParams &inParams) override;
	virtual void				PostPhysicsUpdate(float inDeltaTime) override;
	virtual void				SaveState(StateRecorder &inStream) const override;
	virtual void				RestoreState(StateRecorder &inStream) override;

	// If this test implements a contact listener, it should be returned here
	virtual ContactListener *	GetContactListener() override		{ return this; }

	// See: ContactListener
	virtual void				OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;
	virtual void				OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;

private:
	// Get the scale factor for a body based on its user data
	static float				sGetInvMassScale(const Body &inBody);

	// Reset the bodies to their initial states
	void						ResetBodies(int inCycle);

	float						mTime = 0.0f;

	BodyID						mBodies[2];
};
