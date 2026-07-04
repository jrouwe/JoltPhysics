// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include <Jolt/Physics/StateRecorderImpl.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>

TEST_SUITE("StateRecorderTests")
{
	static BodyID sCreateBox(BodyInterface &inBI)
	{
		BodyCreationSettings settings(new BoxShape(Vec3::sReplicate(0.5f)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		Body *body = inBI.CreateBody(settings);
		CHECK(body != nullptr);
		inBI.AddBody(body->GetID(), EActivation::Activate);
		return body->GetID();
	}

	static void sRemoveAndDestroy(BodyInterface &inBI, BodyID inID)
	{
		inBI.RemoveBody(inID);
		inBI.DestroyBody(inID);
	}

	// Test that body id allocation after RestoreState matches the original allocation when restoring with EStateRecorderState::IdSequences
	TEST_CASE("TestBodyIDConsistencyAfterRestore")
	{
		PhysicsTestContext c;
		BodyInterface &bi = c.GetBodyInterface();
		PhysicsSystem *system = c.GetSystem();

		// Create two persistent bodies
		BodyID p0 = sCreateBox(bi);
		BodyID p1 = sCreateBox(bi);
		CHECK(p0.GetIndex() == 0);
		CHECK(p0.GetSequenceNumber() == 1);
		CHECK(p1.GetIndex() == 1);
		CHECK(p1.GetSequenceNumber() == 1);

		c.SimulateSingleStep();

		// Save state with and without sequence numbers
		StateRecorderImpl state_no_seq;
		system->SaveState(state_no_seq, EStateRecorderState::All);
		StateRecorderImpl state_with_seq;
		system->SaveState(state_with_seq, EStateRecorderState::All | EStateRecorderState::IdSequences);

		// Create and destroy a body after the snapshot, this advances the sequence number of slot 2
		BodyID first_transient = sCreateBox(bi);
		CHECK(first_transient.GetIndex() == 2);
		CHECK(first_transient.GetSequenceNumber() == 1);
		sRemoveAndDestroy(bi, first_transient);

		// Restore without sequence numbers: the slot 2 counter is not rolled back so the next body gets a different id
		{
			state_no_seq.Rewind();
			CHECK(system->RestoreState(state_no_seq));

			BodyID after_no_seq = sCreateBox(bi);
			CHECK(after_no_seq.GetIndex() == 2);
			CHECK(after_no_seq.GetSequenceNumber() == 2);
			CHECK(after_no_seq != first_transient);
			sRemoveAndDestroy(bi, after_no_seq);
		}

		// Restore with sequence numbers: the counter is rolled back so the next body gets the same id as the first time
		{
			state_with_seq.Rewind();
			CHECK(system->RestoreState(state_with_seq));

			BodyID after_with_seq = sCreateBox(bi);
			CHECK(after_with_seq.GetIndex() == 2);
			CHECK(after_with_seq.GetSequenceNumber() == 1);
			CHECK(after_with_seq == first_transient);
			sRemoveAndDestroy(bi, after_with_seq);
		}
	}

	// Test that the free list order is restored so that body creation after a restore produces the same ids it did after the snapshot was taken
	TEST_CASE("TestFreeListOrderPreservedAfterRestore")
	{
		PhysicsTestContext c;
		BodyInterface &bi = c.GetBodyInterface();
		PhysicsSystem *system = c.GetSystem();

		// Create and destroy a body so slot 0 is on the free list
		BodyID first = sCreateBox(bi);
		CHECK(first.GetIndex() == 0);
		sRemoveAndDestroy(bi, first);

		c.SimulateSingleStep();

		StateRecorderImpl snapshot;
		system->SaveState(snapshot, EStateRecorderState::All | EStateRecorderState::IdSequences);

		// Create two bodies and destroy them in reverse order so the free list order no longer matches the snapshot
		BodyID b0 = sCreateBox(bi);
		CHECK(b0.GetIndex() == 0);
		CHECK(b0.GetSequenceNumber() == 2);
		BodyID b1 = sCreateBox(bi);
		CHECK(b1.GetIndex() == 1);
		CHECK(b1.GetSequenceNumber() == 1);
		sRemoveAndDestroy(bi, b1);
		sRemoveAndDestroy(bi, b0);

		snapshot.Rewind();
		CHECK(system->RestoreState(snapshot));

		// Creating the same bodies again should produce the same ids
		BodyID r0 = sCreateBox(bi);
		CHECK(r0 == b0);
		BodyID r1 = sCreateBox(bi);
		CHECK(r1 == b1);

		sRemoveAndDestroy(bi, r0);
		sRemoveAndDestroy(bi, r1);
	}

	// Test that bodies not present in the state are destroyed when restoring with inDestroyBodiesNotInState
	TEST_CASE("TestRestoreStateDestroyOrphanBodies")
	{
		PhysicsTestContext c;
		BodyInterface &bi = c.GetBodyInterface();
		PhysicsSystem *system = c.GetSystem();

		BodyID p0 = sCreateBox(bi);
		BodyID p1 = sCreateBox(bi);

		c.SimulateSingleStep();

		StateRecorderImpl snapshot;
		system->SaveState(snapshot, EStateRecorderState::All | EStateRecorderState::IdSequences);
		CHECK(system->GetNumBodies() == 2);

		// Create a body that is not part of the snapshot
		BodyID orphan = sCreateBox(bi);
		CHECK(orphan.GetIndex() == 2);
		CHECK(system->GetNumBodies() == 3);

		SUBCASE("OrphanSurvivesRestore")
		{
			snapshot.Rewind();
			CHECK(system->RestoreState(snapshot, nullptr, false));

			CHECK(system->GetNumBodies() == 3);
			CHECK(bi.IsAdded(orphan));

			sRemoveAndDestroy(bi, orphan);
		}

		SUBCASE("OrphanDestroyedOnRestore")
		{
			snapshot.Rewind();
			CHECK(system->RestoreState(snapshot, nullptr, true));

			CHECK(system->GetNumBodies() == 2);
			CHECK_FALSE(bi.IsAdded(orphan));
			{
				BodyLockRead lock(system->GetBodyLockInterface(), orphan);
				CHECK_FALSE(lock.Succeeded());
			}
			CHECK(bi.IsAdded(p0));
			CHECK(bi.IsAdded(p1));

			// The next body created reuses the orphan slot and gets the same id
			BodyID reuse = sCreateBox(bi);
			CHECK(reuse == orphan);

			sRemoveAndDestroy(bi, reuse);
		}
	}

	// Test that a body whose slot was reused after the snapshot (destroy + create -> same slot), gets its id corrected back to the saved value on restore
	TEST_CASE("TestBodyIDDriftCorrectedOnRestore")
	{
		PhysicsTestContext c;
		BodyInterface &bi = c.GetBodyInterface();
		PhysicsSystem *system = c.GetSystem();

		BodyID a = sCreateBox(bi);
		CHECK(a.GetSequenceNumber() == 1);

		c.SimulateSingleStep();
		RVec3 a_pos = bi.GetPosition(a);

		StateRecorderImpl snapshot;
		system->SaveState(snapshot, EStateRecorderState::All | EStateRecorderState::IdSequences);

		// Destroy the body and create a new one, this reuses the slot with a higher sequence number
		sRemoveAndDestroy(bi, a);
		BodyID b = sCreateBox(bi);
		CHECK(b.GetIndex() == a.GetIndex());
		CHECK(b.GetSequenceNumber() == 2);

		// The body occupying the slots is matched by index and its id is corrected back to the saved value
		snapshot.Rewind();
		CHECK(system->RestoreState(snapshot, nullptr, true));

		CHECK(bi.IsAdded(a));
		CHECK_FALSE(bi.IsAdded(b));
		CHECK(bi.IsActive(a));
		CHECK(bi.GetPosition(a) == a_pos);

		// Id allocation continues deterministically from the snapshot state
		BodyID next = sCreateBox(bi);
		CHECK(next.GetIndex() == 1);
		CHECK(next.GetSequenceNumber() == 1);

		sRemoveAndDestroy(bi, next);
		sRemoveAndDestroy(bi, a);
	}

} // TEST_SUITE("StateRecorderTests")
