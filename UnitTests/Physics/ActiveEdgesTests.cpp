// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include "PhysicsTestContext.h"
#include "Layers.h"
#include <Jolt/Physics/Collision/Shape/BoxShape.h>
#include <Jolt/Physics/Collision/Shape/CapsuleShape.h>
#include <Jolt/Physics/Collision/Shape/RotatedTranslatedShape.h>
#include <Jolt/Physics/Collision/Shape/MeshShape.h>
#include <Jolt/Physics/Collision/Shape/HeightFieldShape.h>
#include <Jolt/Physics/Collision/Shape/ScaledShape.h>
#include <Jolt/Physics/Collision/CollisionCollectorImpl.h>
#include <Jolt/Physics/Collision/CollideShape.h>
#include <Jolt/Physics/Collision/ShapeCast.h>
#include <Jolt/Physics/Collision/CollisionDispatch.h>

TEST_SUITE("ActiveEdgesTest")
{
	static const float cCapsuleProbeOffset = 0.1f; // How much to offset the probe from y = 0 in order to avoid hitting a back instead of a front face
	static const float cCapsuleRadius = 0.1f;

	// Create a capsule as our probe
	static Ref<Shape> sCreateProbeCapsule()
	{
		// Ensure capsule is long enough so that when active edges mode is on, we will always get a horizontal penetration axis rather than a vertical one
		CapsuleShapeSettings capsule(1.0f, cCapsuleRadius);
		capsule.SetEmbedded();
		return capsule.Create().Get();
	}

	// Create a flat mesh shape consisting of 7 x 7 quads, we know that only the outer edges of this shape are active
	static Ref<ShapeSettings> sCreateMeshShape()
	{
		TriangleList triangles;
		for (int z = 0; z < 7; ++z)
			for (int x = 0; x < 7; ++x)
			{
				float fx = (float)x - 3.5f, fz = (float)z - 3.5f;
				triangles.push_back(Triangle(Vec3(fx, 0, fz), Vec3(fx, 0, fz + 1), Vec3(fx + 1, 0, fz + 1)));
				triangles.push_back(Triangle(Vec3(fx, 0, fz), Vec3(fx + 1, 0, fz + 1), Vec3(fx + 1, 0, fz)));
			}

		return new MeshShapeSettings(triangles);
	}

	// Create a flat height field shape that has the same properties as the mesh shape
	static Ref<ShapeSettings> sCreateHeightFieldShape()
	{
		float samples[8*8];
		memset(samples, 0, sizeof(samples));
		return new HeightFieldShapeSettings(samples, Vec3(-3.5f, 0, -3.5f), Vec3::sOne(), 8);
	}

	// This struct indicates what we hope to find as hit
	struct ExpectedHit
	{
		Vec3		mPosition;
		Vec3		mPenetrationAxis;
	};

	// Compare expected hits with returned hits
	template <class ResultType>
	static void sCheckMatch(const Array<ResultType> &inResult, const Array<ExpectedHit> &inExpectedHits, float inAccuracySq)
	{
		CHECK(inResult.size() == inExpectedHits.size());

		for (const ExpectedHit &hit : inExpectedHits)
		{
			bool found = false;
			for (const ResultType &result : inResult)
				if (result.mContactPointOn2.IsClose(hit.mPosition, inAccuracySq)
					&& result.mPenetrationAxis.Normalized().IsClose(hit.mPenetrationAxis, inAccuracySq))
				{
					found = true;
					break;
				}
			CHECK(found);
		}
	}

	// Collide our probe against the test shape and validate the hit results
	static void sTestCollideShape(Shape *inProbeShape, Shape *inTestShape, Vec3Arg inTestShapeScale, const CollideShapeSettings &inSettings, Vec3Arg inProbeShapePos, const Array<ExpectedHit> &inExpectedHits)
	{
		AllHitCollisionCollector<CollideShapeCollector> collector;
		CollisionDispatch::sCollideShapeVsShape(inProbeShape, inTestShape, Vec3::sOne(), inTestShapeScale, Mat44::sTranslation(inProbeShapePos), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), inSettings, collector);

		sCheckMatch(collector.mHits, inExpectedHits, 1.0e-8f);
	}

	// Collide a probe shape against our test shape in various locations to verify active edge behavior
	static void sTestCollideShape(const ShapeSettings *inTestShape, Vec3Arg inTestShapeScale, bool inActiveEdgesOnly)
	{
		CollideShapeSettings settings;
		settings.mActiveEdgeMode = inActiveEdgesOnly? EActiveEdgeMode::CollideOnlyWithActive : EActiveEdgeMode::CollideWithAll;

		Ref<Shape> test_shape = inTestShape->Create().Get();
		Ref<Shape> capsule = sCreateProbeCapsule();

		// Test hitting all active edges
		sTestCollideShape(capsule, test_shape, inTestShapeScale, settings, Vec3(-3.5f, cCapsuleProbeOffset, 0), { { Vec3(-3.5f, 0, 0), Vec3(1, 0, 0) } });
		sTestCollideShape(capsule, test_shape, inTestShapeScale, settings, Vec3(3.5f, cCapsuleProbeOffset, 0), { { Vec3(3.5f, 0, 0), Vec3(-1, 0, 0) } });
		sTestCollideShape(capsule, test_shape, inTestShapeScale, settings, Vec3(0, cCapsuleProbeOffset, -3.5f), { { Vec3(0, 0, -3.5f), Vec3(0, 0, 1) } });
		sTestCollideShape(capsule, test_shape, inTestShapeScale, settings, Vec3(0, cCapsuleProbeOffset, 3.5f), { { Vec3(0, 0, 3.5f), Vec3(0, 0, -1) } });

		// Test hitting internal edges, this should return two hits
		sTestCollideShape(capsule, test_shape, inTestShapeScale, settings, Vec3(-2.5f, cCapsuleProbeOffset, 0), { { Vec3(-2.5f, 0, 0), inActiveEdgesOnly? Vec3(0, -1, 0) : Vec3(-1, 0, 0) }, { Vec3(-2.5f, 0, 0), inActiveEdgesOnly? Vec3(0, -1, 0) : Vec3(1, 0, 0) } });
		sTestCollideShape(capsule, test_shape, inTestShapeScale, settings, Vec3(0, cCapsuleProbeOffset, -2.5f), { { Vec3(0, 0, -2.5f), inActiveEdgesOnly? Vec3(0, -1, 0) : Vec3(0, 0, -1) }, { Vec3(0, 0, -2.5f), inActiveEdgesOnly? Vec3(0, -1, 0) : Vec3(0, 0, -1) } });

		// Test hitting an interior diagonal, this should return two hits
		sTestCollideShape(capsule, test_shape, inTestShapeScale, settings, Vec3(-3.0f, cCapsuleProbeOffset, 0), { { Vec3(-3.0f, 0, 0), inActiveEdgesOnly? Vec3(0, -1, 0) : (inTestShapeScale * Vec3(1, 0, -1)).Normalized() }, { Vec3(-3.0f, 0, 0), inActiveEdgesOnly? Vec3(0, -1, 0) : (inTestShapeScale * Vec3(-1, 0, 1)).Normalized() } });
	}

	TEST_CASE("CollideShapeMesh")
	{
		Ref<ShapeSettings> shape = sCreateMeshShape();

		sTestCollideShape(shape, Vec3::sOne(), false);

		sTestCollideShape(shape, Vec3::sOne(), true);

		sTestCollideShape(shape, Vec3(-1, 1, 1), false);

		sTestCollideShape(shape, Vec3(-1, 1, 1), true);
	}

	TEST_CASE("CollideShapeHeightField")
	{
		Ref<ShapeSettings> shape = sCreateHeightFieldShape();

		sTestCollideShape(shape, Vec3::sOne(), false);

		sTestCollideShape(shape, Vec3::sOne(), true);

		sTestCollideShape(shape, Vec3(-1, 1, 1), false);

		sTestCollideShape(shape, Vec3(-1, 1, 1), true);
	}

	// Cast our probe against the test shape and validate the hit results
	static void sTestCastShape(Shape *inProbeShape, Shape *inTestShape, Vec3Arg inTestShapeScale, const ShapeCastSettings &inSettings, Vec3Arg inProbeShapePos, Vec3Arg inProbeShapeDirection, const Array<ExpectedHit> &inExpectedHits)
	{
		AllHitCollisionCollector<CastShapeCollector> collector;
		ShapeCast shape_cast(inProbeShape, Vec3::sOne(), Mat44::sTranslation(inProbeShapePos), inProbeShapeDirection);
		CollisionDispatch::sCastShapeVsShapeLocalSpace(shape_cast, inSettings, inTestShape, inTestShapeScale, ShapeFilter(), Mat44::sIdentity(), SubShapeIDCreator(), SubShapeIDCreator(), collector);

		sCheckMatch(collector.mHits, inExpectedHits, 1.0e-6f);
	}

	// Cast a probe shape against our test shape in various locations to verify active edge behavior
	static void sTestCastShape(const ShapeSettings *inTestShape, Vec3Arg inTestShapeScale, bool inActiveEdgesOnly)
	{
		ShapeCastSettings settings;
		settings.mActiveEdgeMode = inActiveEdgesOnly? EActiveEdgeMode::CollideOnlyWithActive : EActiveEdgeMode::CollideWithAll;
		settings.mReturnDeepestPoint = true;

		Ref<Shape> test_shape = inTestShape->Create().Get();
		Ref<Shape> capsule = sCreateProbeCapsule();

		// Test hitting all active edges
		sTestCastShape(capsule, test_shape, inTestShapeScale, settings, Vec3(-4, cCapsuleProbeOffset, 0), Vec3(0.5f, 0, 0), { { Vec3(-3.5f, 0, 0), Vec3(1, 0, 0) } });
		sTestCastShape(capsule, test_shape, inTestShapeScale, settings, Vec3(4, cCapsuleProbeOffset, 0), Vec3(-0.5f, 0, 0), { { Vec3(3.5f, 0, 0), Vec3(-1, 0, 0) } });
		sTestCastShape(capsule, test_shape, inTestShapeScale, settings, Vec3(0, cCapsuleProbeOffset, -4), Vec3(0, 0, 0.5f), { { Vec3(0, 0, -3.5f), Vec3(0, 0, 1) } });
		sTestCastShape(capsule, test_shape, inTestShapeScale, settings, Vec3(0, cCapsuleProbeOffset, 4), Vec3(0, 0, -0.5f), { { Vec3(0, 0, 3.5f), Vec3(0, 0, -1) } });

		// Test hitting internal edges, this should return two hits
		sTestCastShape(capsule, test_shape, inTestShapeScale, settings, Vec3(-2.5f - 1.1f * cCapsuleRadius, cCapsuleProbeOffset, 0), Vec3(0.2f * cCapsuleRadius, 0, 0), { { Vec3(-2.5f, 0, 0), inActiveEdgesOnly? Vec3(0, -1, 0) : Vec3(-1, 0, 0) }, { Vec3(-2.5f, 0, 0), inActiveEdgesOnly? Vec3(0, -1, 0) : Vec3(1, 0, 0) } });
		sTestCastShape(capsule, test_shape, inTestShapeScale, settings, Vec3(0, cCapsuleProbeOffset, -2.5f - 1.1f * cCapsuleRadius), Vec3(0, 0, 0.2f * cCapsuleRadius), { { Vec3(0, 0, -2.5f), inActiveEdgesOnly? Vec3(0, -1, 0) : Vec3(0, 0, -1) }, { Vec3(0, 0, -2.5f), inActiveEdgesOnly? Vec3(0, -1, 0) : Vec3(0, 0, -1) } });
	}

	TEST_CASE("CastShapeMesh")
	{
		Ref<ShapeSettings> shape = sCreateMeshShape();

		sTestCastShape(shape, Vec3::sOne(), false);

		sTestCastShape(shape, Vec3::sOne(), true);

		sTestCastShape(shape, Vec3(-1, 1, 1), false);

		sTestCastShape(shape, Vec3(-1, 1, 1), true);
	}

	TEST_CASE("CastShapeHeightField")
	{
		Ref<ShapeSettings> shape = sCreateHeightFieldShape();

		sTestCastShape(shape, Vec3::sOne(), false);

		sTestCastShape(shape, Vec3::sOne(), true);

		sTestCastShape(shape, Vec3(-1, 1, 1), false);

		sTestCastShape(shape, Vec3(-1, 1, 1), true);
	}

	// Tests a discrete cube sliding over a mesh / heightfield shape
	static void sDiscreteCubeSlide(Ref<ShapeSettings> inShape, bool inCheckActiveEdges)
	{
		PhysicsTestContext c;

		const float cPenetrationSlop = c.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		// Set simulation settings
		PhysicsSettings settings;
		settings.mCheckActiveEdges = inCheckActiveEdges;
		c.GetSystem()->SetPhysicsSettings(settings);

		// Create frictionless floor
		Body &floor = c.CreateBody(inShape, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);
		floor.SetFriction(0.0f);

		// Create box sliding over the floor
		RVec3 initial_position(-3, 0.1f - cPenetrationSlop, 0);
		Vec3 initial_velocity(3, 0, 0);
		Body &box = c.CreateBox(initial_position, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::Discrete, Layers::MOVING, Vec3::sReplicate(0.1f));
		box.SetLinearVelocity(initial_velocity);
		box.SetFriction(0.0f);
		box.GetMotionProperties()->SetLinearDamping(0.0f);

		const float cSimulationTime = 2.0f;
		c.Simulate(cSimulationTime);

		RVec3 expected_position = initial_position + cSimulationTime * initial_velocity;
		if (inCheckActiveEdges)
		{
			// Box should have slided frictionless over the plane without encountering any collisions
			CHECK_APPROX_EQUAL(box.GetPosition(), expected_position, 1.0e-3f);
			CHECK_APPROX_EQUAL(box.GetLinearVelocity(), initial_velocity, 2.0e-3f);
		}
		else
		{
			// Box should have bumped into an internal edge and not reached its target
			CHECK(box.GetPosition().GetX() < expected_position.GetX() - 1.0f);
		}
	}

	TEST_CASE("DiscreteCubeSlideMesh")
	{
		Ref<ShapeSettings> shape = sCreateMeshShape();

		sDiscreteCubeSlide(shape, false);

		sDiscreteCubeSlide(shape, true);

		Ref<ShapeSettings> scaled_shape = new ScaledShapeSettings(shape, Vec3(-1, 1, 1));

		sDiscreteCubeSlide(scaled_shape, false);

		sDiscreteCubeSlide(scaled_shape, true);
	}

	TEST_CASE("DiscreteCubeSlideHeightField")
	{
		Ref<ShapeSettings> shape = sCreateHeightFieldShape();

		sDiscreteCubeSlide(shape, false);

		sDiscreteCubeSlide(shape, true);

		Ref<ShapeSettings> scaled_shape = new ScaledShapeSettings(shape, Vec3(-1, 1, 1));

		sDiscreteCubeSlide(scaled_shape, false);

		sDiscreteCubeSlide(scaled_shape, true);
	}

	// Tests a linear cast cube sliding over a mesh / heightfield shape
	static void sLinearCastCubeSlide(Ref<ShapeSettings> inShape, bool inCheckActiveEdges)
	{
		PhysicsTestContext c;

		const float cPenetrationSlop = c.GetSystem()->GetPhysicsSettings().mPenetrationSlop;

		// Set simulation settings
		PhysicsSettings settings;
		settings.mCheckActiveEdges = inCheckActiveEdges;
		c.GetSystem()->SetPhysicsSettings(settings);

		// Create frictionless floor
		Body &floor = c.CreateBody(inShape, RVec3::sZero(), Quat::sIdentity(), EMotionType::Static, EMotionQuality::Discrete, Layers::NON_MOVING, EActivation::DontActivate);
		floor.SetFriction(0.0f);

		// Create box starting a little bit above the floor and ending 0.5 * cPenetrationSlop below the floor so that if no internal edges are hit the motion should not be stopped
		// Note that we need the vertical velocity or else back face culling will ignore the face
		RVec3 initial_position(-3, 0.1f + cPenetrationSlop, 0);
		Vec3 initial_velocity(6 * 60, -1.5f * cPenetrationSlop * 60, 0);
		Body &box = c.CreateBox(initial_position, Quat::sIdentity(), EMotionType::Dynamic, EMotionQuality::LinearCast, Layers::MOVING, Vec3::sReplicate(0.1f));
		box.SetLinearVelocity(initial_velocity);
		box.SetFriction(0.0f);
		box.GetMotionProperties()->SetLinearDamping(0.0f);

		// To avoid extra vertical velocity being picked up in 1 step, zero gravity
		c.ZeroGravity();

		c.SimulateSingleStep();

		RVec3 expected_position = initial_position + initial_velocity / 60.0f;
		if (inCheckActiveEdges)
		{
			// Box should stepped in one frame over the plane without encountering any linear cast collisions
			CHECK_APPROX_EQUAL(box.GetPosition(), expected_position, 1.0e-4f);
			CHECK_APPROX_EQUAL(box.GetLinearVelocity(), initial_velocity, 1.0e-4f);
		}
		else
		{
			// Box should have bumped into an internal edge and not reached its target
			CHECK(box.GetPosition().GetX() < expected_position.GetX() - 1.0f);
		}
	}

	TEST_CASE("LinearCastCubeSlideMesh")
	{
		Ref<ShapeSettings> shape = sCreateMeshShape();

		sLinearCastCubeSlide(shape, false);

		sLinearCastCubeSlide(shape, true);

		Ref<ShapeSettings> scaled_shape = new ScaledShapeSettings(shape, Vec3(-1, 1, 1));

		sLinearCastCubeSlide(scaled_shape, false);

		sLinearCastCubeSlide(scaled_shape, true);
	}

	TEST_CASE("LinearCastCubeSlideHeightField")
	{
		Ref<ShapeSettings> shape = sCreateHeightFieldShape();

		sLinearCastCubeSlide(shape, false);

		sLinearCastCubeSlide(shape, true);

		Ref<ShapeSettings> scaled_shape = new ScaledShapeSettings(shape, Vec3(-1, 1, 1));

		sLinearCastCubeSlide(scaled_shape, false);

		sLinearCastCubeSlide(scaled_shape, true);
	}
}
