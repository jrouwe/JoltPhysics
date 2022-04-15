// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/General/FunnelTest.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/TaperedCapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <random>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(FunnelTest) 
{ 
	JPH_ADD_BASE_CLASS(FunnelTest, Test) 
}

void FunnelTest::Initialize()
{
	RefConst<Shape> box = new BoxShape(Vec3(50, 1, 50), 0.0f);

	// Funnel 
	for (int i = 0; i < 4; ++i)
	{
		Quat rotation = Quat::sRotation(Vec3::sAxisY(), 0.5f * JPH_PI * i);

		mBodyInterface->CreateAndAddBody(BodyCreationSettings(box, rotation * Vec3(25, 25, 0), rotation * Quat::sRotation(Vec3::sAxisZ(), 0.25f * JPH_PI), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);
	}

	default_random_engine random;
	uniform_real_distribution<float> feature_size(0.1f, 2.0f);
	uniform_real_distribution<float> position_variation(-40, 40);
	uniform_real_distribution<float> scale_variation(-1.5f, 1.5f);

	// Random scale
	Vec3 scale(scale_variation(random), scale_variation(random), scale_variation(random));

	// Make it minimally -0.5 or 0.5 depending on the sign
	scale += Vec3::sSelect(Vec3::sReplicate(-0.5f), Vec3::sReplicate(0.5f), Vec3::sGreaterOrEqual(scale, Vec3::sZero()));

	RefConst<Shape> shape;
	for (int i = 0; i < 1000; ++i)
	{
		switch (random() % 8)
		{
		case 0:
			{
				shape = new SphereShape(feature_size(random));
				scale = scale.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(); // Only uniform scale supported
				break;
			}

		case 1:
			{
				shape = new BoxShape(Vec3(feature_size(random), feature_size(random), feature_size(random)));
				break;
			}

		case 2:
			{
				// Create random points
				vector<Vec3> points;
				for (int j = 0; j < 20; ++j)
					points.push_back(feature_size(random) * Vec3::sRandom(random));
				shape = ConvexHullShapeSettings(points).Create().Get();
				break;
			}

		case 3:
			{
				shape = new CapsuleShape(0.5f * feature_size(random), feature_size(random));				
				scale = scale.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(); // Only uniform scale supported
				break;
			}

		case 4:
			{
				float top = feature_size(random);
				float bottom = feature_size(random);
				float half_height = max(0.5f * feature_size(random), 0.5f * abs(top - bottom) + 0.001f);
				shape = TaperedCapsuleShapeSettings(half_height, top, bottom).Create().Get();
				scale = scale.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(); // Only uniform scale supported
				break;
			}

		case 5:
			{
				shape = new CylinderShape(0.5f * feature_size(random), feature_size(random));
				scale = scale.Swizzle<SWIZZLE_X, SWIZZLE_Y, SWIZZLE_X>(); // Scale X must be same as Z
				break;
			}

		case 6:
			{
				// Simple compound
				StaticCompoundShapeSettings compound_shape_settings;
				compound_shape_settings.AddShape(Vec3::sZero(), Quat::sIdentity(), new CapsuleShape(1, 0.1f));
				compound_shape_settings.AddShape(Vec3(0, -1, 0), Quat::sIdentity(), new SphereShape(0.5f));
				compound_shape_settings.AddShape(Vec3(0, 1, 0), Quat::sIdentity(), new SphereShape(0.5f));
				shape = compound_shape_settings.Create().Get();
				scale = scale.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(); // Only uniform scale supported
				break;
			}

		case 7:
			{
				// Compound with sub compound and rotation
				Ref<StaticCompoundShapeSettings> sub_compound = new StaticCompoundShapeSettings();
				sub_compound->AddShape(Vec3(0, 0.75f, 0), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), new BoxShape(Vec3(0.75f, 0.25f, 0.2f)));
				sub_compound->AddShape(Vec3(0.75f, 0, 0), Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI), new CylinderShape(0.75f, 0.2f));
				sub_compound->AddShape(Vec3(0, 0, 0.75f), Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI), new TaperedCapsuleShapeSettings(0.75f, 0.25f, 0.2f));
				StaticCompoundShapeSettings compound_shape_settings;
				compound_shape_settings.AddShape(Vec3(0, 0, 0), Quat::sRotation(Vec3::sAxisX(), -0.25f * JPH_PI) * Quat::sRotation(Vec3::sAxisZ(), 0.25f * JPH_PI), sub_compound);
				compound_shape_settings.AddShape(Vec3(0, -0.1f, 0), Quat::sRotation(Vec3::sAxisX(), 0.25f * JPH_PI) * Quat::sRotation(Vec3::sAxisZ(), -0.75f * JPH_PI), sub_compound);
				shape = compound_shape_settings.Create().Get();
				scale = scale.Swizzle<SWIZZLE_X, SWIZZLE_X, SWIZZLE_X>(); // Only uniform scale supported
				break;
			}
		}

		// Scale the shape
		if ((random() % 3) == 0)
			shape = new ScaledShape(shape, scale);

		Vec3 position(position_variation(random), 100.0f + position_variation(random), position_variation(random));
		mBodyInterface->CreateAndAddBody(BodyCreationSettings(shape, position, Quat::sRandom(random), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
	}
}

void FunnelTest::GetInitialCamera(CameraState &ioState) const
{
	ioState.mPos = Vec3(50, 100, 50);
	ioState.mForward = (Vec3(0, 50, 0) - ioState.mPos).Normalized();
}
