// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/MutableCompoundShapeTest.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Core/StreamWrapper.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(MutableCompoundShapeTest) 
{ 
	JPH_ADD_BASE_CLASS(MutableCompoundShapeTest, Test) 
}

void MutableCompoundShapeTest::Initialize() 
{
	// Floor (extra thick because we can randomly add sub shapes that then may stick out underneath the floor and cause objects to be pushed through)
	Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(100.0f, 10.0f, 100.0f), 0.0f), Vec3(0.0f, -10.0f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);
		
	// Compound with sub compound and rotation
	StaticCompoundShapeSettings sub_compound_settings;
	sub_compound_settings.AddShape(Vec3(0, 1.5f, 0), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), new BoxShape(Vec3(1.5f, 0.25f, 0.2f)));
	sub_compound_settings.AddShape(Vec3(1.5f, 0, 0), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), new CylinderShape(1.5f, 0.2f));
	sub_compound_settings.AddShape(Vec3(0, 0, 1.5f), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), new TaperedCapsuleShapeSettings(1.5f, 0.25f, 0.2f));
	mSubCompound = sub_compound_settings.Create().Get();

	for (int i = 0; i < 10; ++i)
	{
		// Create a mutable compound per body and fill it up with 2 shapes initially
		Ref<MutableCompoundShapeSettings> compound_shape = new MutableCompoundShapeSettings;
		compound_shape->AddShape(Vec3::sZero(), Quat::sRotation(Vec3::sAxisX(), -0.25f * JPH_PI) * Quat::sRotation(Vec3::sAxisZ(), 0.25f * JPH_PI), mSubCompound);
		compound_shape->AddShape(Vec3::sZero(), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI) * Quat::sRotation(Vec3::sAxisZ(), -0.75f * JPH_PI), mSubCompound);

		// Create a body
		Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(compound_shape, Vec3(0, 10.0f + 5.0f * i, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
		mBodyIDs.push_back(body.GetID());
	}
}

void MutableCompoundShapeTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	BodyInterface &no_lock = mPhysicsSystem->GetBodyInterfaceNoLock();

	uniform_real_distribution<float> roll_distribution(0, 1);

	for (BodyID id : mBodyIDs)
	{
		BodyLockWrite lock(mPhysicsSystem->GetBodyLockInterface(), id);
		if (lock.Succeeded())
		{
			Body &body = lock.GetBody();

			// Get the shape
			MutableCompoundShape *shape = static_cast<MutableCompoundShape *>(const_cast<Shape *>(body.GetShape()));

			// Remember center of mass from before changes
			Vec3 old_com = shape->GetCenterOfMass();

			// Consistently seeded random engine so that bodies move in a predictable way
			default_random_engine consistent_random; 

			// Simulate an engine data structure with strided positions/rotations
			struct PositionRotation
			{
				Vec3	mPosition;
				Quat	mRotation;
			};
			vector<PositionRotation> pos_rot;

			// Animate sub shapes
			uint count = shape->GetNumSubShapes();
			for (uint i = 0; i < count; ++i)
			{
				const CompoundShape::SubShape &sub_shape = shape->GetSubShape(i);
				pos_rot.push_back({ Vec3::sZero(), (Quat::sRotation(Vec3::sRandom(consistent_random), DegreesToRadians(10.0f) * inParams.mDeltaTime) * sub_shape.GetRotation()).Normalized() });
			}

			// Set the new rotations/orientations on the sub shapes
			shape->ModifyShapes(0, count, &pos_rot.front().mPosition, &pos_rot.front().mRotation, sizeof(PositionRotation), sizeof(PositionRotation));

			// Initialize frame dependent random number generator
			default_random_engine frame_random(mFrameNumber++);

			// Roll the dice
			float roll = roll_distribution(frame_random);
			if (roll < 0.001f && count > 1)
			{
				// Remove a random shape
				uniform_int_distribution<uint> index_distribution(0, count - 1);
				shape->RemoveShape(index_distribution(frame_random));
			}
			else if (roll < 0.002f && count < 10)
			{
				// Add a shape in a random rotation
				shape->AddShape(Vec3::sZero(), Quat::sRandom(frame_random), mSubCompound);
			}

			// Ensure that the center of mass is updated
			shape->AdjustCenterOfMass();

			// Since we're already locking the body, we don't need to lock it again
			// We always update the mass properties of the shape because we're reorienting them every frame
			no_lock.NotifyShapeChanged(id, old_com, true, EActivation::Activate);
		}
	}
}

void MutableCompoundShapeTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mFrameNumber);

	for (BodyID id : mBodyIDs)
	{
		BodyLockRead lock(mPhysicsSystem->GetBodyLockInterface(), id);
		if (lock.Succeeded())
		{
			const Body &body = lock.GetBody();

			// Write the shape as a binary string
			stringstream data;
			StreamOutWrapper stream_out(data);
			body.GetShape()->SaveBinaryState(stream_out);
			inStream.Write(data.str());
		}
	}
}

void MutableCompoundShapeTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mFrameNumber);

	for (BodyID id : mBodyIDs)
	{
		BodyLockWrite lock(mPhysicsSystem->GetBodyLockInterface(), id);
		if (lock.Succeeded())
		{
			Body &body = lock.GetBody();

			// Read the shape as a binary string
			string str;
			if (inStream.IsValidating())
			{
				stringstream data;
				StreamOutWrapper stream_out(data);
				body.GetShape()->SaveBinaryState(stream_out);
				str = data.str();
			}
			inStream.Read(str);

			// Deserialize the shape
			stringstream data(str);
			StreamInWrapper stream_in(data);
			Shape::ShapeResult result = Shape::sRestoreFromBinaryState(stream_in);
			MutableCompoundShape *shape = static_cast<MutableCompoundShape *>(result.Get().GetPtr());

			// Restore the pointers to the sub compound
			ShapeList sub_shapes(shape->GetNumSubShapes(), mSubCompound);
			shape->RestoreSubShapeState(sub_shapes.data(), (uint)sub_shapes.size());

			// Update the shape (we're under lock protection, so use the no lock interface)
			mPhysicsSystem->GetBodyInterfaceNoLock().SetShape(id, shape, false, EActivation::DontActivate);
		}
	}
}
