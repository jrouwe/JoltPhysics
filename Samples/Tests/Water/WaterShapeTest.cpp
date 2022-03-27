// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Water/WaterShapeTest.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/Shape/OffsetCenterOfMassShape.h>
#include <Layers.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(WaterShapeTest) 
{ 
	JPH_ADD_BASE_CLASS(WaterShapeTest, Test) 
}

void WaterShapeTest::Initialize()
{
	CreateFloor();
	   
	// Create scaled box
	BodyID body_id = mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShape(new BoxShape(Vec3(1.0f, 2.0f, 2.5f)), Vec3(0.5f, 0.6f, -0.7f)), Vec3(-10, 20, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING))->GetID();
	mBodyInterface->AddBody(body_id, EActivation::Activate);

	// Create box
	body_id = mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(1.0f, 2.0f, 2.5f)), Vec3(-7, 20, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING))->GetID();
	mBodyInterface->AddBody(body_id, EActivation::Activate);

	// Create sphere
	body_id = mBodyInterface->CreateBody(BodyCreationSettings(new SphereShape(2.0f), Vec3(-3, 20, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING))->GetID();
	mBodyInterface->AddBody(body_id, EActivation::Activate);

	// Create static compound
	Ref<StaticCompoundShapeSettings> static_compound = new StaticCompoundShapeSettings;
	static_compound->AddShape(Vec3(2.0f, 0, 0), Quat::sIdentity(), new SphereShape(2.0f));
	static_compound->AddShape(Vec3(-1.0f, 0, 0), Quat::sIdentity(), new SphereShape(1.0f));

	body_id = mBodyInterface->CreateBody(BodyCreationSettings(static_compound, Vec3(3, 20, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING))->GetID();
	mBodyInterface->AddBody(body_id, EActivation::Activate);

	// Create tetrahedron
	vector<Vec3> tetrahedron;
	tetrahedron.push_back(Vec3(-2, 0, -2));
	tetrahedron.push_back(Vec3(0, 0, 2));
	tetrahedron.push_back(Vec3(2, 0, -2));
	tetrahedron.push_back(Vec3(0, -2, 0));
	Ref<ConvexHullShapeSettings> tetrahedron_shape = new ConvexHullShapeSettings(tetrahedron);
	body_id = mBodyInterface->CreateBody(BodyCreationSettings(tetrahedron_shape, Vec3(10, 20, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING))->GetID();
	mBodyInterface->AddBody(body_id, EActivation::Activate);

	// Non-uniform scaled tetrahedron
	body_id = mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(tetrahedron_shape, Vec3(1, -1.5f, 2.0f)), Vec3(15, 20, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING))->GetID();
	mBodyInterface->AddBody(body_id, EActivation::Activate);

	// Create convex hull box
	vector<Vec3> box;
	box.push_back(Vec3(1.5f, 1.0f, 0.5f));
	box.push_back(Vec3(-1.5f, 1.0f, 0.5f));
	box.push_back(Vec3(1.5f, -1.0f, 0.5f));
	box.push_back(Vec3(-1.5f, -1.0f, 0.5f));
	box.push_back(Vec3(1.5f, 1.0f, -0.5f));
	box.push_back(Vec3(-1.5f, 1.0f, -0.5f));
	box.push_back(Vec3(1.5f, -1.0f, -0.5f));
	box.push_back(Vec3(-1.5f, -1.0f, -0.5f));
	body_id = mBodyInterface->CreateBody(BodyCreationSettings(new ConvexHullShapeSettings(box), Vec3(18, 20, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING))->GetID();
	mBodyInterface->AddBody(body_id, EActivation::Activate);

	// Create random convex shape
	default_random_engine random;
	uniform_real_distribution<float> hull_size(0.1f, 1.9f);
	vector<Vec3> points;
	for (int j = 0; j < 20; ++j)
		points.push_back(hull_size(random) * Vec3::sRandom(random));
	body_id = mBodyInterface->CreateBody(BodyCreationSettings(new ConvexHullShapeSettings(points), Vec3(21, 20, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING))->GetID();
	mBodyInterface->AddBody(body_id, EActivation::Activate);

	// Create mutable compound
	Ref<MutableCompoundShapeSettings> mutable_compound = new MutableCompoundShapeSettings;
	mutable_compound->AddShape(Vec3(1.0f, 0, 0), Quat::sIdentity(), new BoxShape(Vec3(0.5f, 0.75f, 1.0f)));
	mutable_compound->AddShape(Vec3(-1.0f, 0, 0), Quat::sIdentity(), new SphereShape(1.0f));

	body_id = mBodyInterface->CreateBody(BodyCreationSettings(mutable_compound, Vec3(25, 20, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING))->GetID();
	mBodyInterface->AddBody(body_id, EActivation::Activate);

	// Create box with center of mass offset
	body_id = mBodyInterface->CreateBody(BodyCreationSettings(new OffsetCenterOfMassShapeSettings(Vec3(-1.0f, 0.0f, 0.0f), new BoxShape(Vec3(2.0f, 0.25f, 0.25f))), Vec3(30, 20, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING))->GetID();
	mBodyInterface->AddBody(body_id, EActivation::Activate);
}

void WaterShapeTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Draw the water surface 5mm below actual surface to avoid z fighting with intersection shapes
	Vec3 surface_point = Vec3(0, 10, 0);
	for (int i = -20; i <= 20; ++i)
	{
		mDebugRenderer->DrawLine(surface_point + Vec3(5.0f * i, 0, -100), surface_point + Vec3(5.0f * i, 0, 100), Color::sBlue);
		mDebugRenderer->DrawLine(surface_point + Vec3(-100, 0, 5.0f * i), surface_point + Vec3(100, 0, 5.0f * i), Color::sBlue);
	}

	// Broadphase results, will apply buoyancy to any body that intersects with the water volume
	class MyCollector : public CollideShapeBodyCollector 
	{
	public:
								MyCollector(PhysicsSystem *inSystem, const Plane &inSurface, float inDeltaTime) : mSystem(inSystem), mSurface(inSurface), mDeltaTime(inDeltaTime) { }

		virtual void			AddHit(const BodyID &inBodyID) override
		{
			BodyLockWrite lock(mSystem->GetBodyLockInterface(), inBodyID);
			Body &body = lock.GetBody();
			if (body.IsActive())
				body.ApplyBuoyancyImpulse(mSurface, 1.1f, 0.3f, 0.05f, Vec3::sZero(), mSystem->GetGravity(), mDeltaTime);
		}

	private:
		PhysicsSystem *			mSystem;
		Plane					mSurface;
		float					mDeltaTime;
	};
	Plane surface = Plane::sFromPointAndNormal(surface_point, Vec3::sAxisY());
	MyCollector collector(mPhysicsSystem, surface, inParams.mDeltaTime);
	
	// Apply buoyancy to all bodies that intersect with the water
	AABox water_box(surface_point - Vec3(100, 100, 100), surface_point + Vec3(100, 0, 100));
	mPhysicsSystem->GetBroadPhaseQuery().CollideAABox(water_box, collector, SpecifiedBroadPhaseLayerFilter(BroadPhaseLayers::MOVING), SpecifiedObjectLayerFilter(Layers::MOVING));
}
