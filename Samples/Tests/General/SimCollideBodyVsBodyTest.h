// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>

class SimCollideBodyVsBodyTest : public Test, public ContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(JPH_NO_EXPORT, SimCollideBodyVsBodyTest)

	// Description of the test
	virtual const char *GetDescription() const override
	{
		return	"Overrides the collide body vs body function on the simulation to reduce the number of contact points generated between sensors and other objects in the simulation.\n"
				"This can be useful to improve performance if you don't need to know about all contact points and are only interested in an overlap/no-overlap result.\n"
				"The static world consists of a single compound shape with many pyramid sub shapes.";
	}

	// See: Test
	virtual void		Initialize() override;
	virtual void		PrePhysicsUpdate(const PreUpdateParams &inParams) override;

	// If this test implements a contact listener, it should be returned here
	virtual ContactListener *GetContactListener() override	{ return this; }

	// See: ContactListener
	virtual void		OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;
	virtual void		OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;

	// Saving / restoring state for replay
	virtual void		SaveState(StateRecorder &inStream) const override;
	virtual void		RestoreState(StateRecorder &inStream) override;

private:
	int					mPrevMode = -1;						// Previous mode
	float				mTime = 0.0f;						// Total elapsed time

	BodyID				mSensorID;							// Body ID of the sensor
	BodyIDVector		mBodyIDs;							// List of dynamic bodies
};
