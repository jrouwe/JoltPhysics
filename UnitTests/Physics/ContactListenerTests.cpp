// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include "LoggingContactListener.h"
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

TEST_SUITE("ContactListenerTests")
{
	// Gravity vector
	const Vec3 cGravity = Vec3(0.0f, -9.81f, 0.0f);

	using LogEntry = LoggingContactListener::LogEntry;
	using EType = LoggingContactListener::EType;

	// Let a sphere bounce on the floor with restition = 1
	TEST_CASE("TestContactListenerElastic")
	{
		PhysicsTestContext c;

		const float cSimulationTime = 1.0f;
		const RVec3 cDistanceTraveled = c.PredictPosition(RVec3::sZero(), Vec3::sZero(), cGravity, cSimulationTime);
		const float cFloorHitEpsilon = 1.0e-4f; // Apply epsilon so that we're sure that the collision algorithm will find a collision
		const RVec3 cFloorHitPos(0.0f, 1.0f - cFloorHitEpsilon, 0.0f); // Sphere with radius 1 will hit floor when 1 above the floor
		const RVec3 cInitialPos = cFloorHitPos - cDistanceTraveled;
		const float cPenetrationSlop = c.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Create sphere
		Body &floor = c.CreateFloor();
		Body &body = c.CreateSphere(cInitialPos, 1.0f, EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING);
		body.SetRestitution(1.0f);
		CHECK(floor.GetID() < body.GetID());

		// Simulate until at floor
		c.Simulate(cSimulationTime);

		// Assert collision not yet processed
		CHECK(listener.GetEntryCount() == 0);

		// Simulate one more step to process the collision
		c.Simulate(c.GetDeltaTime());

		// We expect a validate and a contact point added message
		CHECK(listener.GetEntryCount() == 2);
		if (listener.GetEntryCount() == 2)
		{
			// Check validate callback
			const LogEntry &validate = listener.GetEntry(0);
			CHECK(validate.mType == EType::Validate);
			CHECK(validate.mBody1 == body.GetID()); // Dynamic body should always be the 1st
			CHECK(validate.mBody2 == floor.GetID());

			// Check add contact callback
			const LogEntry &add_contact = listener.GetEntry(1);
			CHECK(add_contact.mType == EType::Add);
			CHECK(add_contact.mBody1 == floor.GetID()); // Lowest ID should be first
			CHECK(add_contact.mManifold.mSubShapeID1.GetValue() == SubShapeID().GetValue()); // Floor doesn't have any sub shapes
			CHECK(add_contact.mBody2 == body.GetID()); // Highest ID should be second
			CHECK(add_contact.mManifold.mSubShapeID2.GetValue() == SubShapeID().GetValue()); // Sphere doesn't have any sub shapes
			CHECK_APPROX_EQUAL(add_contact.mManifold.mWorldSpaceNormal, Vec3::sAxisY()); // Normal should move body 2 out of collision
			CHECK(add_contact.mManifold.mRelativeContactPointsOn1.size() == 1);
			CHECK(add_contact.mManifold.mRelativeContactPointsOn2.size() == 1);
			CHECK(add_contact.mManifold.GetWorldSpaceContactPointOn1(0).IsClose(RVec3::sZero(), Square(cPenetrationSlop)));
			CHECK(add_contact.mManifold.GetWorldSpaceContactPointOn2(0).IsClose(RVec3::sZero(), Square(cPenetrationSlop)));
		}
		listener.Clear();

		// Simulate same time, with a fully elastic body we should reach the initial position again
		c.Simulate(cSimulationTime);

		// We should only have a remove contact point
		CHECK(listener.GetEntryCount() == 1);
		if (listener.GetEntryCount() == 1)
		{
			// Check remove contact callback
			const LogEntry &remove = listener.GetEntry(0);
			CHECK(remove.mType == EType::Remove);
			CHECK(remove.mBody1 == floor.GetID()); // Lowest ID should be first
			CHECK(remove.mBody2 == body.GetID()); // Highest ID should be second
		}
	}

	// Let a sphere fall on the floor with restition = 0, then give it horizontal velocity, then take it away from the floor
	TEST_CASE("TestContactListenerInelastic")
	{
		PhysicsTestContext c;

		const float cSimulationTime = 1.0f;
		const RVec3 cDistanceTraveled = c.PredictPosition(RVec3::sZero(), Vec3::sZero(), cGravity, cSimulationTime);
		const float cFloorHitEpsilon = 1.0e-4f; // Apply epsilon so that we're sure that the collision algorithm will find a collision
		const RVec3 cFloorHitPos(0.0f, 1.0f - cFloorHitEpsilon, 0.0f); // Sphere with radius 1 will hit floor when 1 above the floor
		const RVec3 cInitialPos = cFloorHitPos - cDistanceTraveled;
		const float cPenetrationSlop = c.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		// Register listener
		LoggingContactListener listener;
		c.GetSystem()->SetContactListener(&listener);

		// Create sphere
		Body &floor = c.CreateFloor();
		Body &body = c.CreateSphere(cInitialPos, 1.0f, EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING);
		body.SetRestitution(0.0f);
		body.SetAllowSleeping(false);
		CHECK(floor.GetID() < body.GetID());

		// Simulate until at floor
		c.Simulate(cSimulationTime);

		// Assert collision not yet processed
		CHECK(listener.GetEntryCount() == 0);

		// Simulate one more step to process the collision
		c.Simulate(c.GetDeltaTime());
		CHECK_APPROX_EQUAL(body.GetPosition(), cFloorHitPos, cPenetrationSlop);

		// We expect a validate and a contact point added message
		CHECK(listener.GetEntryCount() == 2);
		if (listener.GetEntryCount() == 2)
		{
			// Check validate callback
			const LogEntry &validate = listener.GetEntry(0);
			CHECK(validate.mType == EType::Validate);
			CHECK(validate.mBody1 == body.GetID()); // Dynamic body should always be the 1st
			CHECK(validate.mBody2 == floor.GetID());

			// Check add contact callback
			const LogEntry &add_contact = listener.GetEntry(1);
			CHECK(add_contact.mType == EType::Add);
			CHECK(add_contact.mBody1 == floor.GetID()); // Lowest ID first
			CHECK(add_contact.mManifold.mSubShapeID1.GetValue() == SubShapeID().GetValue()); // Floor doesn't have any sub shapes
			CHECK(add_contact.mBody2 == body.GetID()); // Highest ID second
			CHECK(add_contact.mManifold.mSubShapeID2.GetValue() == SubShapeID().GetValue()); // Sphere doesn't have any sub shapes
			CHECK_APPROX_EQUAL(add_contact.mManifold.mWorldSpaceNormal, Vec3::sAxisY()); // Normal should move body 2 out of collision
			CHECK(add_contact.mManifold.mRelativeContactPointsOn1.size() == 1);
			CHECK(add_contact.mManifold.mRelativeContactPointsOn2.size() == 1);
			CHECK(add_contact.mManifold.GetWorldSpaceContactPointOn1(0).IsClose(RVec3::sZero(), Square(cPenetrationSlop)));
			CHECK(add_contact.mManifold.GetWorldSpaceContactPointOn2(0).IsClose(RVec3::sZero(), Square(cPenetrationSlop)));
		}
		listener.Clear();

		// Simulate 10 steps
		c.Simulate(10 * c.GetDeltaTime());
		CHECK_APPROX_EQUAL(body.GetPosition(), cFloorHitPos, cPenetrationSlop);

		// We're not moving, we should have persisted contacts only
		CHECK(listener.GetEntryCount() == 10);
		for (size_t i = 0; i < listener.GetEntryCount(); ++i)
		{
			// Check persist callback
			const LogEntry &persist_contact = listener.GetEntry(i);
			CHECK(persist_contact.mType == EType::Persist);
			CHECK(persist_contact.mBody1 == floor.GetID()); // Lowest ID first
			CHECK(persist_contact.mManifold.mSubShapeID1.GetValue() == SubShapeID().GetValue()); // Floor doesn't have any sub shapes
			CHECK(persist_contact.mBody2 == body.GetID()); // Highest ID second
			CHECK(persist_contact.mManifold.mSubShapeID2.GetValue() == SubShapeID().GetValue()); // Sphere doesn't have any sub shapes
			CHECK_APPROX_EQUAL(persist_contact.mManifold.mWorldSpaceNormal, Vec3::sAxisY()); // Normal should move body 2 out of collision
			CHECK(persist_contact.mManifold.mRelativeContactPointsOn1.size() == 1);
			CHECK(persist_contact.mManifold.mRelativeContactPointsOn2.size() == 1);
			CHECK(persist_contact.mManifold.GetWorldSpaceContactPointOn1(0).IsClose(RVec3::sZero(), Square(cPenetrationSlop)));
			CHECK(persist_contact.mManifold.GetWorldSpaceContactPointOn2(0).IsClose(RVec3::sZero(), Square(cPenetrationSlop)));
		}
		listener.Clear();

		// Make the body able to go to sleep
		body.SetAllowSleeping(true);

		// Let the body go to sleep
		c.Simulate(1.0f);
		CHECK_APPROX_EQUAL(body.GetPosition(), cFloorHitPos, cPenetrationSlop);

		// Check it went to sleep and that we received a contact removal callback
		CHECK(!body.IsActive());
		CHECK(listener.GetEntryCount() > 0);
		for (size_t i = 0; i < listener.GetEntryCount(); ++i)
		{
			// Check persist / removed callbacks
			const LogEntry &entry = listener.GetEntry(i);
			CHECK(entry.mBody1 == floor.GetID());
			CHECK(entry.mBody2 == body.GetID());
			CHECK(entry.mType == ((i == listener.GetEntryCount() - 1)? EType::Remove : EType::Persist)); // The last entry should remove the contact as the body went to sleep
		}
		listener.Clear();

		// Wake the body up again
		c.GetBodyInterface().ActivateBody(body.GetID());
		CHECK(body.IsActive());

		// Simulate 1 time step to detect the collision with the floor again
		c.SimulateSingleStep();

		// Check that the contact got readded
		CHECK(listener.GetEntryCount() == 2);
		CHECK(listener.Contains(EType::Validate, floor.GetID(), body.GetID()));
		CHECK(listener.Contains(EType::Add, floor.GetID(), body.GetID()));
		listener.Clear();

		// Prevent body from going to sleep again
		body.SetAllowSleeping(false);

		// Make the sphere move horizontal
		body.SetLinearVelocity(Vec3::sAxisX());

		// Simulate 10 steps
		c.Simulate(10 * c.GetDeltaTime());

		// We should have 10 persisted contacts events
		int validate = 0;
		int persisted = 0;
		for (size_t i = 0; i < listener.GetEntryCount(); ++i)
		{
			const LogEntry &entry = listener.GetEntry(i);
			switch (entry.mType)
			{
			case EType::Validate:
				++validate;
				break;

			case EType::Persist:
				// Check persist callback
				CHECK(entry.mBody1 == floor.GetID()); // Lowest ID first
				CHECK(entry.mManifold.mSubShapeID1.GetValue() == SubShapeID().GetValue()); // Floor doesn't have any sub shapes
				CHECK(entry.mBody2 == body.GetID()); // Highest ID second
				CHECK(entry.mManifold.mSubShapeID2.GetValue() == SubShapeID().GetValue()); // Sphere doesn't have any sub shapes
				CHECK_APPROX_EQUAL(entry.mManifold.mWorldSpaceNormal, Vec3::sAxisY()); // Normal should move body 2 out of collision
				CHECK(entry.mManifold.mRelativeContactPointsOn1.size() == 1);
				CHECK(entry.mManifold.mRelativeContactPointsOn2.size() == 1);
				CHECK(abs(entry.mManifold.GetWorldSpaceContactPointOn1(0).GetY()) < cPenetrationSlop);
				CHECK(abs(entry.mManifold.GetWorldSpaceContactPointOn2(0).GetY()) < cPenetrationSlop);
				++persisted;
				break;

			case EType::Add:
			case EType::Remove:
			default:
				CHECK(false); // Unexpected event
			}
		}
		CHECK(validate <= 10); // We may receive extra validate callbacks when the object is moving
		CHECK(persisted == 10);
		listener.Clear();

		// Move the sphere away from the floor
		c.GetBodyInterface().SetPosition(body.GetID(), cInitialPos, EActivation::Activate);

		// Simulate 10 steps
		c.Simulate(10 * c.GetDeltaTime());

		// We should only have a remove contact point
		CHECK(listener.GetEntryCount() == 1);
		if (listener.GetEntryCount() == 1)
		{
			// Check remove contact callback
			const LogEntry &remove = listener.GetEntry(0);
			CHECK(remove.mType == EType::Remove);
			CHECK(remove.mBody1 == floor.GetID()); // Lowest ID first
			CHECK(remove.mBody2 == body.GetID()); // Highest ID second
		}
	}

	TEST_CASE("TestWereBodiesInContact")
	{
		for (int sign = -1; sign <= 1; sign += 2)
		{
			PhysicsTestContext c;

			PhysicsSystem *s = c.GetSystem();
			BodyInterface &bi = c.GetBodyInterface();

			Body &floor = c.CreateFloor();

			// Two spheres at a distance so that when one sphere leaves the floor the body can still be touching the floor with the other sphere
			Ref<StaticCompoundShapeSettings> compound_shape = new StaticCompoundShapeSettings;
			compound_shape->AddShape(Vec3(-2, 0, 0), Quat::sIdentity(), new SphereShape(1));
			compound_shape->AddShape(Vec3(2, 0, 0), Quat::sIdentity(), new SphereShape(1));
			Body &body = *bi.CreateBody(BodyCreationSettings(compound_shape, RVec3(0, 0.999f, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
			bi.AddBody(body.GetID(), EActivation::Activate);

			class ContactListenerImpl : public ContactListener
			{
			public:
								ContactListenerImpl(PhysicsSystem *inSystem) : mSystem(inSystem) { }

				virtual void	OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
				{
					++mAdded;
				}

				virtual void	OnContactRemoved(const SubShapeIDPair &inSubShapePair) override
				{
					++mRemoved;
					mWasInContact = mSystem->WereBodiesInContact(inSubShapePair.GetBody1ID(), inSubShapePair.GetBody2ID());
					CHECK(mWasInContact == mSystem->WereBodiesInContact(inSubShapePair.GetBody2ID(), inSubShapePair.GetBody1ID())); // Returned value should be the same regardless of order
				}

				int				GetAddCount() const
				{
					return mAdded - mRemoved;
				}

				void			Reset()
				{
					mAdded = 0;
					mRemoved = 0;
					mWasInContact = false;
				}

				PhysicsSystem *	mSystem;

				int				mAdded = 0;

				int				mRemoved = 0;
				bool			mWasInContact = false;
			};

			// Set listener
			ContactListenerImpl listener(s);
			s->SetContactListener(&listener);

			// If the simulation hasn't run yet, we can't be in contact
			CHECK(!s->WereBodiesInContact(floor.GetID(), body.GetID()));

			// Step the simulation to allow detecting the contact
			c.SimulateSingleStep();

			// Should be in contact now
			CHECK(s->WereBodiesInContact(floor.GetID(), body.GetID()));
			CHECK(s->WereBodiesInContact(body.GetID(), floor.GetID()));
			CHECK(listener.GetAddCount() == 1);
			listener.Reset();

			// Impulse on one side
			bi.AddImpulse(body.GetID(), Vec3(0, 10000, 0), RVec3(Real(-sign * 2), 0, 0));
			c.SimulateSingleStep(); // One step to detach from the ground (but starts penetrating so will not send a remove callback)
			CHECK(listener.GetAddCount() == 0);
			c.SimulateSingleStep(); // One step to get contact remove callback

			// Should still be in contact
			// Note that we may get a remove and an add callback because manifold reduction has combined the collision with both spheres into 1 contact manifold.
			// At that point it has to select one of the sub shapes for the contact and if that sub shape no longer collides we get a remove for this sub shape and then an add callback for the other sub shape.
			CHECK(s->WereBodiesInContact(floor.GetID(), body.GetID()));
			CHECK(s->WereBodiesInContact(body.GetID(), floor.GetID()));
			CHECK(listener.GetAddCount() == 0);
			CHECK((listener.mRemoved == 0 || listener.mWasInContact));
			listener.Reset();

			// Impulse on the other side
			bi.AddImpulse(body.GetID(), Vec3(0, 10000, 0), RVec3(Real(sign * 2), 0, 0));
			c.SimulateSingleStep(); // One step to detach from the ground (but starts penetrating so will not send a remove callback)
			CHECK(listener.GetAddCount() == 0);
			c.SimulateSingleStep(); // One step to get contact remove callback

			// Should no longer be in contact
			CHECK(!s->WereBodiesInContact(floor.GetID(), body.GetID()));
			CHECK(!s->WereBodiesInContact(body.GetID(), floor.GetID()));
			CHECK(listener.GetAddCount() == -1);
			CHECK((listener.mRemoved == 1 && !listener.mWasInContact));
		}
	}

	TEST_CASE("TestSurfaceVelocity")
	{
		PhysicsTestContext c;

		Body &floor = c.CreateBox(RVec3(0, -1, 0), Quat::sRotation(Vec3::sAxisY(), DegreesToRadians(10.0f)), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, Vec3(100.0f, 1.0f, 100.0f));
		floor.SetFriction(1.0f);

		for (int iteration = 0; iteration < 2; ++iteration)
		{
			Body &box = c.CreateBox(RVec3(0, 0.999f, 0), Quat::sRotation(Vec3::sAxisY(), DegreesToRadians(30.0f)), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(1.0f));
			box.SetFriction(1.0f);

			// Contact listener sets a constant surface velocity
			class ContactListenerImpl : public ContactListener
			{
			public:
								ContactListenerImpl(Body &inFloor, Body &inBox) : mFloor(inFloor), mBox(inBox) { }

				virtual void	OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
				{
					// Ensure that the body order is as expected
					JPH_ASSERT(inBody1.GetID() == mFloor.GetID() || inBody2.GetID() == mBox.GetID());

					// Calculate the relative surface velocity
					ioSettings.mRelativeLinearSurfaceVelocity = -(inBody1.GetRotation() * mLocalSpaceLinearVelocity);
					ioSettings.mRelativeAngularSurfaceVelocity = -(inBody1.GetRotation() * mLocalSpaceAngularVelocity);
				}

				virtual void	OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
				{
					OnContactAdded(inBody1, inBody2, inManifold, ioSettings);
				}

				Body &			mFloor;
				Body &			mBox;
				Vec3			mLocalSpaceLinearVelocity;
				Vec3			mLocalSpaceAngularVelocity;
			};

			// Set listener
			ContactListenerImpl listener(floor, box);
			c.GetSystem()->SetContactListener(&listener);

			// Set linear velocity or angular velocity depending on the iteration
			listener.mLocalSpaceLinearVelocity = iteration == 0? Vec3(0, 0, -2.0f) : Vec3::sZero();
			listener.mLocalSpaceAngularVelocity = iteration == 0? Vec3::sZero() : Vec3(0, DegreesToRadians(30.0f), 0);

			// Simulate
			c.Simulate(5.0f);

			// Check that the box is moving with the correct linear/angular velocity
			CHECK_APPROX_EQUAL(box.GetLinearVelocity(), floor.GetRotation() * listener.mLocalSpaceLinearVelocity, 0.005f);
			CHECK_APPROX_EQUAL(box.GetAngularVelocity(), floor.GetRotation() * listener.mLocalSpaceAngularVelocity, 1.0e-4f);
		}
	}

	static float sGetInvMassScale(const Body &inBody)
	{
		uint64 ud = inBody.GetUserData();
		int index = ((ud & 1) != 0? (ud >> 1) : (ud >> 3)) & 0b11;
		float mass_overrides[] = { 1.0f, 0.0f, 0.5f, 2.0f };
		return mass_overrides[index];
	}

	TEST_CASE("TestMassOverride")
	{
		for (EMotionType m1 = EMotionType::Static; m1 <= EMotionType::Dynamic; m1 = EMotionType((int)m1 + 1))
			for (EMotionType m2 = EMotionType::Static; m2 <= EMotionType::Dynamic; m2 = EMotionType((int)m2 + 1))
				if (m1 != EMotionType::Static || m2 != EMotionType::Static)
					for (int i = 0; i < 16; ++i)
					{
						PhysicsTestContext c;
						c.ZeroGravity();

						const float cInitialVelocity1 = m1 != EMotionType::Static? 3.0f : 0.0f;
						const float cInitialVelocity2 = m2 != EMotionType::Static? -4.0f : 0.0f;

						// Create two spheres on a collision course
						BodyCreationSettings bcs(new SphereShape(1.0f), RVec3::sZero(), Quat::sIdentity(), m1, m1 != EMotionType::Static? Layers::MOVING : Layers::NON_MOVING);
						bcs.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
						bcs.mMassPropertiesOverride.mMass = 1.0f;
						bcs.mRestitution = 1.0f;
						bcs.mLinearDamping = 0.0f;
						bcs.mPosition = RVec3(-2, 0, 0);
						bcs.mLinearVelocity = Vec3(cInitialVelocity1, 0, 0);
						bcs.mUserData = i << 1;
						Body &body1 = *c.GetBodyInterface().CreateBody(bcs);
						c.GetBodyInterface().AddBody(body1.GetID(), EActivation::Activate);

						bcs.mMotionType = m2;
						bcs.mObjectLayer = m2 != EMotionType::Static? Layers::MOVING : Layers::NON_MOVING;
						bcs.mMassPropertiesOverride.mMass = 2.0f;
						bcs.mPosition = RVec3(2, 0, 0);
						bcs.mLinearVelocity = Vec3(cInitialVelocity2, 0, 0);
						bcs.mUserData++;
						Body &body2 = *c.GetBodyInterface().CreateBody(bcs);
						c.GetBodyInterface().AddBody(body2.GetID(), EActivation::Activate);

						// Contact listener that modifies mass
						class ContactListenerImpl : public ContactListener
						{
						public:
							virtual void	OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
							{
								// Override the mass of body 1
								float scale1 = sGetInvMassScale(inBody1);
								ioSettings.mInvMassScale1 = scale1;
								ioSettings.mInvInertiaScale1 = scale1;

								// Override the mass of body 2
								float scale2 = sGetInvMassScale(inBody2);
								ioSettings.mInvMassScale2 = scale2;
								ioSettings.mInvInertiaScale2 = scale2;
							}

							virtual void	OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
							{
								OnContactAdded(inBody1, inBody2, inManifold, ioSettings);
							}
						};

						// Set listener
						ContactListenerImpl listener;
						c.GetSystem()->SetContactListener(&listener);

						// Simulate
						c.Simulate(1.0f);

						// Calculate resulting inverse mass
						float inv_m1 = body1.GetMotionType() == EMotionType::Dynamic? sGetInvMassScale(body1) * body1.GetMotionProperties()->GetInverseMass() : 0.0f;
						float inv_m2 = body2.GetMotionType() == EMotionType::Dynamic? sGetInvMassScale(body2) * body2.GetMotionProperties()->GetInverseMass() : 0.0f;

						float v1, v2;
						if (inv_m1 == 0.0f && inv_m2 == 0.0f)
						{
							// If both bodies became kinematic they will pass through each other
							v1 = cInitialVelocity1;
							v2 = cInitialVelocity2;
						}
						else
						{
							// Calculate resulting velocity using conservation of momentum and energy
							// See: https://en.wikipedia.org/wiki/Elastic_collision where m1 = 1 / inv_m1 and m2 = 1 / inv_m2
							v1 = (2.0f * inv_m1 * cInitialVelocity2 + (inv_m2 - inv_m1) * cInitialVelocity1) / (inv_m1 + inv_m2);
							v2 = (2.0f * inv_m2 * cInitialVelocity1 + (inv_m1 - inv_m2) * cInitialVelocity2) / (inv_m1 + inv_m2);
						}

						// Check that the spheres move according to their overridden masses
						CHECK_APPROX_EQUAL(body1.GetLinearVelocity(), Vec3(v1, 0, 0));
						CHECK_APPROX_EQUAL(body2.GetLinearVelocity(), Vec3(v2, 0, 0));
					}
	}

	TEST_CASE("TestInfiniteMassOverride")
	{
		for (bool do_swap : { false, true })
			for (EMotionQuality quality : { EMotionQuality::Discrete, EMotionQuality::LinearCast })
			{
				// A contact listener that makes a body have infinite mass
				class ContactListenerImpl : public ContactListener
				{
				public:
									ContactListenerImpl(const BodyID &inBodyID) : mBodyID(inBodyID) { }

					virtual void	OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
					{
						if (mBodyID == inBody1.GetID())
						{
							ioSettings.mInvInertiaScale1 = 0.0f;
							ioSettings.mInvMassScale1 = 0.0f;
						}
						else if (mBodyID == inBody2.GetID())
						{
							ioSettings.mInvInertiaScale2 = 0.0f;
							ioSettings.mInvMassScale2 = 0.0f;
						}
					}

					virtual void	OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings) override
					{
						OnContactAdded(inBody1, inBody2, inManifold, ioSettings);
					}

				private:
					BodyID			mBodyID;
				};

				PhysicsTestContext c;
				c.ZeroGravity();

				// Create a box
				const RVec3 cInitialBoxPos(0, 2, 0);
				BodyCreationSettings box_settings(new BoxShape(Vec3::sReplicate(2)), cInitialBoxPos, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
				box_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
				box_settings.mMassPropertiesOverride.mMass = 1.0f;

				// Create a sphere
				BodyCreationSettings sphere_settings(new SphereShape(2), RVec3(30, 2, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
				sphere_settings.mLinearVelocity = Vec3(-100, 0, 0);
				sphere_settings.mOverrideMassProperties = EOverrideMassProperties::CalculateInertia;
				sphere_settings.mMassPropertiesOverride.mMass = 10.0f;
				sphere_settings.mRestitution = 0.1f;
				sphere_settings.mLinearDamping = 0.0f;
				sphere_settings.mMotionQuality = quality;

				BodyID box_id, sphere_id;
				if (do_swap)
				{
					// Swap the bodies so that the contact listener will receive the bodies in the opposite order
					sphere_id = c.GetBodyInterface().CreateAndAddBody(sphere_settings, EActivation::Activate);
					box_id = c.GetBodyInterface().CreateAndAddBody(box_settings, EActivation::Activate);
				}
				else
				{
					box_id = c.GetBodyInterface().CreateAndAddBody(box_settings, EActivation::Activate);
					sphere_id = c.GetBodyInterface().CreateAndAddBody(sphere_settings, EActivation::Activate);
				}

				// Add listener that will make the box have infinite mass
				ContactListenerImpl listener(box_id);
				c.GetSystem()->SetContactListener(&listener);

				// Simulate
				const float cSimulationTime = 0.3f;
				c.Simulate(cSimulationTime);

				// Check that the box didn't move
				BodyInterface &bi = c.GetBodyInterface();
				CHECK(bi.GetPosition(box_id) == cInitialBoxPos);
				CHECK(bi.GetLinearVelocity(box_id) == Vec3::sZero());
				CHECK(bi.GetAngularVelocity(box_id) == Vec3::sZero());

				// Check that the sphere bounced off the box
				CHECK_APPROX_EQUAL(bi.GetLinearVelocity(sphere_id), -sphere_settings.mLinearVelocity * sphere_settings.mRestitution);
			}
		}
}
