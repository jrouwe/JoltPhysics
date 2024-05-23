// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Vehicle/VehicleTest.h>
#include <Jolt/Physics/Constraints/DistanceConstraint.h>
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/GroupFilterTable.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/PhysicsScene.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Layers.h>
#include <Application/DebugUI.h>
#include <Utils/Log.h>
#include <Renderer/DebugRendererImp.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(VehicleTest)
{
	JPH_ADD_BASE_CLASS(VehicleTest, Test)
}

const char *VehicleTest::sScenes[] =
{
	"Flat",
	"Flat With Slope",
	"Steep Slope",
	"Step",
	"Dynamic Step",
	"Playground",
	"Loop",
#ifdef JPH_OBJECT_STREAM
	"Terrain1",
#endif // JPH_OBJECT_STREAM
};

const char *VehicleTest::sSceneName = "Playground";

void VehicleTest::Initialize()
{
	if (strcmp(sSceneName, "Flat") == 0)
	{
		// Flat test floor
		Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(1000.0f, 1.0f, 1000.0f), 0.0f), RVec3(0.0f, -1.0f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		floor.SetFriction(1.0f);
		mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);

		// Load a race track to have something to assess speed and steering behavior
		LoadRaceTrack("Assets/Racetracks/Zandvoort.csv");
	}
	else if (strcmp(sSceneName, "Flat With Slope") == 0)
	{
		const float cSlopeStartDistance = 100.0f;
		const float cSlopeLength = 100.0f;
		const float cSlopeAngle = DegreesToRadians(30.0f);

		// Flat test floor
		Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(1000.0f, 1.0f, 1000.0f), 0.0f), RVec3(0.0f, -1.0f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		floor.SetFriction(1.0f);
		mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);

		Body &slope_up = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(25.0f, 1.0f, cSlopeLength), 0.0f), RVec3(0.0f, cSlopeLength * Sin(cSlopeAngle) - 1.0f, cSlopeStartDistance + cSlopeLength * Cos(cSlopeAngle)), Quat::sRotation(Vec3::sAxisX(), -cSlopeAngle), EMotionType::Static, Layers::NON_MOVING));
		slope_up.SetFriction(1.0f);
		mBodyInterface->AddBody(slope_up.GetID(), EActivation::DontActivate);

		Body &slope_down = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(25.0f, 1.0f, cSlopeLength), 0.0f), RVec3(0.0f, cSlopeLength * Sin(cSlopeAngle) - 1.0f, cSlopeStartDistance + 3.0f * cSlopeLength * Cos(cSlopeAngle)), Quat::sRotation(Vec3::sAxisX(), cSlopeAngle), EMotionType::Static, Layers::NON_MOVING));
		slope_down.SetFriction(1.0f);
		mBodyInterface->AddBody(slope_down.GetID(), EActivation::DontActivate);
	}
	else if (strcmp(sSceneName, "Steep Slope") == 0)
	{
		// Steep slope test floor (20 degrees = 36% grade)
		Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(1000.0f, 1.0f, 1000.0f), 0.0f), RVec3(0.0f, -1.0f, 0.0f), Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(-20.0f)), EMotionType::Static, Layers::NON_MOVING));
		floor.SetFriction(1.0f);
		mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);
	}
	else if (strcmp(sSceneName, "Step") == 0)
	{
		// Flat test floor
		Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(1000.0f, 1.0f, 1000.0f), 0.0f), RVec3(0.0f, -1.0f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		floor.SetFriction(1.0f);
		mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);

		// A 5cm step rotated under an angle
		constexpr float cStepHeight = 0.05f;
		Body &step = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(5.0f, 0.5f * cStepHeight, 5.0f), 0.0f), RVec3(-2.0f, 0.5f * cStepHeight, 60.0f), Quat::sRotation(Vec3::sAxisY(), -0.3f * JPH_PI), EMotionType::Static, Layers::NON_MOVING));
		step.SetFriction(1.0f);
		mBodyInterface->AddBody(step.GetID(), EActivation::DontActivate);
	}
	else if (strcmp(sSceneName, "Dynamic Step") == 0)
	{
		// Flat test floor
		Body &floor = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(1000.0f, 1.0f, 1000.0f), 0.0f), RVec3(0.0f, -1.0f, 0.0f), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		floor.SetFriction(1.0f);
		mBodyInterface->AddBody(floor.GetID(), EActivation::DontActivate);

		// A dynamic body that acts as a step to test sleeping behavior
		constexpr float cStepHeight = 0.05f;
		Body &step = *mBodyInterface->CreateBody(BodyCreationSettings(new BoxShape(Vec3(15.0f, 0.5f * cStepHeight, 15.0f), 0.0f), RVec3(-2.0f, 0.5f * cStepHeight, 30.0f), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING));
		step.SetFriction(1.0f);
		mBodyInterface->AddBody(step.GetID(), EActivation::Activate);
	}
	else if (strcmp(sSceneName, "Playground") == 0)
	{
		// Scene with hilly terrain and some objects to drive into
		Body &floor = CreateMeshTerrain();
		floor.SetFriction(1.0f);

		CreateBridge();

		CreateWall();

		CreateRubble();
	}
	else if (strcmp(sSceneName, "Loop") == 0)
	{
		CreateFloor();

		TriangleList triangles;
		const int cNumSegments = 100;
		const float cLoopWidth = 20.0f;
		const float cLoopRadius = 20.0f;
		const float cLoopThickness = 0.5f;
		Vec3 prev_center = Vec3::sZero();
		Vec3 prev_center_bottom = Vec3::sZero();
		for (int i = 0; i < cNumSegments; ++i)
		{
			float angle = i * 2.0f * JPH_PI / (cNumSegments - 1);
			Vec3 radial(0, -Cos(angle), Sin(angle));
			Vec3 center = Vec3(-i * cLoopWidth / (cNumSegments - 1), cLoopRadius, cLoopRadius) + cLoopRadius * radial;
			Vec3 half_width(0.5f * cLoopWidth, 0, 0);
			Vec3 center_bottom = center + cLoopThickness * radial;
			if (i > 0)
			{
				// Top surface
				triangles.push_back(Triangle(prev_center + half_width, prev_center - half_width, center - half_width));
				triangles.push_back(Triangle(prev_center + half_width, center - half_width, center + half_width));

				// Bottom surface
				triangles.push_back(Triangle(prev_center_bottom + half_width, center_bottom - half_width, prev_center_bottom - half_width));
				triangles.push_back(Triangle(prev_center_bottom + half_width, center_bottom + half_width, center_bottom - half_width));

				// Sides
				triangles.push_back(Triangle(prev_center + half_width, center + half_width, prev_center_bottom + half_width));
				triangles.push_back(Triangle(prev_center_bottom + half_width, center + half_width, center_bottom + half_width));
				triangles.push_back(Triangle(prev_center - half_width, prev_center_bottom - half_width, center - half_width));
				triangles.push_back(Triangle(prev_center_bottom - half_width, center_bottom - half_width, center - half_width));
			}
			prev_center = center;
			prev_center_bottom = center_bottom;
		}
		MeshShapeSettings mesh(triangles);
		mesh.SetEmbedded();

		Body &loop = *mBodyInterface->CreateBody(BodyCreationSettings(&mesh, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING));
		loop.SetFriction(1.0f);
		mBodyInterface->AddBody(loop.GetID(), EActivation::Activate);
	}
#ifdef JPH_OBJECT_STREAM
	else
	{
		// Load scene
		Ref<PhysicsScene> scene;
		if (!ObjectStreamIn::sReadObject((String("Assets/") + sSceneName + ".bof").c_str(), scene))
			FatalError("Failed to load scene");
		for (BodyCreationSettings &body : scene->GetBodies())
			body.mObjectLayer = Layers::NON_MOVING;
		scene->FixInvalidScales();
		scene->CreateBodies(mPhysicsSystem);
	}
#endif // JPH_OBJECT_STREAM
}

void VehicleTest::CreateBridge()
{
	const int cChainLength = 20;

	// Build a collision group filter that disables collision between adjacent bodies
	Ref<GroupFilterTable> group_filter = new GroupFilterTable(cChainLength);
	for (CollisionGroup::SubGroupID i = 0; i < cChainLength - 1; ++i)
		group_filter->DisableCollision(i, i + 1);

	Vec3 part_half_size = Vec3(2.5f, 0.25f, 1.0f);
	RefConst<Shape> part_shape = new BoxShape(part_half_size);

	Vec3 large_part_half_size = Vec3(2.5f, 0.25f, 22.5f);
	RefConst<Shape> large_part_shape = new BoxShape(large_part_half_size);

	Quat first_part_rot = Quat::sRotation(Vec3::sAxisX(), DegreesToRadians(-10.0f));

	RVec3 prev_pos(-25, 7, 0);
	Body *prev_part = nullptr;

	for (int i = 0; i < cChainLength; ++i)
	{
		RVec3 pos = prev_pos + Vec3(0, 0, 2.0f * part_half_size.GetZ());

		Body &part = i == 0? *mBodyInterface->CreateBody(BodyCreationSettings(large_part_shape, pos - first_part_rot * Vec3(0, large_part_half_size.GetY() - part_half_size.GetY(), large_part_half_size.GetZ() - part_half_size.GetZ()), first_part_rot, EMotionType::Static, Layers::NON_MOVING))
					: *mBodyInterface->CreateBody(BodyCreationSettings(part_shape, pos, Quat::sIdentity(), i == 19? EMotionType::Static : EMotionType::Dynamic, i == 19? Layers::NON_MOVING : Layers::MOVING));
		part.SetCollisionGroup(CollisionGroup(group_filter, 1, CollisionGroup::SubGroupID(i)));
		part.SetFriction(1.0f);
		mBodyInterface->AddBody(part.GetID(), EActivation::Activate);

		if (prev_part != nullptr)
		{
			DistanceConstraintSettings dc;
			dc.mPoint1 = prev_pos + Vec3(-part_half_size.GetX(), 0, part_half_size.GetZ());
			dc.mPoint2 = pos + Vec3(-part_half_size.GetX(), 0, -part_half_size.GetZ());
			mPhysicsSystem->AddConstraint(dc.Create(*prev_part, part));

			dc.mPoint1 = prev_pos + Vec3(part_half_size.GetX(), 0, part_half_size.GetZ());
			dc.mPoint2 = pos + Vec3(part_half_size.GetX(), 0, -part_half_size.GetZ());
			mPhysicsSystem->AddConstraint(dc.Create(*prev_part, part));
		}

		prev_part = &part;
		prev_pos = pos;
	}
}

void VehicleTest::CreateWall()
{
	RefConst<Shape> box_shape = new BoxShape(Vec3(0.5f, 0.5f, 0.5f));
	for (int i = 0; i < 3; ++i)
		for (int j = i / 2; j < 5 - (i + 1) / 2; ++j)
		{
			RVec3 position(2.0f + j * 1.0f + (i & 1? 0.5f : 0.0f), 2.0f + i * 1.0f, 10.0f);
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(box_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
		}
}

void VehicleTest::CreateRubble()
{
	// Flat and light objects
	RefConst<Shape> box_shape = new BoxShape(Vec3(0.5f, 0.1f, 0.5f));
	for (int i = 0; i < 5; ++i)
		for (int j = 0; j < 5; ++j)
		{
			RVec3 position(-5.0f + j, 2.0f + i * 0.2f, 10.0f + 0.5f * i);
			mBodyInterface->CreateAndAddBody(BodyCreationSettings(box_shape, position, Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
		}


	// Light convex shapes
	default_random_engine random;
	uniform_real_distribution<float> hull_size(0.2f, 0.4f);
	for (int i = 0; i < 10; ++i)
		for (int j = 0; j < 10; ++j)
		{
			// Create random points
			Array<Vec3> points;
			for (int k = 0; k < 20; ++k)
				points.push_back(hull_size(random) * Vec3::sRandom(random));

			mBodyInterface->CreateAndAddBody(BodyCreationSettings(new ConvexHullShapeSettings(points), RVec3(-5.0f + 0.5f * j, 2.0f, 15.0f + 0.5f * i), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING), EActivation::Activate);
		}
}

void VehicleTest::LoadRaceTrack(const char *inFileName)
{
	// Open the track file
	std::ifstream stream;
	stream.open(inFileName, std::ifstream::in);
	if (!stream.is_open())
		return;

	// Ignore header line
	String line;
	std::getline(stream, line);

	// Read coordinates
	struct Segment
	{
		RVec3				mCenter;
		float				mWidthLeft;
		float				mWidthRight;
	};
	Array<Segment> segments;
	Real x, y;
	float wl, wr;
	char c;
	RVec3 track_center = RVec3::sZero();
	while (stream >> x >> c >> y >> c >> wl >> c >> wr)
	{
		RVec3 center(x, 0, -y);
		segments.push_back({ center, wl, wr });
		track_center += center;
	}
	if (!segments.empty())
		track_center /= (float)segments.size();

	// Convert to line segments
	RVec3 prev_tleft = RVec3::sZero(), prev_tright = RVec3::sZero();
	for (size_t i = 0; i < segments.size(); ++i)
	{
		const Segment &segment = segments[i];
		const Segment &next_segment = segments[(i + 1) % segments.size()];

		// Calculate left and right point of the track
		Vec3 fwd = Vec3(next_segment.mCenter - segment.mCenter);
		Vec3 right = fwd.Cross(Vec3::sAxisY()).Normalized();
		RVec3 tcenter = segment.mCenter - track_center + Vec3(0, 0.1f, 0); // Put a bit above the floor to avoid z fighting
		RVec3 tleft = tcenter - right * segment.mWidthLeft;
		RVec3 tright = tcenter + right * segment.mWidthRight;
		mTrackData.push_back({ tleft, tright });

		// Connect left and right point with the previous left and right point
		if (i > 0)
		{
			mTrackData.push_back({ prev_tleft, tleft });
			mTrackData.push_back({ prev_tright, tright });
		}

		prev_tleft = tleft;
		prev_tright = tright;
	}
}

void VehicleTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Render the track
	for (const Line &l : mTrackData)
		mDebugRenderer->DrawLine(l.mStart, l.mEnd, Color::sBlack);
}

void VehicleTest::CreateSettingsMenu(DebugUI *inUI, UIElement *inSubMenu)
{
	inUI->CreateTextButton(inSubMenu, "Select Scene", [this, inUI]() {
		UIElement *scene_name = inUI->CreateMenu();
		for (uint i = 0; i < size(sScenes); ++i)
			inUI->CreateTextButton(scene_name, sScenes[i], [this, i]() { sSceneName = sScenes[i]; RestartTest(); });
		inUI->ShowMenu(scene_name);
	});
}

