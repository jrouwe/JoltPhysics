// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Constraints/ConstraintVsCOMChangeTest.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Constraints/HingeConstraint.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ConstraintVsCOMChangeTest)
{
	JPH_ADD_BASE_CLASS(ConstraintVsCOMChangeTest, Test)
}

void ConstraintVsCOMChangeTest::Initialize()
{
	constexpr int cChainLength = 15;
	constexpr float cMinAngle = DegreesToRadians(-10.0f);
	constexpr float cMaxAngle = DegreesToRadians(20.0f);

	// Floor
	CreateFloor();

	// Create box shape
	mBox = new BoxShape(Vec3::sReplicate(0.5f * cBoxSize));

	// Build a collision group filter that disables collision between adjacent bodies
	Ref<GroupFilterTable> group_filter = new GroupFilterTable(cChainLength);
	for (CollisionGroup::SubGroupID i = 0; i < cChainLength - 1; ++i)
		group_filter->DisableCollision(i, i + 1);

	// Create chain of bodies
	RVec3 position(0, 25, 0);
	for (int i = 0; i < cChainLength; ++i)
	{
		position += Vec3(cBoxSize, 0, 0);
		Quat rotation = Quat::sIdentity();

		// Create compound shape specific for this body
		MutableCompoundShapeSettings compound_shape;
		compound_shape.SetEmbedded();
		compound_shape.AddShape(Vec3::sZero(), Quat::sIdentity(), mBox);

		// Create body
		Body& segment = *mBodyInterface->CreateBody(BodyCreationSettings(&compound_shape, position, rotation, i == 0 ? EMotionType::Static : EMotionType::Dynamic, i == 0 ? Layers::NON_MOVING : Layers::MOVING));
		segment.SetCollisionGroup(CollisionGroup(group_filter, 0, CollisionGroup::SubGroupID(i)));
		mBodyInterface->AddBody(segment.GetID(), EActivation::Activate);

		if (i > 0)
		{
			// Create hinge
			HingeConstraintSettings settings;
			settings.mPoint1 = settings.mPoint2 = position + Vec3(-0.5f * cBoxSize, -0.5f * cBoxSize, 0);
			settings.mHingeAxis1 = settings.mHingeAxis2 = Vec3::sAxisZ();
			settings.mNormalAxis1 = settings.mNormalAxis2 = Vec3::sAxisX();
			settings.mLimitsMin = cMinAngle;
			settings.mLimitsMax = cMaxAngle;
			Constraint* constraint = settings.Create(*mBodies.back(), segment);
			mPhysicsSystem->AddConstraint(constraint);

			mConstraints.push_back(constraint);
		}

		mBodies.push_back(&segment);
	}
}

void ConstraintVsCOMChangeTest::PrePhysicsUpdate(const PreUpdateParams& inParams)
{
	// Increment time
	mTime += inParams.mDeltaTime;

	UpdateShapes();
}

void ConstraintVsCOMChangeTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mTime);
}

void ConstraintVsCOMChangeTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mTime);

	UpdateShapes();
}

void ConstraintVsCOMChangeTest::UpdateShapes()
{
	// Check if we need to change the configuration
	int num_shapes = int(mTime) & 1? 2 : 1;
	if (mNumShapes != num_shapes)
	{
		mNumShapes = num_shapes;

		// Change the COM of the bodies
		for (int i = 1; i < (int)mBodies.size(); i += 2)
		{
			Body *b = mBodies[i];
			MutableCompoundShape *s = static_cast<MutableCompoundShape *>(const_cast<Shape *>(b->GetShape()));

			// Remember the center of mass before the change
			Vec3 prev_com = s->GetCenterOfMass();

			// First remove all existing shapes
			for (int j = s->GetNumSubShapes() - 1; j >= 0; --j)
				s->RemoveShape(j);

			// Then create the desired number of shapes
			for (int j = 0; j < num_shapes; ++j)
				s->AddShape(Vec3(0, 0, (1.0f + cBoxSize) * j), Quat::sIdentity(), mBox);

			// Update the center of mass to account for the new box configuration
			s->AdjustCenterOfMass();

			// Notify the physics system that the shape has changed
			mBodyInterface->NotifyShapeChanged(b->GetID(), prev_com, true, EActivation::Activate);

			// Notify the constraints that the shape has changed (this could be done more efficient as we know which constraints are affected)
			Vec3 delta_com = s->GetCenterOfMass() - prev_com;
			for (Constraint *c : mConstraints)
				c->NotifyShapeChanged(b->GetID(), delta_com);
		}
	}
}
