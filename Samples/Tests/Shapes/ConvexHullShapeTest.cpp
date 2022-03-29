// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/ConvexHullShapeTest.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ConvexHullShapeTest) 
{ 
	JPH_ADD_BASE_CLASS(ConvexHullShapeTest, Test) 
}

void ConvexHullShapeTest::Initialize() 
{
	// Floor
	CreateFloor();

	// Create tetrahedron
	vector<Vec3> tetrahedron;
	tetrahedron.push_back(Vec3(-5, 0, -5));
	tetrahedron.push_back(Vec3(0, 0, 5));
	tetrahedron.push_back(Vec3(5, 0, -5));
	tetrahedron.push_back(Vec3(0, -5, 0));

	Body &body_tetrahedron = *mBodyInterface->CreateBody(BodyCreationSettings(new ConvexHullShapeSettings(tetrahedron), Vec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body_tetrahedron.GetID(), EActivation::Activate);

	// Create box
	vector<Vec3> box;
	box.push_back(Vec3(5, 5, 5));
	box.push_back(Vec3(-5, 5, 5));
	box.push_back(Vec3(5, -5, 5));
	box.push_back(Vec3(-5, -5, 5));
	box.push_back(Vec3(5, 5, -5));
	box.push_back(Vec3(-5, 5, -5));
	box.push_back(Vec3(5, -5, -5));
	box.push_back(Vec3(-5, -5, -5));

	Body &body_box = *mBodyInterface->CreateBody(BodyCreationSettings(new ConvexHullShapeSettings(box), Vec3(20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body_box.GetID(), EActivation::Activate);

	// Add a sphere of many points
	vector <Vec3> sphere;
	for (float theta = 0.0f; theta <= JPH_PI; theta += JPH_PI / 20.0f)
		for (float phi = 0.0f; phi <= 2.0f * JPH_PI; phi += 2.0f * JPH_PI / 20.0f)
			sphere.push_back(5.0f * Vec3::sUnitSpherical(theta, phi));

	Body &body_sphere = *mBodyInterface->CreateBody(BodyCreationSettings(new ConvexHullShapeSettings(sphere), Vec3(40, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body_sphere.GetID(), EActivation::Activate);

	// Add a tapered cylinder of many points
	vector <Vec3> tapered_cylinder;
	for (float theta = 0.0f; theta <= 2.0f * JPH_PI; theta += JPH_PI / 128.0f)
	{
		tapered_cylinder.push_back(4.0f * Vec3(-0.1f, sin(theta), cos(theta)));
		tapered_cylinder.push_back(4.5f * Vec3(0.1f, sin(theta), cos(theta)));
	}

	Body &body_tapered_cylinder = *mBodyInterface->CreateBody(BodyCreationSettings(new ConvexHullShapeSettings(tapered_cylinder), Vec3(60, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body_tapered_cylinder.GetID(), EActivation::Activate);

	// Create convex hull with on one side nearly coplanar faces
	vector<Vec3> coplanar;
	coplanar.push_back(Vec3(1.04298747f, 4.68531752f, 0.858853102f));
	coplanar.push_back(Vec3(-1.00753999f, 4.63935566f, -0.959064901f));
	coplanar.push_back(Vec3(-1.01861656f, 4.72096348f, 0.846121550f));
	coplanar.push_back(Vec3(-2.37996006f, 1.26311386f, -1.10994697f));
	coplanar.push_back(Vec3(0.213164970f, 0.0198628306f, -1.70677519f));
	coplanar.push_back(Vec3(-2.27295995f, -0.899001241f, -0.472913086f));
	coplanar.push_back(Vec3(-1.85078228f, -1.25204790f, 2.42339849f));
	coplanar.push_back(Vec3(1.91183412f, -1.25204790f, 2.42339849f));
	coplanar.push_back(Vec3(-2.75279832f, 3.25019693f, 1.67055058f));
	coplanar.push_back(Vec3(-0.0697868019f, -2.78841114f, -0.422013819f));
	coplanar.push_back(Vec3(2.26410985f, -0.918261647f, -0.493922710f));
	coplanar.push_back(Vec3(0.765828013f, -2.82050991f, 1.91100550f));
	coplanar.push_back(Vec3(2.33326006f, 1.26643038f, -1.18808103f));
	coplanar.push_back(Vec3(-0.591650009f, 2.27845216f, -1.87628603f));
	coplanar.push_back(Vec3(-2.22145009f, 3.04359150f, 0.234738767f));
	coplanar.push_back(Vec3(-1.00753999f, 4.39097166f, -1.27783847f));
	coplanar.push_back(Vec3(0.995577991f, 4.39734173f, -1.27900386f));
	coplanar.push_back(Vec3(0.995577991f, 4.64572525f, -0.960230291f));
	coplanar.push_back(Vec3(2.74527335f, 3.06491613f, 1.77647924f));
	coplanar.push_back(Vec3(-1.53122997f, -2.18120861f, 2.31516361f));

	Body &body_coplanar = *mBodyInterface->CreateBody(BodyCreationSettings(new ConvexHullShapeSettings(coplanar), Vec3(80, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body_coplanar.GetID(), EActivation::Activate);

	// Bodies with random convex shapes
	default_random_engine random;
	uniform_real_distribution<float> hull_size(0.1f, 10.0f);
	for (int i = 0; i < 10; ++i)
	{
		// Create random points
		vector<Vec3> points;
		for (int j = 0; j < 20; ++j)
			points.push_back(hull_size(random) * Vec3::sRandom(random));

		Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(new ConvexHullShapeSettings(points), Vec3(-90.0f + i * 18.0f, 10, 20), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
	}

	// Bodies with random convex polygons (this is not something you should be doing, but this tests the 2D convex hull shape generation and allows you to test the probe against them)
	for (int i = 0; i < 10; ++i)
	{
		// Create random points
		vector<Vec3> points;
		for (int j = 0; j < 20; ++j)
		{
			Vec3 v = hull_size(random) * Vec3::sRandom(random);
			v.SetZ(0.0f);
			points.push_back(v);
		}

		// Convex hull needs to be created with convex radius of 0 because the shape has no volume, so we cannot move the planes backwards to make space for the convex radius
		Ref<ShapeSettings> shape_settings = new ConvexHullShapeSettings(points, 0.0f);

		BodyCreationSettings creation_settings(shape_settings, Vec3(-90.0f + i * 18.0f, 10, 40), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);

		// The polygon has no volume, so we need to provide a dummy mass and inertia for this shape
		creation_settings.mOverrideMassProperties = EOverrideMassProperties::MassAndInertiaProvided;
		creation_settings.mMassPropertiesOverride.mMass = 1.0f;
		creation_settings.mMassPropertiesOverride.mInertia = Mat44::sIdentity();

		Body &body = *mBodyInterface->CreateBody(creation_settings);
		mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
	}
}
