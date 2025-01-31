// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/SimCollideBodyVsBodyTest.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(SimCollideBodyVsBodyTest)
{
	JPH_ADD_BASE_CLASS(SimCollideBodyVsBodyTest, Test)
}

template <class LeafCollector>
static void sCollideBodyVsBodyPerBody(const Body &inBody1, const Body &inBody2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, CollideShapeSettings &ioCollideShapeSettings, CollideShapeCollector &ioCollector, const ShapeFilter &inShapeFilter)
{
	if (inBody1.IsSensor() || inBody2.IsSensor())
	{
		LeafCollector collector;
		SubShapeIDCreator part1, part2;
		CollisionDispatch::sCollideShapeVsShape(inBody1.GetShape(), inBody2.GetShape(), Vec3::sOne(), Vec3::sOne(), inCenterOfMassTransform1, inCenterOfMassTransform2, part1, part2, ioCollideShapeSettings, collector);
		if (collector.HadHit())
			ioCollector.AddHit(collector.mHit);
	}
	else
	{
		// If not a sensor: fall back to the default
		PhysicsSystem::sDefaultSimCollideBodyVsBody(inBody1, inBody2, inCenterOfMassTransform1, inCenterOfMassTransform2, ioCollideShapeSettings, ioCollector, inShapeFilter);
	}
}

template <class LeafCollector>
static void sCollideBodyVsBodyPerLeaf(const Body &inBody1, const Body &inBody2, Mat44Arg inCenterOfMassTransform1, Mat44Arg inCenterOfMassTransform2, CollideShapeSettings &ioCollideShapeSettings, CollideShapeCollector &ioCollector, const ShapeFilter &inShapeFilter)
{
	if (inBody1.IsSensor() || inBody2.IsSensor())
	{
		// Get bounds of both shapes
		AABox bounds1 = inBody1.GetShape()->GetWorldSpaceBounds(inCenterOfMassTransform1, Vec3::sOne());
		AABox bounds2 = inBody2.GetShape()->GetWorldSpaceBounds(inCenterOfMassTransform2, Vec3::sOne());

		// Get leaf shapes that overlap with the bounds of the other shape
		SubShapeIDCreator part1, part2;
		AllHitCollisionCollector<TransformedShapeCollector> leaves1, leaves2;
		inBody1.GetShape()->CollectTransformedShapes(bounds2, inCenterOfMassTransform1.GetTranslation(), inCenterOfMassTransform1.GetQuaternion(), Vec3::sOne(), part1, leaves1, inShapeFilter);
		inBody2.GetShape()->CollectTransformedShapes(bounds1, inCenterOfMassTransform2.GetTranslation(), inCenterOfMassTransform2.GetQuaternion(), Vec3::sOne(), part2, leaves2, inShapeFilter);

		// Now test each leaf shape against each other leaf
		for (const TransformedShape &leaf1 : leaves1.mHits)
			for (const TransformedShape &leaf2 : leaves2.mHits)
			{
				LeafCollector collector;
				CollisionDispatch::sCollideShapeVsShape(leaf1.mShape, leaf2.mShape, leaf1.GetShapeScale(), leaf2.GetShapeScale(), leaf1.GetCenterOfMassTransform().ToMat44(), leaf2.GetCenterOfMassTransform().ToMat44(), leaf1.mSubShapeIDCreator, leaf2.mSubShapeIDCreator, ioCollideShapeSettings, collector);
				if (collector.HadHit())
					ioCollector.AddHit(collector.mHit);
			}
	}
	else
	{
		// If not a sensor: fall back to the default
		PhysicsSystem::sDefaultSimCollideBodyVsBody(inBody1, inBody2, inCenterOfMassTransform1, inCenterOfMassTransform2, ioCollideShapeSettings, ioCollector, inShapeFilter);
	}
}

void SimCollideBodyVsBodyTest::Initialize()
{
	// Create pyramid
	MeshShapeSettings pyramid;
	pyramid.mTriangleVertices = { Float3(1, 0, 1), Float3(1, 0, -1), Float3(-1, 0, -1), Float3(-1, 0, 1), Float3(0, 1, 0) };
	pyramid.mIndexedTriangles = { IndexedTriangle(0, 1, 4), IndexedTriangle(1, 2, 4), IndexedTriangle(2, 3, 4), IndexedTriangle(3, 0, 4) };
	pyramid.SetEmbedded();

	// Create floor of many pyramids
	StaticCompoundShapeSettings compound;
	for (int x = -10; x <= 10; ++x)
		for (int z = -10; z <= 10; ++z)
			compound.AddShape(Vec3(x * 2.0f, 0, z * 2.0f), Quat::sIdentity(), &pyramid);
	compound.SetEmbedded();

	mBodyInterface->CreateAndAddBody(BodyCreationSettings(&compound, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// A kinematic sensor that also detects static bodies
	BodyCreationSettings sensor_settings(new BoxShape(Vec3::sReplicate(10.0f)), RVec3(0, 5, 0), Quat::sIdentity(), EMotionType::Kinematic, Layers::MOVING); // Put in a layer that collides with static
	sensor_settings.mIsSensor = true;
	sensor_settings.mCollideKinematicVsNonDynamic = true;
	sensor_settings.mUseManifoldReduction = false;
	mSensorID = mBodyInterface->CreateAndAddBody(sensor_settings, EActivation::Activate);

	// Dynamic bodies
	for (int i = 0; i < 10; ++i)
		mBodyIDs.push_back(mBodyInterface->CreateAndAddBody(BodyCreationSettings(new BoxShape(Vec3(0.1f, 0.5f, 0.2f)), RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate));
}

void SimCollideBodyVsBodyTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Update time
	mTime += inParams.mDeltaTime;

	const char *mode_string = nullptr;
	int mode = int(mTime / 3.0f) % 5;
	switch (mode)
	{
	default:
		mode_string = "Sensor: Collect all contact points";
		mPhysicsSystem->SetSimCollideBodyVsBody(PhysicsSystem::sDefaultSimCollideBodyVsBody);
		break;

	case 1:
		mode_string = "Sensor: Collect any contact point per body";
		mPhysicsSystem->SetSimCollideBodyVsBody(sCollideBodyVsBodyPerBody<AnyHitCollisionCollector<CollideShapeCollector>>);
		break;

	case 2:
		mode_string = "Sensor: Collect closest contact point per body";
		mPhysicsSystem->SetSimCollideBodyVsBody(sCollideBodyVsBodyPerBody<ClosestHitCollisionCollector<CollideShapeCollector>>);
		break;

	case 3:
		mode_string = "Sensor: Collect any contact point per leaf shape";
		mPhysicsSystem->SetSimCollideBodyVsBody(sCollideBodyVsBodyPerLeaf<AnyHitCollisionCollector<CollideShapeCollector>>);
		break;

	case 4:
		mode_string = "Sensor: Collect closest contact point per leaf shape";
		mPhysicsSystem->SetSimCollideBodyVsBody(sCollideBodyVsBodyPerLeaf<ClosestHitCollisionCollector<CollideShapeCollector>>);
		break;
	}
	DebugRenderer::sInstance->DrawText3D(RVec3(0, 5, 0), mode_string);

	// If the mode changes
	if (mode != mPrevMode)
	{
		// Start all bodies from the top
		for (int i = 0; i < (int)mBodyIDs.size(); ++i)
		{
			BodyID id = mBodyIDs[i];
			mBodyInterface->SetPositionRotationAndVelocity(id, RVec3(-4.9_r + i * 1.0_r, 5.0_r, 0), Quat::sIdentity(), Vec3::sZero(), Vec3::sZero());
			mBodyInterface->ActivateBody(id);
		}

		// Invalidate collisions with sensor to refresh contacts
		mBodyInterface->InvalidateContactCache(mSensorID);

		mPrevMode = mode;
	}
}

void SimCollideBodyVsBodyTest::OnContactAdded(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	mDebugRenderer->DrawWirePolygon(RMat44::sTranslation(inManifold.mBaseOffset), inManifold.mRelativeContactPointsOn1, Color::sRed, 0.1f);
	mDebugRenderer->DrawWirePolygon(RMat44::sTranslation(inManifold.mBaseOffset), inManifold.mRelativeContactPointsOn2, Color::sRed, 0.1f);
}

void SimCollideBodyVsBodyTest::OnContactPersisted(const Body &inBody1, const Body &inBody2, const ContactManifold &inManifold, ContactSettings &ioSettings)
{
	mDebugRenderer->DrawWirePolygon(RMat44::sTranslation(inManifold.mBaseOffset), inManifold.mRelativeContactPointsOn1, Color::sGreen, 0.1f);
	mDebugRenderer->DrawWirePolygon(RMat44::sTranslation(inManifold.mBaseOffset), inManifold.mRelativeContactPointsOn2, Color::sGreen, 0.1f);
}

void SimCollideBodyVsBodyTest::SaveState(StateRecorder &inStream) const
{
	inStream.Write(mPrevMode);
	inStream.Write(mTime);
}

void SimCollideBodyVsBodyTest::RestoreState(StateRecorder &inStream)
{
	inStream.Read(mPrevMode);
	inStream.Read(mTime);
}
