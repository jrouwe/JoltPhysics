// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <TestFramework.h>

#include <Tests/ConvexCollision/ConvexHullShrinkTest.h>
#include <Utils/Log.h>
#include <Utils/DebugRendererSP.h>
#include <Jolt/Geometry/ConvexSupport.h>
#include <Jolt/Physics/Collision/Shape/ConvexHullShape.h>
#include <Renderer/DebugRendererImp.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <fstream>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_IMPLEMENT_RTTI_VIRTUAL(ConvexHullShrinkTest)
{
	JPH_ADD_BASE_CLASS(ConvexHullShrinkTest, Test)
}

void ConvexHullShrinkTest::Initialize()
{
	// First add a list of shapes that were problematic before
	mPoints = {
		{
			Vec3(1, 1, 1),
			Vec3(1, 1, -1),
			Vec3(1, -1, 1),
			Vec3(1, -1, -1),
		},
		{
			Vec3(1, 1, 1),
			Vec3(1, 1, -1),
			Vec3(1, -1, 1),
			Vec3(1, -1, -1),
			Vec3(-1, 1, 1),
			Vec3(-1, 1, -1),
			Vec3(-1, -1, 1),
			Vec3(-1, -1, -1),
		},
		{
			Vec3(0.24055352f, 0.42262089f, 0.20811508f),
			Vec3(0.23034751f, 0.42984104f, -0.21389426f),
			Vec3(0.21995061f, 0.43724900f, 0.20929135f),
			Vec3(0.18619442f, 0.44122630f, 0.10257969f),
			Vec3(-0.22997921f, 0.43706810f, 0.21128670f),
			Vec3(0.18488347f, -0.44135576f, 0.10415942f),
			Vec3(-0.20950880f, -0.43603044f, 0.20873074f),
			Vec3(-0.21230474f, -0.43691945f, -0.20506332f),
			Vec3(0.23440370f, -0.43392032f, 0.20985059f),
			Vec3(0.22406587f, -0.43578571f, -0.21132792f),
			Vec3(0.24845430f, -0.41821426f, -0.21033705f),
			Vec3(0.24780219f, -0.42262548f, 0.21058462f),
			Vec3(-0.24866026f, 0.41188520f, 0.20908103f),
			Vec3(-0.25144735f, 0.41933101f, -0.20718251f),
			Vec3(-0.24799588f, -0.20490804f, 0.21178717f),
			Vec3(0.01075744f, -0.41775572f, -0.22181017f),
			Vec3(-0.18624404f, -0.18736419f, -0.21975047f),
			Vec3(0.22080457f, 0.01773871f, -0.22080121f),
			Vec3(-0.17988407f, 0.40095943f, -0.21670545f),
			Vec3(-0.23094913f, 0.42154532f, 0.21846796f),
			Vec3(0.23783659f, 0.41114848f, -0.20812420f),
			Vec3(0.25242796f, 0.00087111f, 0.04875314f),
			Vec3(0.20976084f, 0.43694448f, -0.20819492f),
			Vec3(0.21914389f, -0.42215359f, -0.21839635f),
			Vec3(0.22120973f, 0.42172050f, 0.21581716f),
			Vec3(0.07287904f, 0.40937370f, 0.21898652f),
			Vec3(-0.23638439f, 0.42299985f, -0.21391643f),
			Vec3(0.25210538f, -0.20603905f, 0.20603551f),
			Vec3(-0.22867783f, -0.43080616f, -0.21309699f),
			Vec3(-0.22365719f, 0.43650645f, -0.20515810f),
			Vec3(-0.23701435f, 0.43320888f, -0.20985882f),
			Vec3(-0.24509817f, 0.42541492f, 0.21352110f),
			Vec3(0.22803798f, -0.41877448f, 0.21590335f),
			Vec3(-0.21627685f, -0.41884291f, 0.21908275f),
			Vec3(-0.24125161f, -0.13299965f, -0.21386964f),
			Vec3(-0.22310710f, -0.43280768f, 0.21368177f),
			Vec3(-0.23707944f, -0.41916745f, 0.21170078f),
			Vec3(-0.23729360f, -0.42400050f, -0.20905880f),
			Vec3(-0.23056241f, 0.44033193f, -0.00191451f),
			Vec3(-0.24118152f, -0.41101628f, -0.20855166f),
			Vec3(0.21646300f, 0.42087674f, -0.21763385f),
			Vec3(0.25090047f, -0.41023433f, 0.10248772f),
			Vec3(0.03950108f, -0.43627834f, -0.21231101f),
			Vec3(-0.22727611f, -0.24993966f, 0.21899925f),
			Vec3(0.24388977f, -0.07015021f, -0.21204789f)
		}
	};

	// Open the external file with hulls
	// A stream containing predefined convex hulls
	ifstream points_stream("Assets/convex_hulls.bin", std::ios::binary);
	if (points_stream.is_open())
	{
		for (;;)
		{
			// Read the length of the next point cloud
			uint32 len = 0;
			points_stream.read((char *)&len, sizeof(len));
			if (points_stream.eof())
				break;

			// Read the points
			if (len > 0)
			{
				Points p;
				for (uint32 i = 0; i < len; ++i)
				{
					Float3 v;
					points_stream.read((char *)&v, sizeof(v));
					p.push_back(Vec3(v));
				}
				mPoints.push_back(std::move(p));
			}
		}
	}
}

void ConvexHullShrinkTest::PrePhysicsUpdate(const PreUpdateParams &inParams)
{
	// Take one of the predefined shapes
	const Points &points = mIteration < mPoints.size()? mPoints[mIteration] : mPoints.back();
	mIteration++;

	// Create shape
	ConvexHullShapeSettings settings(points, cDefaultConvexRadius);
	Shape::ShapeResult result = settings.Create();
	if (!result.IsValid())
	{
		Trace("%d: %s", mIteration - 1, result.GetError().c_str());
		return;
	}
	RefConst<ConvexHullShape> shape = StaticCast<ConvexHullShape>(result.Get());

	// Shape creation may have reduced the convex radius, fetch the result
	const float convex_radius = shape->GetConvexRadius();
	if (convex_radius > 0.0f)
	{
		// Get the support function of the shape excluding convex radius and add the convex radius
		ConvexShape::SupportBuffer buffer;
		const ConvexShape::Support *support = shape->GetSupportFunction(ConvexShape::ESupportMode::ExcludeConvexRadius, buffer, Vec3::sReplicate(1.0f));
		AddConvexRadius<ConvexShape::Support> add_cvx(*support, convex_radius);

		// Calculate the error w.r.t. the original hull
		float max_error = -FLT_MAX;
		int max_error_plane = 0;
		Vec3 max_error_support_point = Vec3::sZero();
		const Array<Plane> &planes = shape->GetPlanes();
		for (int i = 0; i < (int)planes.size(); ++i)
		{
			const Plane &plane = planes[i];
			Vec3 support_point = add_cvx.GetSupport(plane.GetNormal());
			float distance = plane.SignedDistance(support_point);
			if (distance > max_error)
			{
				max_error = distance;
				max_error_support_point = support_point;
				max_error_plane = i;
			}
		}
		if (max_error > settings.mMaxErrorConvexRadius)
		{
			Trace("%d, %f, %f", mIteration - 1, (double)convex_radius, (double)max_error);
			DrawMarkerSP(mDebugRenderer, max_error_support_point, Color::sPurple, 0.1f);
			DrawArrowSP(mDebugRenderer, max_error_support_point, max_error_support_point - max_error * planes[max_error_plane].GetNormal(), Color::sPurple, 0.01f);
		}
	}

#ifdef JPH_DEBUG_RENDERER
	// Draw the hulls
	shape->Draw(DebugRenderer::sInstance, RMat44::sIdentity(), Vec3::sReplicate(1.0f), Color::sRed, false, false);
	shape->DrawGetSupportFunction(DebugRenderer::sInstance, RMat44::sIdentity(), Vec3::sReplicate(1.0f), Color::sLightGrey, false);
	shape->DrawShrunkShape(DebugRenderer::sInstance, RMat44::sIdentity(), Vec3::sReplicate(1.0f));
#endif // JPH_DEBUG_RENDERER
}
