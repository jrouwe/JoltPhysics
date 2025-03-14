// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Collision/ContactListener.h>

class ModifyMassTest : public Test, public ContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, ModifyMassTest)

	// Description of the test
	virtual const char *		GetDescription() const override
	{
		return	"Uses a contact listener to modify the mass of bodies per contacting body pair.\n"
				"Can be used to e.g. make a dynamic body respond normally to one body and appear to have infinite mass for another.";
	}

	// See: Test
	virtual void				Initialize() override;
	virtual void				PrePhysicsUpdate(const PreUpdateParams &inParams) override;
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

	// Update the labels on the bodies
	void						UpdateLabels();

	float						mTime = 0.0f;

	BodyID						mBodies[2];
};
