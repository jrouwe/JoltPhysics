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

	TEST_CASE("TestHullEdgeCases")
	{
		const char *error = nullptr;

		Positions positions[] = {
			{
				// A hull with 2 faces that are nearly coplanar
				Vec3(-0.020472288f, -0.195635557f, 0.308015466f),
				Vec3(0.136248738f, 0.633286834f, 0.135366619f),
				Vec3(0.286418647f, -0.228475571f, 0.308084548f),
				Vec3(-0.267285109f, 1.024676085f, 0.308042824f),
				Vec3(0.396568149f, -0.971658647f, 0.308055162f),
				Vec3(0.321081549f, -1.024676085f, 0.308036327f),
				Vec3(0.034643859f, -0.404506862f, 0.308015764f),
				Vec3(0.189224690f, -0.252762139f, 0.308060408f),
			},
			{
				// Nearly coplanar points
				Vec3(0.917345762f, 0.157111734f, 1.650970459f),
				Vec3(-0.098074198f, 0.157116055f, 0.664742708f),
				Vec3(1.777100325f, 0.157112047f, 1.238879442f),
				Vec3(2.114324570f, 0.157112464f, 0.780688763f),
				Vec3(1.926570415f, 0.157114446f, 0.240761161f),
				Vec3(-1.045998096f, 0.157108605f, 1.548911095f),
				Vec3(-1.820045233f, 0.157106474f, 1.050360918f),
				Vec3(-1.918573976f, 0.157108605f, 0.039246202f),
				Vec3(0.042619467f, 0.157113969f, -1.405336142f),
				Vec3(0.575986624f, 0.157114401f, -1.370834589f),
				Vec3(1.402592659f, 0.157115221f, -0.834864557f),
				Vec3(1.110557318f, 0.157113969f, -1.336267948f),
				Vec3(1.689781666f, 0.157115355f, -0.308773756f),
				Vec3(2.205337524f, 0.157113209f, -0.281754494f),
				Vec3(-1.346967936f, 0.157110974f, -0.978962541f),
				Vec3(-1.346967936f, 0.157110974f, -0.978962541f),
				Vec3(-2.085033417f, 0.157106936f, -0.506602883f),
				Vec3(-0.981224537f, 0.157110706f, -1.445893764f),
				Vec3(-0.481085658f, 0.157112658f, -1.426232934f),
				Vec3(-0.981224537f, 0.157110706f, -1.445893764f),
			}
		};

		for (const Positions &p : positions)
		{
			// Build hull
			ConvexHullBuilder builder(p);
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
