// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2023 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/Shapes/DeformedHeightFieldShapeTest.h>
#include <External/Perlin.h>
#include <Jolt/Physics/Body/BodyCreationSettings.h>
#include <Jolt/Physics/Collision/Shape/SphereShape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Layers.h>

JPH_IMPLEMENT_RTTI_VIRTUAL(DeformedHeightFieldShapeTest)
{
	JPH_ADD_BASE_CLASS(DeformedHeightFieldShapeTest, Test)
}

void DeformedHeightFieldShapeTest::Initialize()
{
	constexpr float cCellSize = 1.0f;
	constexpr float cMaxHeight = 2.5f;
	constexpr float cSphereRadius = 2.0f;

	// Create height samples
	mHeightSamples.resize(cSampleCount * cSampleCount);
	for (int y = 0; y < cSampleCount; ++y)
		for (int x = 0; x < cSampleCount; ++x)
			mHeightSamples[y * cSampleCount + x] = cMaxHeight * PerlinNoise3(float(x) * 8.0f / cSampleCount, 0, float(y) * 8.0f / cSampleCount, 256, 256, 256);

	// Determine scale and offset of the terrain
	Vec3 offset(-0.5f * cCellSize * cSampleCount, 0, -0.5f * cCellSize * cSampleCount);
	Vec3 scale(cCellSize, 1.0f, cCellSize);

	// Create height field
	HeightFieldShapeSettings settings(mHeightSamples.data(), offset, scale, cSampleCount);
	settings.mBlockSize = cBlockSize;
	settings.mBitsPerSample = 8;
	settings.mMinHeightValue = -15.0f;
	mHeightField = StaticCast<HeightFieldShape>(settings.Create().Get());
	mHeightFieldID = mBodyInterface->CreateAndAddBody(BodyCreationSettings(mHeightField, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, Layers::NON_MOVING), EActivation::DontActivate);

	// Spheres on top of the terrain
	RefConst<Shape> sphere_shape = new SphereShape(cSphereRadius);
	for (float t = 0.2f; t < 12.4f; t += 0.1f)
	{
		// Get the center of the path
		Vec3 center = offset + GetPathCenter(t);

		// Cast a ray onto the terrain
		RShapeCast shape_cast(sphere_shape, Vec3::sOne(), RMat44::sTranslation(RVec3(0, 10, 0) + center), Vec3(0, -20, 0));
		ClosestHitCollisionCollector<CastShapeCollector> collector;
		mPhysicsSystem->GetNarrowPhaseQuery().CastShape(shape_cast, { }, RVec3::sZero(), collector);
		if (collector.mHit.mBodyID2 == mHeightFieldID)
		{
			// Create sphere on terrain
			BodyCreationSettings bcs(sphere_shape, shape_cast.GetPointOnRay(collector.mHit.mFraction), Quat::sIdentity(), EMotionType::Dynamic, Layers::MOVING);
			mBodyInterface->CreateAndAddBody(bcs, EActivation::DontActivate);
		}
	}
}

Vec3 DeformedHeightFieldShapeTest::GetPathCenter(float inTime) const
{
	constexpr float cOffset = 5.0f;
	constexpr float cRadiusX = 60.0f;
	constexpr float cRadiusY = 25.0f;
	constexpr float cFallOff = 0.1f;
	constexpr float cAngularSpeed = 2.0f;
	constexpr float cDisplacementSpeed = 10.0f;

	float fall_off = exp(-cFallOff * inTime);
	float angle = cAngularSpeed * inTime;
	return Vec3(cRadiusX * Cos(angle) * fall_off + 64.0f, 0, cOffset + cDisplacementSpeed * inTime + cRadiusY * Sin(angle) * fall_off);
}

void DeformedHeightFieldShapeTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	constexpr float cPitRadius = 6.0f;
	constexpr float cPitHeight = 1.0f;
	constexpr float cSpeedScale = 2.0f;

	// Calculate center of pit
	Vec3 center = GetPathCenter(cSpeedScale * mTime);
	mTime += inParams.mDeltaTime;

	// Calculate affected area
	int start_x = max((int)floor(center.GetX() - cPitRadius) & ~cBlockMask, 0);
	int start_y = max((int)floor(center.GetZ() - cPitRadius) & ~cBlockMask, 0);
	int count_x = min(((int)ceil(center.GetX() + cPitRadius) + cBlockMask) & ~cBlockMask, cSampleCount) - start_x;
	int count_y = min(((int)ceil(center.GetZ() + cPitRadius) + cBlockMask) & ~cBlockMask, cSampleCount) - start_y;

	if (count_x > 0 && count_y > 0)
	{
		// Remember COM before we change the height field
		Vec3 old_com = mHeightField->GetCenterOfMass();

		// A function to calculate the delta height at a certain distance from the center of the pit
		constexpr float cHalfPi = 0.5f * JPH_PI;
		auto pit_shape = [=](float inDistanceX, float inDistanceY) { return Cos(min(sqrt(Square(inDistanceX) + Square(inDistanceY)) * cHalfPi / cPitRadius, cHalfPi)); };

		AABox affected_area;
		for (int y = 0; y < count_y; ++y)
			for (int x = 0; x < count_x; ++x)
			{
				// Update the height field
				float delta = pit_shape(float(start_x) + x - center.GetX(), float(start_y) + y - center.GetZ()) * cPitHeight;
				mHeightSamples[(start_y + y) * cSampleCount + start_x + x] -= delta;

				// Keep track of affected area to wake up bodies
				affected_area.Encapsulate(mHeightField->GetPosition(start_x + x, start_y + y));
			}
		mHeightField->SetHeights(start_x, start_y, count_x, count_y, mHeightSamples.data() + start_y * cSampleCount + start_x, cSampleCount, *mTempAllocator);

		// Notify the shape that it has changed its bounding box
		mBodyInterface->NotifyShapeChanged(mHeightFieldID, old_com, false, EActivation::DontActivate);

		// Activate bodies in the affected area (a change in the height field doesn't wake up bodies)
		affected_area.ExpandBy(Vec3::sReplicate(0.1f));
		DefaultBroadPhaseLayerFilter broadphase_layer_filter = mPhysicsSystem->GetDefaultBroadPhaseLayerFilter(Layers::MOVING);
		DefaultObjectLayerFilter object_layer_filter = mPhysicsSystem->GetDefaultLayerFilter(Layers::MOVING);
		mBodyInterface->ActivateBodiesInAABox(affected_area, broadphase_layer_filter, object_layer_filter);
	}
}
