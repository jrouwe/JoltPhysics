// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/Physics/Collision/Shape/TaperedCylinderShape.h>
#include <Jolt/Physics/Collision/Shape/CylinderShape.h>
#include <Jolt/Physics/Collision/Shape/ScaleHelpers.h>
#include <Jolt/Physics/SoftBody/SoftBodyVertex.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>
#include <Jolt/Core/StreamIn.h>
#include <Jolt/Core/StreamOut.h>
#ifdef JPH_DEBUG_RENDERER
	#include <Jolt/Renderer/DebugRenderer.h>
#endif // JPH_DEBUG_RENDERER

JPH_NAMESPACE_BEGIN

// Approximation of a face of the tapered capsule
static const Vec3 cTaperedCapsuleFace[] =
{
	Vec3(0.0f,			0.0f,	1.0f),
	Vec3(0.7071067f,	0.0f,	0.7071067f),
	Vec3(1.0f,			0.0f,	0.0f),
	Vec3(0.7071067f,	0.0f,	-0.7071067f),
	Vec3(-0.0f,			0.0f,	-1.0f),
	Vec3(-0.7071067f,	0.0f,	-0.7071067f),
	Vec3(-1.0f,			0.0f,	0.0f),
	Vec3(-0.7071067f,	0.0f,	0.7071067f)
};

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(TaperedCylinderShapeSettings)
{
	JPH_ADD_BASE_CLASS(TaperedCylinderShapeSettings, ConvexShapeSettings)

	JPH_ADD_ATTRIBUTE(TaperedCylinderShapeSettings, mHalfHeight)
	JPH_ADD_ATTRIBUTE(TaperedCylinderShapeSettings, mTopRadius)
	JPH_ADD_ATTRIBUTE(TaperedCylinderShapeSettings, mBottomRadius)
	JPH_ADD_ATTRIBUTE(TaperedCylinderShapeSettings, mConvexRadius)
}

ShapeSettings::ShapeResult TaperedCylinderShapeSettings::Create() const
{
	if (mCachedResult.IsEmpty())
	{
		Ref<Shape> shape;
		if (mTopRadius == mBottomRadius)
		{
			// Convert to regular cylinder
			CylinderShapeSettings settings;
			settings.mHalfHeight = mHalfHeight;
			settings.mRadius = mTopRadius;
			settings.mMaterial = mMaterial;
			settings.mConvexRadius = mConvexRadius;
			new CylinderShape(settings, mCachedResult);
		}
		else
		{
			// Normal tapered cylinder shape
			new TaperedCylinderShape(*this, mCachedResult);
		}
	}
	return mCachedResult;
}

TaperedCylinderShapeSettings::TaperedCylinderShapeSettings(float inHalfHeightOfTaperedCylinder, float inTopRadius, float inBottomRadius, float inConvexRadius, const PhysicsMaterial *inMaterial) :
	ConvexShapeSettings(inMaterial),
	mHalfHeight(inHalfHeightOfTaperedCylinder),
	mTopRadius(inTopRadius),
	mBottomRadius(inBottomRadius),
	mConvexRadius(inConvexRadius)
{
}

TaperedCylinderShape::TaperedCylinderShape(const TaperedCylinderShapeSettings &inSettings, ShapeResult &outResult) :
	ConvexShape(EShapeSubType::TaperedCylinder, inSettings, outResult),
	mTopRadius(inSettings.mTopRadius),
	mBottomRadius(inSettings.mBottomRadius),
	mConvexRadius(inSettings.mConvexRadius)
{
	if (mTopRadius < 0.0f)
	{
		outResult.SetError("Invalid top radius");
		return;
	}

	if (mBottomRadius < 0.0f)
	{
		outResult.SetError("Invalid bottom radius");
		return;
	}

	if (inSettings.mHalfHeight <= 0.0f)
	{
		outResult.SetError("Invalid height");
		return;
	}

	if (inSettings.mConvexRadius < 0.0f)
	{
		outResult.SetError("Invalid convex radius");
		return;
	}

	if (inSettings.mTopRadius < inSettings.mConvexRadius)
	{
		outResult.SetError("Convex radius must be smaller than convex radius");
		return;
	}

	if (inSettings.mBottomRadius < inSettings.mConvexRadius)
	{
		outResult.SetError("Convex radius must be smaller than bottom radius");
		return;
	}

	// Calculate the center of mass (using wxMaxima).
	// We define the cylinder with bottom at x=0 and radius b
	// The top is at x=h width b + dr
	// Radius as a function of x is: r(x):=b+x*dr/h;
	// Area at x is: a(x):=%pi*r(x)^2;
	// Volume of a cylinder x = [0, c]: v(c):=integrate(a(x),x,0,c);
	// Solving for c: c(b,dr):=factor(rhs(ratsimp(solve(v(h)/2=v(c),c))[3]));
	// Result: c(b,dr):=((((dr^3+3*(b*dr^2+b^2*dr)+2*b^3)/2)^(1/3)-b)*h)/dr
	float dr = mTopRadius - mBottomRadius;
	if (abs(dr) < 1.0e-4f)
	{
		// If difference is small, just center the cylinder to avoid dividing by zero
		mTop = inSettings.mHalfHeight;
		mBottom = -inSettings.mHalfHeight;
	}
	else
	{
		float h = 2.0f * inSettings.mHalfHeight;
		float b = mBottomRadius;
		float b2 = Square(b);
		float b3 = b * b2;
		float dr2 = Square(dr);
		float dr3 = dr * dr2;
		float c = ((std::pow(0.5f * dr3 + 1.5f * (b * dr2 + b2 * dr) + b3, 1.0f / 3.0f) - b) * h) / dr;
		mTop = h - c;
		mBottom = -c;
	}

	outResult.Set(this);
}

class TaperedCylinderShape::TaperedCylinder final : public Support
{
public:
					TaperedCylinder(float inTop, float inBottom, float inTopRadius, float inBottomRadius, float inConvexRadius) :
		mTop(inTop),
		mBottom(inBottom),
		mTopRadius(inTopRadius),
		mBottomRadius(inBottomRadius),
		mConvexRadius(inConvexRadius)
	{
		static_assert(sizeof(TaperedCylinder) <= sizeof(SupportBuffer), "Buffer size too small");
		JPH_ASSERT(IsAligned(this, alignof(TaperedCylinder)));
	}

	virtual Vec3	GetSupport(Vec3Arg inDirection) const override
	{
		float x = inDirection.GetX(), y = inDirection.GetY(), z = inDirection.GetZ();
		float o = sqrt(Square(x) + Square(z));
		if (o > 0.0f)
		{
			Vec3 top_support((mTopRadius * x) / o, mTop, (mTopRadius * z) / o);
			Vec3 bottom_support((mBottomRadius * x) / o, mBottom, (mBottomRadius * z) / o);
			return inDirection.Dot(top_support) > inDirection.Dot(bottom_support)? top_support : bottom_support;
		}
		else
		{
			if (y > 0.0f)
				return Vec3(0, mTop, 0);
			else
				return Vec3(0, mBottom, 0);
		}
	}

	virtual float	GetConvexRadius() const override
	{
		return mConvexRadius;
	}

private:
	float			mTop;
	float			mBottom;
	float			mTopRadius;
	float			mBottomRadius;
	float			mConvexRadius;
};

JPH_INLINE void TaperedCylinderShape::GetScaled(Vec3Arg inScale, float &outTop, float &outBottom, float &outTopRadius, float &outBottomRadius, float &outConvexRadius) const
{
	Vec3 abs_scale = inScale.Abs();
	float scale_xz = abs_scale.GetX();
	float scale_y = inScale.GetY();

	outTop = scale_y * mTop;
	outBottom = scale_y * mBottom;
	outTopRadius = scale_xz * mTopRadius;
	outBottomRadius = scale_xz * mBottomRadius;
	outConvexRadius = min(abs_scale.GetY(), scale_xz) * mConvexRadius;

	// Negative Y-scale flips the top and bottom
	if (outBottom > outTop)
	{
		swap(outTop, outBottom);
		swap(outTopRadius, outBottomRadius);
	}
}

const ConvexShape::Support *TaperedCylinderShape::GetSupportFunction(ESupportMode inMode, SupportBuffer &inBuffer, Vec3Arg inScale) const
{
	JPH_ASSERT(IsValidScale(inScale));

	// Get scaled tapered cylinder
	float top, bottom, top_radius, bottom_radius, convex_radius;
	GetScaled(inScale, top, bottom, top_radius, bottom_radius, convex_radius);

	switch (inMode)
	{
	case ESupportMode::IncludeConvexRadius:
	case ESupportMode::Default:
		return new (&inBuffer) TaperedCylinder(top, bottom, top_radius, bottom_radius, 0.0f);

	case ESupportMode::ExcludeConvexRadius:
		return new (&inBuffer) TaperedCylinder(top - convex_radius, bottom + convex_radius, top_radius - convex_radius, bottom_radius - convex_radius, convex_radius);
	}

	JPH_ASSERT(false);
	return nullptr;
}

JPH_INLINE static Vec3 sCalculateSideNormalXZ(Vec3Arg inSurfacePosition)
{
	return (Vec3(1, 0, 1) * inSurfacePosition).NormalizedOr(Vec3::sAxisX());
}

JPH_INLINE static Vec3 sCalculateSideNormal(Vec3Arg inNormalXZ, float inTop, float inBottom, float inTopRadius, float inBottomRadius)
{
	float tan_alpha = (inBottomRadius - inTopRadius) / (inTop - inBottom);
	return Vec3(inNormalXZ.GetX(), tan_alpha, inNormalXZ.GetZ()).Normalized();
}

void TaperedCylinderShape::GetSupportingFace(const SubShapeID &inSubShapeID, Vec3Arg inDirection, Vec3Arg inScale, Mat44Arg inCenterOfMassTransform, SupportingFace &outVertices) const
{
	JPH_ASSERT(inSubShapeID.IsEmpty(), "Invalid subshape ID");
	JPH_ASSERT(IsValidScale(inScale));

	// Get scaled tapered cylinder
	float top, bottom, top_radius, bottom_radius, convex_radius;
	GetScaled(inScale, top, bottom, top_radius, bottom_radius, convex_radius);

	// Get the normal of the side of the cylinder
	Vec3 normal_xz = sCalculateSideNormalXZ(-inDirection);
	Vec3 normal = sCalculateSideNormal(normal_xz, top, bottom, top_radius, bottom_radius);

	constexpr float cMinRadius = 1.0e-3f;

	// Check if the normal is closer to the side than to the top or bottom
	if (abs(normal.Dot(inDirection)) > abs(inDirection.GetY()))
	{
		// Return the side of the cylinder
		outVertices.push_back(inCenterOfMassTransform * (normal_xz * top_radius + Vec3(0, top, 0)));
		outVertices.push_back(inCenterOfMassTransform * (normal_xz * bottom_radius + Vec3(0, bottom, 0)));
	}
	else if (inDirection.GetY() < 0.0f)
	{
		// Top of the cylinder
		if (top_radius > cMinRadius)
		{
			Vec3 top_3d(0, top, 0);
			for (Vec3 v : cTaperedCapsuleFace)
				outVertices.push_back(inCenterOfMassTransform * (top_radius * v + top_3d));
		}
	}
	else
	{
		// Bottom of the cylinder
		if (bottom_radius > cMinRadius)
		{
			Vec3 bottom_3d(0, bottom, 0);
			for (const Vec3 *v = cTaperedCapsuleFace + std::size(cTaperedCapsuleFace) - 1; v >= cTaperedCapsuleFace; --v)
				outVertices.push_back(inCenterOfMassTransform * (bottom_radius * *v + bottom_3d));
		}
	}
}

MassProperties TaperedCylinderShape::GetMassProperties() const
{
	MassProperties p;
	p.mMass = GetVolume() * GetDensity();

	// Calculate inertia of a tapered cylinder (using wxMaxima)
	// Radius: r(x):=br+x*drdh;
	// Where drdh=(top_radius-bottom_radius)/(top-bottom)
	// Delta mass (note: needs to be multiplied by density but we're leaving that until later as it is a constant)
	// dm(x):=%pi*r(x)^2;
	// Delta inertia x (using inertia of a solid disc, see https://en.wikipedia.org/wiki/List_of_moments_of_inertia):
	// dix(x):=dm(x)*r(x)^2/4;
	// Delta inertia y:
	// diy(x):=dm(x)*r(x)^2/2;
	// The constant density:
	// density(b,t):=m/integrate(dm(x),x,b,t);
	// Inertia tensor element xx, note that we use the parallel axis theorem to move the inertia: Ixx' = Ixx + m translation^2
	// Ixx(br,drdh,b,t):=integrate(dix(x)+dm(x)*x^2,x,b,t)*density(b,t);
	// Inertia tensor element yy:
	// Iyy(br,drdh,b,t):=integrate(diy(x),x,b,t)*density(b,t);
	// For a cylinder this formula matches what is listed on the wiki:
	// factor(Ixx(r,0,-h/2,h/2));
	// factor(Iyy(r,0,-h/2,h/2));
	// For a cone with tip at origin too:
	// factor(Ixx(0,r/h,0,h));
	// factor(Iyy(0,r/h,0,h));
	// Now for the tapered cylinder:
	// rat(Ixx(br,drdh,b,t),br,dr);
	// rat(Iyy(br,drdh,b,t),br,dr);
	float br = mBottomRadius;
	float br2 = Square(br);
	float br3 = br * br2;
	float br4 = Square(br2);
	float drdh = (mTopRadius - br) / (mTop - mBottom);
	float drdh2 = Square(drdh);
	float drdh3 = drdh * drdh2;
	float drdh4 = drdh2 * drdh2;
	float t = mTop;
	float t2 = Square(t);
	float t3 = t * t2;
	float t4 = Square(t2);
	float b = mBottom;
	float b2 = Square(b);
	float b3 = b * b2;
	float b4 = Square(b2);
	float f0 = t+b;
	float f1 = t2+b*t+b2;
	float f2 = t3+b*t2+b2*t+b3;
	float f3 = t4+b*t3+b2*t2+b3*t+b4;
	float denom = 10.0f*f1*drdh2+30.0f*(f0*br*drdh+br2);
	float inertia_y = p.mMass*(3.0f*f3*drdh4+15.0f*f2*br*drdh3+30.0f*(f1*br2*drdh2+f0*br3*drdh)+15.0f*br4)/denom;
	float inertia_x = 0.5f*(inertia_y+p.mMass*(12.0f*f3*drdh2+30.0f*f2*br*drdh+20.0f*f1*br2)/denom);
	float inertia_z = inertia_x;
	p.mInertia = Mat44::sScale(Vec3(inertia_x, inertia_y, inertia_z));
	return p;
}

Vec3 TaperedCylinderShape::GetSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inLocalSurfacePosition) const
{
	JPH_ASSERT(inSubShapeID.IsEmpty(), "Invalid subshape ID");

	constexpr float cEpsilon = 1.0e-5f;

	if (inLocalSurfacePosition.GetY() > mTop - cEpsilon)
		return Vec3(0, 1, 0);
	else if (inLocalSurfacePosition.GetY() < mBottom + cEpsilon)
		return Vec3(0, -1, 0);
	else
		return sCalculateSideNormal(sCalculateSideNormalXZ(inLocalSurfacePosition), mTop, mBottom, mTopRadius, mBottomRadius);
}

AABox TaperedCylinderShape::GetLocalBounds() const
{
	float max_radius = max(mTopRadius, mBottomRadius);
	return AABox(Vec3(-max_radius, mBottom, -max_radius), Vec3(max_radius, mTop, max_radius));
}

void TaperedCylinderShape::CollideSoftBodyVertices(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, SoftBodyVertex *ioVertices, uint inNumVertices, [[maybe_unused]] float inDeltaTime, [[maybe_unused]] Vec3Arg inDisplacementDueToGravity, int inCollidingShapeIndex) const
{
	JPH_ASSERT(IsValidScale(inScale));

	Mat44 inverse_transform = inCenterOfMassTransform.InversedRotationTranslation();

	// Get scaled tapered cylinder
	float top, bottom, top_radius, bottom_radius, convex_radius;
	GetScaled(inScale, top, bottom, top_radius, bottom_radius, convex_radius);
	Vec3 top_3d(0, top, 0);
	Vec3 bottom_3d(0, bottom, 0);

	for (SoftBodyVertex *v = ioVertices, *sbv_end = ioVertices + inNumVertices; v < sbv_end; ++v)
		if (v->mInvMass > 0.0f)
		{
			Vec3 local_pos = inverse_transform * v->mPosition;

			// Calculate penetration into side surface
			Vec3 normal_xz = sCalculateSideNormalXZ(local_pos);
			Vec3 side_normal = sCalculateSideNormal(normal_xz, top, bottom, top_radius, bottom_radius);
			Vec3 side_support_top = normal_xz * top_radius + top_3d;
			float side_penetration = (side_support_top - local_pos).Dot(side_normal);

			// Calculate penetration into top and bottom plane
			float top_penetration = top - local_pos.GetY();
			float bottom_penetration = local_pos.GetY() - bottom;
			float min_top_bottom_penetration = min(top_penetration, bottom_penetration);

			Vec3 point, normal;
			if (side_penetration < 0.0f || min_top_bottom_penetration < 0.0f)
			{
				// We're outside the cylinder
				// Calculate the closest point on the line segment from bottom to top support point:
				// closest_point = bottom + fraction * (top - bottom) / |top - bottom|^2
				Vec3 side_support_bottom = normal_xz * bottom_radius + bottom_3d;
				Vec3 bottom_to_top = side_support_top - side_support_bottom;
				float fraction = (local_pos - side_support_bottom).Dot(bottom_to_top);

				// Calculate the distance to the axis of the cylinder
				float distance_to_axis = normal_xz.Dot(local_pos);
				bool inside_top_radius = distance_to_axis <= top_radius;
				bool inside_bottom_radius = distance_to_axis <= bottom_radius;

				// Regions of tapered cylinder (side view):
				//
				//     _  B |       |
				//      --_ |   A   |
				//          t-------+
				//    C    /         \
				//        /  tapered  \
				// _     /  cylinder   \
				//  --_ /               \
				//     b-----------------+
				//  D  |        E        |
				//     |                 |
				//
				// t = side_support_top, b = side_support_bottom
				// Lines between B and C and C and D are at a 90 degree angle to the line between t and b
				if (fraction >= bottom_to_top.LengthSq() // Region B: Above the line segment
					&& !inside_top_radius) // Outside the top radius
				{
					// Top support point is closest
					point = side_support_top;
					normal = (local_pos - point).NormalizedOr(Vec3::sAxisY());
				}
				else if (fraction < 0.0f // Region D: Below the line segment
					&& !inside_bottom_radius) // Outside the bottom radius
				{
					// Bottom support point is closest
					point = side_support_bottom;
					normal = (local_pos - point).NormalizedOr(Vec3::sAxisY());
				}
				else if (top_penetration < 0.0f // Region A: Above the top plane
					&& inside_top_radius) // Inside the top radius
				{
					// Top plane is closest
					point = top_3d;
					normal = Vec3(0, 1, 0);
				}
				else if (bottom_penetration < 0.0f // Region E: Below the bottom plane
					&& inside_bottom_radius) // Inside the bottom radius
				{
					// Bottom plane is closest
					point = bottom_3d;
					normal = Vec3(0, -1, 0);
				}
				else // Region C
				{
					// Side surface is closest
					point = side_support_top;
					normal = side_normal;
				}
			}
			else if (side_penetration < min_top_bottom_penetration)
			{
				// Side surface is closest
				point = side_support_top;
				normal = side_normal;
			}
			else if (top_penetration < bottom_penetration)
			{
				// Top plane is closest
				point = top_3d;
				normal = Vec3(0, 1, 0);
			}
			else
			{
				// Bottom plane is closest
				point = bottom_3d;
				normal = Vec3(0, -1, 0);
			}

			// Calculate penetration
			Plane plane = Plane::sFromPointAndNormal(point, normal);
			float penetration = -plane.SignedDistance(local_pos);
			if (penetration > v->mLargestPenetration)
			{
				v->mLargestPenetration = penetration;

				// Store collision
				v->mCollisionPlane = plane.GetTransformed(inCenterOfMassTransform);
				v->mCollidingShapeIndex = inCollidingShapeIndex;
			}
		}
}

class TaperedCylinderShape::TCSGetTrianglesContext
{
public:
				TCSGetTrianglesContext(Mat44Arg inTransform) : mTransform(inTransform) { }

	Mat44		mTransform;
	uint		mProcessed = 0; // Which elements we processed, bit 0 = top, bit 1 = bottom, bit 2 = side
};

void TaperedCylinderShape::GetTrianglesStart(GetTrianglesContext &ioContext, const AABox &inBox, Vec3Arg inPositionCOM, QuatArg inRotation, Vec3Arg inScale) const
{
	static_assert(sizeof(TCSGetTrianglesContext) <= sizeof(GetTrianglesContext), "GetTrianglesContext too small");
	JPH_ASSERT(IsAligned(&ioContext, alignof(TCSGetTrianglesContext)));

	// Make sure the scale is not inside out
	Vec3 scale = ScaleHelpers::IsInsideOut(inScale)? Vec3(-1, 1, 1) * inScale : inScale;

	// Mark top and bottom processed if their radius is too small
	TCSGetTrianglesContext *context = new (&ioContext) TCSGetTrianglesContext(Mat44::sRotationTranslation(inRotation, inPositionCOM) * Mat44::sScale(scale));
	constexpr float cMinRadius = 1.0e-3f;
	if (mTopRadius < cMinRadius)
		context->mProcessed |= 0b001;
	if (mBottomRadius < cMinRadius)
		context->mProcessed |= 0b010;
}

int TaperedCylinderShape::GetTrianglesNext(GetTrianglesContext &ioContext, int inMaxTrianglesRequested, Float3 *outTriangleVertices, const PhysicsMaterial **outMaterials) const
{
	constexpr int cNumVertices = int(std::size(cTaperedCapsuleFace));

	static_assert(cGetTrianglesMinTrianglesRequested >= 2 * cNumVertices);
	JPH_ASSERT(inMaxTrianglesRequested >= cGetTrianglesMinTrianglesRequested);

	TCSGetTrianglesContext &context = (TCSGetTrianglesContext &)ioContext;

	int total_num_triangles = 0;

	// Top cap
	Vec3 top_3d(0, mTop, 0);
	if ((context.mProcessed & 0b001) == 0)
	{
		Vec3 v0 = context.mTransform * (top_3d + mTopRadius * cTaperedCapsuleFace[0]);
		Vec3 v1 = context.mTransform * (top_3d + mTopRadius * cTaperedCapsuleFace[1]);

		for (const Vec3 *v = cTaperedCapsuleFace + 2, *v_end = cTaperedCapsuleFace + cNumVertices; v < v_end; ++v)
		{
			Vec3 v2 = context.mTransform * (top_3d + mTopRadius * *v);

			v0.StoreFloat3(outTriangleVertices++);
			v1.StoreFloat3(outTriangleVertices++);
			v2.StoreFloat3(outTriangleVertices++);

			v1 = v2;
		}

		total_num_triangles = cNumVertices - 2;
		context.mProcessed |= 0b001;
	}

	// Bottom cap
	Vec3 bottom_3d(0, mBottom, 0);
	if ((context.mProcessed & 0b010) == 0
		&& total_num_triangles + cNumVertices - 2 < inMaxTrianglesRequested)
	{
		Vec3 v0 = context.mTransform * (bottom_3d + mBottomRadius * cTaperedCapsuleFace[0]);
		Vec3 v1 = context.mTransform * (bottom_3d + mBottomRadius * cTaperedCapsuleFace[1]);

		for (const Vec3 *v = cTaperedCapsuleFace + 2, *v_end = cTaperedCapsuleFace + cNumVertices; v < v_end; ++v)
		{
			Vec3 v2 = context.mTransform * (bottom_3d + mBottomRadius * *v);

			v0.StoreFloat3(outTriangleVertices++);
			v2.StoreFloat3(outTriangleVertices++);
			v1.StoreFloat3(outTriangleVertices++);

			v1 = v2;
		}

		total_num_triangles += cNumVertices - 2;
		context.mProcessed |= 0b010;
	}

	// Side
	if ((context.mProcessed & 0b100) == 0
		&& total_num_triangles + 2 * cNumVertices < inMaxTrianglesRequested)
	{
		Vec3 v0t = context.mTransform * (top_3d + mTopRadius * cTaperedCapsuleFace[cNumVertices - 1]);
		Vec3 v0b = context.mTransform * (bottom_3d + mBottomRadius * cTaperedCapsuleFace[cNumVertices - 1]);

		for (const Vec3 *v = cTaperedCapsuleFace, *v_end = cTaperedCapsuleFace + cNumVertices; v < v_end; ++v)
		{
			Vec3 v1t = context.mTransform * (top_3d + mTopRadius * *v);
			v0t.StoreFloat3(outTriangleVertices++);
			v0b.StoreFloat3(outTriangleVertices++);
			v1t.StoreFloat3(outTriangleVertices++);

			Vec3 v1b = context.mTransform * (bottom_3d + mBottomRadius * *v);
			v1t.StoreFloat3(outTriangleVertices++);
			v0b.StoreFloat3(outTriangleVertices++);
			v1b.StoreFloat3(outTriangleVertices++);

			v0t = v1t;
			v0b = v1b;
		}

		total_num_triangles += 2 * cNumVertices;
		context.mProcessed |= 0b100;
	}

	// Store materials
	if (outMaterials != nullptr)
	{
		const PhysicsMaterial *material = GetMaterial();
		for (const PhysicsMaterial **m = outMaterials, **m_end = outMaterials + total_num_triangles; m < m_end; ++m)
			*m = material;
	}

	return total_num_triangles;
}

#ifdef JPH_DEBUG_RENDERER
void TaperedCylinderShape::Draw(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const
{
	// Preserve flip along y axis but make sure we're not inside out
	Vec3 scale = ScaleHelpers::IsInsideOut(inScale)? Vec3(-1, 1, 1) * inScale : inScale;
	RMat44 world_transform = inCenterOfMassTransform * Mat44::sScale(scale);

	DebugRenderer::EDrawMode draw_mode = inDrawWireframe? DebugRenderer::EDrawMode::Wireframe : DebugRenderer::EDrawMode::Solid;
	inRenderer->DrawTaperedCylinder(world_transform, mTop, mBottom, mTopRadius, mBottomRadius, inUseMaterialColors? GetMaterial()->GetDebugColor() : inColor, DebugRenderer::ECastShadow::On, draw_mode);
}
#endif // JPH_DEBUG_RENDERER

void TaperedCylinderShape::SaveBinaryState(StreamOut &inStream) const
{
	ConvexShape::SaveBinaryState(inStream);

	inStream.Write(mTop);
	inStream.Write(mBottom);
	inStream.Write(mTopRadius);
	inStream.Write(mBottomRadius);
	inStream.Write(mConvexRadius);
}

void TaperedCylinderShape::RestoreBinaryState(StreamIn &inStream)
{
	ConvexShape::RestoreBinaryState(inStream);

	inStream.Read(mTop);
	inStream.Read(mBottom);
	inStream.Read(mTopRadius);
	inStream.Read(mBottomRadius);
	inStream.Read(mConvexRadius);
}

float TaperedCylinderShape::GetVolume() const
{
	// Volume of a tapered cylinder is: integrate(%pi*(b+x*(t-b)/h)^2,x,0,h) where t is the top radius, b is the bottom radius and h is the height
	return (JPH_PI / 3.0f) * (mTop - mBottom) * (Square(mTopRadius) + mTopRadius * mBottomRadius + Square(mBottomRadius));
}

bool TaperedCylinderShape::IsValidScale(Vec3Arg inScale) const
{
	return ConvexShape::IsValidScale(inScale) && ScaleHelpers::IsUniformScaleXZ(inScale.Abs());
}

Vec3 TaperedCylinderShape::MakeScaleValid(Vec3Arg inScale) const
{
	Vec3 scale = ScaleHelpers::MakeNonZeroScale(inScale);

	return scale.GetSign() * ScaleHelpers::MakeUniformScaleXZ(scale.Abs());
}

void TaperedCylinderShape::sRegister()
{
	ShapeFunctions &f = ShapeFunctions::sGet(EShapeSubType::TaperedCylinder);
	f.mConstruct = []() -> Shape * { return new TaperedCylinderShape; };
	f.mColor = Color::sGreen;
}

JPH_NAMESPACE_END
