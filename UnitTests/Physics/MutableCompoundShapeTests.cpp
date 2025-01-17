// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CollidePointResult.h>

TEST_SUITE("MutableCompoundShapeTests")
{
	TEST_CASE("TestMutableCompoundShapeAddRemove")
	{
		MutableCompoundShapeSettings settings;
		Ref<Shape> sphere1 = new SphereShape(1.0f);
		settings.AddShape(Vec3::sZero(), Quat::sIdentity(), sphere1);
		Ref<MutableCompoundShape> shape = StaticCast<MutableCompoundShape>(settings.Create().Get());

		auto check_shape_hit = [shape] (Vec3Arg inPosition) {
			AllHitCollisionCollector<CollidePointCollector> collector;
			shape->CollidePoint(inPosition - shape->GetCenterOfMass(), SubShapeIDCreator(), collector);
			SubShapeID remainder;
			CHECK(collector.mHits.size() <= 1);
			return !collector.mHits.empty()? shape->GetSubShape(shape->GetSubShapeIndexFromID(collector.mHits[0].mSubShapeID2, remainder)).mShape : nullptr;
		};

		CHECK(shape->GetNumSubShapes() == 1);
		CHECK(shape->GetSubShape(0).mShape == sphere1);
		CHECK(shape->GetLocalBounds() == AABox(Vec3(-1, -1, -1), Vec3(1, 1, 1)));
		CHECK(check_shape_hit(Vec3::sZero()) == sphere1);

		Ref<Shape> sphere2 = new SphereShape(2.0f);
		shape->AddShape(Vec3(10, 0, 0), Quat::sIdentity(), sphere2, 0, 0); // Insert at the start
		CHECK(shape->GetNumSubShapes() == 2);
		CHECK(shape->GetSubShape(0).mShape == sphere2);
		CHECK(shape->GetSubShape(1).mShape == sphere1);
		CHECK(shape->GetLocalBounds() == AABox(Vec3(-1, -2, -2), Vec3(12, 2, 2)));
		CHECK(check_shape_hit(Vec3::sZero()) == sphere1);
		CHECK(check_shape_hit(Vec3(10, 0, 0)) == sphere2);

		Ref<Shape> sphere3 = new SphereShape(3.0f);
		shape->AddShape(Vec3(20, 0, 0), Quat::sIdentity(), sphere3, 0, 2); // Insert at the end
		CHECK(shape->GetNumSubShapes() == 3);
		CHECK(shape->GetSubShape(0).mShape == sphere2);
		CHECK(shape->GetSubShape(1).mShape == sphere1);
		CHECK(shape->GetSubShape(2).mShape == sphere3);
		CHECK(shape->GetLocalBounds() == AABox(Vec3(-1, -3, -3), Vec3(23, 3, 3)));
		CHECK(check_shape_hit(Vec3::sZero()) == sphere1);
		CHECK(check_shape_hit(Vec3(10, 0, 0)) == sphere2);
		CHECK(check_shape_hit(Vec3(20, 0, 0)) == sphere3);

		shape->RemoveShape(1);
		CHECK(shape->GetNumSubShapes() == 2);
		CHECK(shape->GetSubShape(0).mShape == sphere2);
		CHECK(shape->GetSubShape(1).mShape == sphere3);
		CHECK(shape->GetLocalBounds() == AABox(Vec3(8, -3, -3), Vec3(23, 3, 3)));
		CHECK(check_shape_hit(Vec3(0, 0, 0)) == nullptr);
		CHECK(check_shape_hit(Vec3(10, 0, 0)) == sphere2);
		CHECK(check_shape_hit(Vec3(20, 0, 0)) == sphere3);

		Ref<Shape> sphere4 = new SphereShape(4.0f);
		shape->AddShape(Vec3(0, 0, 0), Quat::sIdentity(), sphere4, 0); // Insert at the end
		CHECK(shape->GetNumSubShapes() == 3);
		CHECK(shape->GetSubShape(0).mShape == sphere2);
		CHECK(shape->GetSubShape(1).mShape == sphere3);
		CHECK(shape->GetSubShape(2).mShape == sphere4);
		CHECK(shape->GetLocalBounds() == AABox(Vec3(-4, -4, -4), Vec3(23, 4, 4)));
		CHECK(check_shape_hit(Vec3::sZero()) == sphere4);
		CHECK(check_shape_hit(Vec3(10, 0, 0)) == sphere2);
		CHECK(check_shape_hit(Vec3(20, 0, 0)) == sphere3);

		Ref<Shape> sphere5 = new SphereShape(1.0f);
		shape->AddShape(Vec3(15, 0, 0), Quat::sIdentity(), sphere5, 0, 1); // Insert in the middle
		CHECK(shape->GetNumSubShapes() == 4);
		CHECK(shape->GetSubShape(0).mShape == sphere2);
		CHECK(shape->GetSubShape(1).mShape == sphere5);
		CHECK(shape->GetSubShape(2).mShape == sphere3);
		CHECK(shape->GetSubShape(3).mShape == sphere4);
		CHECK(shape->GetLocalBounds() == AABox(Vec3(-4, -4, -4), Vec3(23, 4, 4)));
		CHECK(check_shape_hit(Vec3::sZero()) == sphere4);
		CHECK(check_shape_hit(Vec3(10, 0, 0)) == sphere2);
		CHECK(check_shape_hit(Vec3(15, 0, 0)) == sphere5);
		CHECK(check_shape_hit(Vec3(20, 0, 0)) == sphere3);

		shape->RemoveShape(3);
		CHECK(shape->GetNumSubShapes() == 3);
		CHECK(shape->GetSubShape(0).mShape == sphere2);
		CHECK(shape->GetSubShape(1).mShape == sphere5);
		CHECK(shape->GetSubShape(2).mShape == sphere3);
		CHECK(shape->GetLocalBounds() == AABox(Vec3(8, -3, -3), Vec3(23, 3, 3)));
		CHECK(check_shape_hit(Vec3::sZero()) == nullptr);
		CHECK(check_shape_hit(Vec3(10, 0, 0)) == sphere2);
		CHECK(check_shape_hit(Vec3(15, 0, 0)) == sphere5);
		CHECK(check_shape_hit(Vec3(20, 0, 0)) == sphere3);

		shape->RemoveShape(1);
		CHECK(shape->GetNumSubShapes() == 2);
		CHECK(shape->GetSubShape(0).mShape == sphere2);
		CHECK(shape->GetSubShape(1).mShape == sphere3);
		CHECK(shape->GetLocalBounds() == AABox(Vec3(8, -3, -3), Vec3(23, 3, 3)));
		CHECK(check_shape_hit(Vec3::sZero()) == nullptr);
		CHECK(check_shape_hit(Vec3(10, 0, 0)) == sphere2);
		CHECK(check_shape_hit(Vec3(15, 0, 0)) == nullptr);
		CHECK(check_shape_hit(Vec3(20, 0, 0)) == sphere3);

		shape->RemoveShape(1);
		CHECK(shape->GetNumSubShapes() == 1);
		CHECK(shape->GetSubShape(0).mShape == sphere2);
		CHECK(shape->GetLocalBounds() == AABox(Vec3(8, -2, -2), Vec3(12, 2, 2)));
		CHECK(check_shape_hit(Vec3::sZero()) == nullptr);
		CHECK(check_shape_hit(Vec3(10, 0, 0)) == sphere2);
		CHECK(check_shape_hit(Vec3(15, 0, 0)) == nullptr);
		CHECK(check_shape_hit(Vec3(20, 0, 0)) == nullptr);

		shape->RemoveShape(0);
		CHECK(shape->GetNumSubShapes() == 0);
		CHECK(!shape->GetLocalBounds().IsValid());
		CHECK(check_shape_hit(Vec3::sZero()) == nullptr);
		CHECK(check_shape_hit(Vec3(10, 0, 0)) == nullptr);
		CHECK(check_shape_hit(Vec3(15, 0, 0)) == nullptr);
		CHECK(check_shape_hit(Vec3(20, 0, 0)) == nullptr);
	}

	TEST_CASE("TestMutableCompoundShapeAdjustCenterOfMass")
	{
		// Start with a box at (-1 0 0)
		MutableCompoundShapeSettings settings;
		Ref<Shape> box_shape1 = new BoxShape(Vec3::sOne());
		box_shape1->SetUserData(1);
		settings.AddShape(Vec3(-1.0f, 0.0f, 0.0f), Quat::sIdentity(), box_shape1);
		Ref<MutableCompoundShape> shape = StaticCast<MutableCompoundShape>(settings.Create().Get());
		CHECK(shape->GetCenterOfMass() == Vec3(-1.0f, 0.0f, 0.0f));
		CHECK(shape->GetLocalBounds() == AABox(Vec3::sReplicate(-1.0f), Vec3::sOne()));

		// Check that we can hit the box
		AllHitCollisionCollector<CollidePointCollector> collector;
		shape->CollidePoint(Vec3(-0.5f, 0.0f, 0.0f) - shape->GetCenterOfMass(), SubShapeIDCreator(), collector);
		CHECK((collector.mHits.size() == 1 && shape->GetSubShapeUserData(collector.mHits[0].mSubShapeID2) == 1));
		collector.Reset();
		CHECK(collector.mHits.empty());

		// Now add another box at (1 0 0)
		Ref<Shape> box_shape2 = new BoxShape(Vec3::sOne());
		box_shape2->SetUserData(2);
		shape->AddShape(Vec3(1.0f, 0.0f, 0.0f), Quat::sIdentity(), box_shape2);
		CHECK(shape->GetCenterOfMass() == Vec3(-1.0f, 0.0f, 0.0f));
		CHECK(shape->GetLocalBounds() == AABox(Vec3(-1.0f, -1.0f, -1.0f), Vec3(3.0f, 1.0f, 1.0f)));

		// Check that we can hit both boxes
		shape->CollidePoint(Vec3(-0.5f, 0.0f, 0.0f) - shape->GetCenterOfMass(), SubShapeIDCreator(), collector);
		CHECK((collector.mHits.size() == 1 && shape->GetSubShapeUserData(collector.mHits[0].mSubShapeID2) == 1));
		collector.Reset();
		shape->CollidePoint(Vec3(0.5f, 0.0f, 0.0f) - shape->GetCenterOfMass(), SubShapeIDCreator(), collector);
		CHECK((collector.mHits.size() == 1 && shape->GetSubShapeUserData(collector.mHits[0].mSubShapeID2) == 2));
		collector.Reset();

		// Adjust the center of mass
		shape->AdjustCenterOfMass();
		CHECK(shape->GetCenterOfMass() == Vec3::sZero());
		CHECK(shape->GetLocalBounds() == AABox(Vec3(-2.0f, -1.0f, -1.0f), Vec3(2.0f, 1.0f, 1.0f)));

		// Check that we can hit both boxes
		shape->CollidePoint(Vec3(-0.5f, 0.0f, 0.0f) - shape->GetCenterOfMass(), SubShapeIDCreator(), collector);
		CHECK((collector.mHits.size() == 1 && shape->GetSubShapeUserData(collector.mHits[0].mSubShapeID2) == 1));
		collector.Reset();
		shape->CollidePoint(Vec3(0.5f, 0.0f, 0.0f) - shape->GetCenterOfMass(), SubShapeIDCreator(), collector);
		CHECK((collector.mHits.size() == 1 && shape->GetSubShapeUserData(collector.mHits[0].mSubShapeID2) == 2));
		collector.Reset();
	}
}
