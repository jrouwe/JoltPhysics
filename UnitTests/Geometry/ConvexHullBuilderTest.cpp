// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Geometry/ConvexHullBuilder.h>

TEST_SUITE("ConvexHullBuilderTest")
{
	constexpr float cTolerance = 1.0e-3f;
	using Face = ConvexHullBuilder::Face;
	using Positions = ConvexHullBuilder::Positions;

	TEST_CASE("TestDegenerate")
	{
		const char *error = nullptr;

		{
			// Too few points / coinciding points should be degenerate
			Positions positions { Vec3(1, 2, 3) };

			ConvexHullBuilder builder(positions);
			CHECK(builder.Initialize(INT_MAX, cTolerance, error) == ConvexHullBuilder::EResult::TooFewPoints);
			positions.push_back(Vec3(1 + 0.5f * cTolerance, 2, 3));
			CHECK(builder.Initialize(INT_MAX, cTolerance, error) == ConvexHullBuilder::EResult::TooFewPoints);
			positions.push_back(Vec3(1, 2 + 0.5f * cTolerance, 3));
			CHECK(builder.Initialize(INT_MAX, cTolerance, error) == ConvexHullBuilder::EResult::Degenerate);
			positions.push_back(Vec3(1, 2, 3 + 0.5f * cTolerance));
			CHECK(builder.Initialize(INT_MAX, cTolerance, error) == ConvexHullBuilder::EResult::Degenerate);
		}

		{
			// A line should be degenerate
			Positions positions;
			for (float v = 0.0f; v < 1.01f; v += 0.1f)
				positions.push_back(Vec3(v, 0, 0));

			ConvexHullBuilder builder(positions);
			CHECK(builder.Initialize(INT_MAX, cTolerance, error) == ConvexHullBuilder::EResult::Degenerate);
		}
	}

	TEST_CASE("Test2DHull")
	{
		const char *error = nullptr;

		{
			// A triangle
			Positions positions { Vec3(-1, 0, -1), Vec3(1, 0, -1), Vec3(-1, 0, 1) };

			ConvexHullBuilder builder(positions);
			CHECK(builder.Initialize(INT_MAX, cTolerance, error) == ConvexHullBuilder::EResult::Success);
			CHECK(builder.GetNumVerticesUsed() == 3);
			CHECK(builder.GetFaces().size() == 2);
			CHECK(builder.ContainsFace({ 0, 1, 2 }));
			CHECK(builder.ContainsFace({ 2, 1, 0 }));
		}

		{
			// A quad with many interior points
			Positions positions;
			for (int x = 0; x < 10; ++x)
				for (int z = 0; z < 10; ++z)
					positions.push_back(Vec3(0.1f * x, 0, 1.0f * 0.2f * z));

			ConvexHullBuilder builder(positions);
			CHECK(builder.Initialize(INT_MAX, cTolerance, error) == ConvexHullBuilder::EResult::Success);
			CHECK(builder.GetNumVerticesUsed() == 4);
			CHECK(builder.GetFaces().size() == 2);
			CHECK(builder.ContainsFace({ 0, 9, 99, 90 }));
			CHECK(builder.ContainsFace({ 90, 99, 9, 0 }));
		}

		{
			// Add disc with many interior points
			Positions positions;
			for (int r = 0; r < 10; ++r)
				for (int phi = 0; phi < 10; ++phi)
				{
					float f_r = 2.0f * r;
					float f_phi = 2.0f * JPH_PI * phi / 10;
					positions.push_back(Vec3(f_r * Cos(f_phi), f_r * Sin(f_phi), 0));
				}

			ConvexHullBuilder builder(positions);
			CHECK(builder.Initialize(INT_MAX, cTolerance, error) == ConvexHullBuilder::EResult::Success);
			CHECK(builder.GetNumVerticesUsed() == 10);
			CHECK(builder.GetFaces().size() == 2);
			CHECK(builder.ContainsFace({ 90, 91, 92, 93, 94, 95, 96, 97, 98, 99 }));
			CHECK(builder.ContainsFace({ 99, 98, 97, 96, 95, 94, 93, 92, 91, 90 }));
		}
	}

	TEST_CASE("Test3DHull")
	{
		const char *error = nullptr;

		{
			// A cube with lots of interior points
			Positions positions;
			for (int x = 0; x < 10; ++x)
				for (int y = 0; y < 10; ++y)
					for (int z = 0; z < 10; ++z)
					positions.push_back(Vec3(0.1f * x, 1.0f + 0.2f * y, 2.0f * 0.3f * z));

			ConvexHullBuilder builder(positions);
			CHECK(builder.Initialize(INT_MAX, cTolerance, error) == ConvexHullBuilder::EResult::Success);
			CHECK(builder.GetNumVerticesUsed() == 8);
			CHECK(builder.GetFaces().size() == 6);
			CHECK(builder.ContainsFace({ 0, 9, 99, 90 }));
			CHECK(builder.ContainsFace({ 0, 90, 990, 900 }));
			CHECK(builder.ContainsFace({ 900, 990, 999, 909 }));
			CHECK(builder.ContainsFace({ 9, 909, 999, 99 }));
			CHECK(builder.ContainsFace({ 90, 99, 999, 990 }));
			CHECK(builder.ContainsFace({ 0, 900, 909, 9 }));
		}

		{
			// Add sphere with many interior points
			Positions positions;
			for (int r = 0; r < 10; ++r)
				for (int phi = 0; phi < 10; ++phi)
					for (int theta = 0; theta < 10; ++theta)
					{
						float f_r = 2.0f * r;
						float f_phi = 2.0f * JPH_PI * phi / 10; // [0, 2 PI)
						float f_theta = JPH_PI * theta / 9; // [0, PI] (inclusive!)
						positions.push_back(f_r * Vec3::sUnitSpherical(f_theta, f_phi));
					}

			ConvexHullBuilder builder(positions);
			CHECK(builder.Initialize(INT_MAX, cTolerance, error) == ConvexHullBuilder::EResult::Success);
			CHECK(builder.GetNumVerticesUsed() == 82); // The two ends of the sphere have 10 points that have the same position

			// Too many faces, calculate the error instead
			Face *error_face;
			float max_error, coplanar_distance;
			int error_position_idx;
			builder.DetermineMaxError(error_face, max_error, error_position_idx, coplanar_distance);
			CHECK(max_error < max(coplanar_distance, cTolerance));
		}
	}

	TEST_CASE("TestRandomHull")
	{
		const char *error = nullptr;

		UnitTestRandom random(0x1ee7c0de);

		uniform_real_distribution<float> zero_one(0.0f, 1.0f);
		uniform_real_distribution<float> zero_two(0.0f, 2.0f);
		uniform_real_distribution<float> scale_start(0.1f, 0.5f);
		uniform_real_distribution<float> scale_range(0.1f, 2.0f);

		for (int iteration = 0; iteration < 100; ++iteration)
		{
			// Define vertex scale
			float start = scale_start(random);
			uniform_real_distribution<float> vertex_scale(start, start + scale_range(random));

			// Define shape scale to make shape less sphere like
			uniform_real_distribution<float> shape_scale(0.1f, 1.0f);
			Vec3 scale(shape_scale(random), shape_scale(random), shape_scale(random));

			// Add some random points
			Positions positions;
			for (int i = 0; i < 100; ++i)
			{
				// Add random point
				Vec3 p1 = vertex_scale(random) * Vec3::sRandom(random) * scale;
				positions.push_back(p1);

				// Point close to p1
				Vec3 p2 = p1 + cTolerance * zero_two(random) * Vec3::sRandom(random);
				positions.push_back(p2);

				// Point on a line to another point
				float fraction = zero_one(random);
				Vec3 p3 = fraction * p1 + (1.0f - fraction) * positions[random() % positions.size()];
				positions.push_back(p3);

				// Point close to p3
				Vec3 p4 = p3 + cTolerance * zero_two(random) * Vec3::sRandom(random);
				positions.push_back(p4);
			}

			// Build hull
			ConvexHullBuilder builder(positions);
			CHECK(builder.Initialize(INT_MAX, cTolerance, error) == ConvexHullBuilder::EResult::Success);

			// Calculate error
			Face *error_face;
			float max_error, coplanar_distance;
			int error_position_idx;
			builder.DetermineMaxError(error_face, max_error, error_position_idx, coplanar_distance);
			CHECK(max_error < max(coplanar_distance, 1.2f * cTolerance));
		}
	}
}
