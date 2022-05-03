// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Tests/Test.h>
#include <Jolt/Physics/Collision/ContactListener.h>
#include <Jolt/Physics/Collision/PhysicsMaterialSimple.h>

/// This test demonstrates how you can use a contact listener and your own material definition to get friction and restitution per triangle or sub shape of a compound shape
class FrictionPerTriangleTest : public Test, public ContactListener
{
public:
	JPH_DECLARE_RTTI_VIRTUAL(FrictionPerTriangleTest)

	// See: Test
	virtual void	Initialize() override;

	/// A custom material implementation that stores its own friction and restitution
	/// Note: Make sure you set PhysicsMaterial::sDefault to something your application understands (explicitly check PhysicsMaterial::sDefault to prevent casting to the wrong type in sGetFrictionAndRestitution)
	class MyMaterial : public PhysicsMaterialSimple
	{
	public:
		// Note: Not implementing serialization because we don't serialize this material in this example!

		/// Constructor
					MyMaterial(const string_view &inName, ColorArg inColor, float inFriction, float inRestitution) : PhysicsMaterialSimple(inName, inColor), mFriction(inFriction), mRestitution(inRestitution) { }

		float		mFriction;
		float		mRestitution;
	};

	/// Extract custom friction and restitution from a body and sub shape ID
	static void		sGetFrictionAndRestitution(const Body &inBody, const SubShapeID &inSubShapeID, float &outFriction, float &outRestitution);

	/// Calculates and overrides friction and restitution settings for a contact between two bodies
	static void		sOverrideContactSettings(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings);

	// If this test implements a contact listener, it should be returned here
	virtual ContactListener *GetContactListener() override		{ return this; }

	// See: ContactListener
	virtual void	OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;
	virtual void	OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override;
};
