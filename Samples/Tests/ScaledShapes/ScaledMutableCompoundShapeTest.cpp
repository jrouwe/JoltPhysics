// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ScaledShapes/ScaledMutableCompoundShapeTest.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MutableCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(ScaledMutableCompoundShapeTest) 
{ 
	JPH_ADD_BASE_CLASS(ScaledMutableCompoundShapeTest, Test) 
}

void ScaledMutableCompoundShapeTest::Initialize()
{
	// Floor
	CreateFloor();

	// Left end
	vector<Vec3> end1;
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
	vector<Vec3> end2;
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
	vector<Vec3> center;
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
	Ref<MutableCompoundShapeSettings> compound_shape = new MutableCompoundShapeSettings;
	compound_shape->AddShape(Vec3(-5, -1.5f, -0.5f), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), end1_shape);
	compound_shape->AddShape(Vec3(5, -0.5f, -0.5f), Quat::sIdentity(), end2_shape);
	compound_shape->AddShape(Vec3(-5, -0.5f, -0.5f), Quat::sIdentity(), center_shape);

	// Original shape
	Body &body1 = *mBodyInterface->CreateBody(BodyCreationSettings(compound_shape, Vec3(-40, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body1.GetID(), EActivation::Activate);

	// Uniformly scaled shape
	Body &body2 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(compound_shape, Vec3::sReplicate(0.25f)), Vec3(-20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body2.GetID(), EActivation::Activate);

	// Non-uniform scaled shape
	Body &body3 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(compound_shape, Vec3(0.25f, 0.5f, 1.5f)), Vec3(0, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body3.GetID(), EActivation::Activate);

	// Flipped in 2 axis
	Body &body4 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(compound_shape, Vec3(-0.25f, 0.5f, -1.5f)), Vec3(20, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body4.GetID(), EActivation::Activate);

	// Inside out
	Body &body5 = *mBodyInterface->CreateBody(BodyCreationSettings(new ScaledShapeSettings(compound_shape, Vec3(-0.25f, 0.5f, 1.5f)), Vec3(40, 10, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	mBodyInterface->AddBody(body5.GetID(), EActivation::Activate);
}
