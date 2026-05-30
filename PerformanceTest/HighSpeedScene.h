// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

// Jolt includes
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/Shape/StaticCompoundShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>

// Local includes
#include "PerformanceTestScene.h"
#include "Layers.h"

// A scene that contains a large number of fast moving objects
class HighSpeedScene : public PerformanceTestScene
{
public:
	virtual const char *	GetName() const override
	{
		return "HighSpeed";
	}

	virtual bool			Load(const String &inAssetPath) override
	{
		const float shape_size = 0.5f;

		Ref<StaticCompoundShapeSettings> compound = new StaticCompoundShapeSettings();
		compound->AddShape(Vec3(0, shape_size, 0), Quat::sIdentity(), new CapsuleShapeSettings(shape_size, 0.25f * shape_size));
		compound->AddShape(Vec3(shape_size, 0, 0), Quat::sIdentity(), new BoxShapeSettings(Vec3(shape_size, 0.25f * shape_size, 0.25f * shape_size)));

		mShapes = {
			new SphereShape(shape_size),
			new BoxShape(Vec3(0.5f * shape_size, shape_size, 1.0f * shape_size)),
			compound->Create().Get()
		};

		return true;
	}

	virtual void			StartTest(PhysicsSystem &inPhysicsSystem, EMotionQuality inMotionQuality) override
	{
		BodyInterface &bi = inPhysicsSystem.GetBodyInterface();

		const float speed = 250.0f;
		const float wall_thickness = 0.2f;
		const float half_box_size = 50.0f;
		const float pos_range = 0.9f * half_box_size;

		// Create hollow box to enclose the objects
		BodyCreationSettings body_settings(new BoxShape(Vec3(half_box_size, wall_thickness, half_box_size)), RVec3(0, -Real(half_box_size), 0), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING);
		body_settings.mRestitution = 1.0f;
		body_settings.mFriction = 1.0f;
		bi.CreateAndAddBody(body_settings, EActivation::DontActivate);
		body_settings.mPosition = RVec3(0, Real(half_box_size), 0);
		bi.CreateAndAddBody(body_settings, EActivation::DontActivate);
		body_settings.mPosition = RVec3(0, 0, -Real(half_box_size));
		body_settings.mRotation = Quat::sRotation(Vec3::sAxisX(), 0.5f * JPH_PI);
		bi.CreateAndAddBody(body_settings, EActivation::DontActivate);
		body_settings.mPosition = RVec3(0, 0, Real(half_box_size));
		bi.CreateAndAddBody(body_settings, EActivation::DontActivate);
		body_settings.mPosition = RVec3(-Real(half_box_size), 0, 0);
		body_settings.mRotation = Quat::sRotation(Vec3::sAxisZ(), 0.5f * JPH_PI);
		bi.CreateAndAddBody(body_settings, EActivation::DontActivate);
		body_settings.mPosition = RVec3(Real(half_box_size), 0, 0);
		bi.CreateAndAddBody(body_settings, EActivation::DontActivate);

		// Create dynamic box
		BodyCreationSettings dynamic_body_settings(mShapes[0], RVec3::sZero(), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
		dynamic_body_settings.mMotionQuality = EMotionQuality::LinearCast;

		// Create many instances with high velocity (don't use std::uniform_real_distribution as that is not cross platform deterministic)
		mt19937 rnd;
		auto random_float = [](mt19937 &inRnd, float inMin, float inMax) { return inMin + (inRnd() - inRnd.min()) * (inMax - inMin) / (inRnd.max() - inRnd.min()); };
		for (int i = 0; i < 5000; ++i)
		{
			dynamic_body_settings.SetShape(mShapes[i % mShapes.size()]);
			dynamic_body_settings.mPosition = RVec3(Real(random_float(rnd, -pos_range, pos_range)), Real(random_float(rnd, -pos_range, pos_range)), Real(random_float(rnd, -pos_range, pos_range)));
			dynamic_body_settings.mRotation = Quat::sRandom(rnd);
			dynamic_body_settings.mFriction = random_float(rnd, 0.5f, 1.0f);
			dynamic_body_settings.mRestitution = random_float(rnd, 0.9f, 1.0f);
			dynamic_body_settings.mLinearVelocity = speed * Vec3::sRandom(rnd);
			bi.CreateAndAddBody(dynamic_body_settings, EActivation::Activate);
		}
	}

private:
	Array<Ref<Shape>>		mShapes;
};
