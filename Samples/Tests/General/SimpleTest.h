// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Body/BodyActivationListener.h>

// This is a very basic test that just drops a few objects on the floor
class SimpleTest : public Test
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(SimpleTest)

	// Destructor
	virtual				~SimpleTest() override;

	// See: Test
	virtual void		Initialize() override;

private:
	// A demo of the activation listener
	class Listener : public BodyActivationListener
	{
	public:
		virtual void	OnBodyActivated(const BodyID &inBodyID, uint64 inBodyUserData) override
		{
			Trace("Body %d activated", inBodyID.GetIndex());
		}

		virtual void	OnBodyDeactivated(const BodyID &inBodyID, uint64 inBodyUserData) override
		{
			Trace("Body %d deactivated", inBodyID.GetIndex());
		}
	};

	Listener			mBodyActivationListener;
};
