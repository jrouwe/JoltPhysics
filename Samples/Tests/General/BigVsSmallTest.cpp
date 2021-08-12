// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/BigVsSmallTest.h>
#include <Physics/Collision/Shape/BoxShape.h>
#include <Physics/Collision/Shape/MeshShape.h>
#include <Physics/Body/BodyCreationSettings.h>
#include <Geometry/Triangle.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(BigVsSmallTest) 
{ 
	JPH_ADD_BASE_CLASS(BigVsSmallTest, Test) 
}

void BigVsSmallTest::Initialize()
{
	// Create a big triangle
	TriangleList triangles;
	triangles.push_back(Triangle(Float3(-100, 0, 0), Float3(0, 0, 100), Float3(100, 0, -100)));
	Body &triangle = *mBodyInterface->CreateBody(BodyCreationSettings(new MeshShapeSettings(triangles), Vec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
	mBodyInterface->AddBody(triangle.GetID(), EActivation::DontActivate);
				
	// A small box
	Body &body = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(0.1f, 0.1f, 0.1f)), Vec3(0, 1.0f, 0), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
	body.SetAllowSleeping(false);
	mBodyInterface->AddBody(body.GetID(), EActivation::Activate);
}