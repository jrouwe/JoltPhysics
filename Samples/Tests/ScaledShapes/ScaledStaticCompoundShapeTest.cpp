// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/ScaledStaticCompoundShapeTest.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ScaledStaticCompoundShapeTest)
{
	JPH_ADD_BASE_CLASS(ScaledStaticCompoundShapeTest, Test)
}

void ScaledStaticCompoundShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Left end
	Array<Vec3> end1;
	end1.push_back(Vec3(0, 0, 0));
	end1.push_back(Vec3(0, 0, 1));
	end1.push_back(Vec3(2, 0, 0));
	end1.push_back(Vec3(2, 0, 1));
	end1.push_back(Vec3(0, 1, 0));
	end1.push_back(Vec3(0, 1, 1));
	end1.push_back(Vec3(2, 1, 0));
	end1.push_back(Vec3(2, 1, 1));
	RefConst<ShapeSettings> end1_shape = new ConvexHullShapeSettings(end1);

	// Right end
	Array<Vec3> end2;
	end2.push_back(Vec3(0, 0, 0));
	end2.push_back(Vec3(0, 0, 5));
	end2.push_back(Vec3(0, 1, 0));
	end2.push_back(Vec3(0, 1, 5));
	end2.push_back(Vec3(1, 0, 0));
	end2.push_back(Vec3(1, 0, 5));
	end2.push_back(Vec3(1, 1, 0));
	end2.push_back(Vec3(1, 1, 5));
	RefConst<ShapeSettings> end2_shape = new ConvexHullShapeSettings(end2);

	// Central part
	Array<Vec3> center;
	center.push_back(Vec3(0, 0, 0));
	center.push_back(Vec3(0, 0, 1));
	center.push_back(Vec3(0, 1, 0));
	center.push_back(Vec3(0, 1, 1));
	center.push_back(Vec3(10, 0, 0));
	center.push_back(Vec3(10, 0, 1));
	center.push_back(Vec3(10, 1, 0));
	center.push_back(Vec3(10, 1, 1));
	RefConst<ShapeSettings> center_shape = new ConvexHullShapeSettings(center);

	// Create compound
	Ref<StaticCompoundShapeSettings> compound_shape = new StaticCompoundShapeSettings;
	compound_shape->AddShape(Vec3(-5, -1.5f, -0.5f), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), end1_shape);
	compound_shape->AddShape(Vec3(5, -0.5f, -0.5f), Quat::sIdentity(), end2_shape);
	compound_shape->AddShape(Vec3(-5, -0.5f, -0.5f), Quat::sIdentity(), center_shape);

	// Original shape
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(compound_shape, RVec3(-40, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Uniformly scaled shape
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(compound_shape, Vec3::sReplicate(0.25f)), RVec3(-20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Non-uniform scaled shape
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(compound_shape, Vec3(0.25f, 0.5f, 1.5f)), RVec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Flipped in 2 axis
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(compound_shape, Vec3(-0.25f, 0.5f, -1.5f)), RVec3(20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);

	// Inside out
	mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ScaledShapeSettings(compound_shape, Vec3(-0.25f, 0.5f, 1.5f)), RVec3(40, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
}
