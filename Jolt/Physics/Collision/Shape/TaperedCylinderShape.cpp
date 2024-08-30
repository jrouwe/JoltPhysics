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
	mHalfHeight(inSettings.mHalfHeight),
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

	if (inSettings.mTopRadius < inSettings.mConvexRadius)
	{
		outResult.SetError("Invalid top radius");
		return;
	}

	if (inSettings.mBottomRadius < inSettings.mConvexRadius)
	{
		outResult.SetError("Invalid bottom radius");
		return;
	}

	if (inSettings.mConvexRadius < 0.0f)
	{
		outResult.SetError("Invalid convex radius");
		return;
	}

	outResult.Set(this);
}

class TaperedCylinderShape::TaperedCylinder final : public Support
{
public:
					TaperedCylinder(float inHalfHeight, float inTopRadius, float inBottomRadius, float inConvexRadius) :
		mHalfHeight(inHalfHeight),
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
			Vec3 top_support((mTopRadius * x) / o, mHalfHeight, (mTopRadius * z) / o);
			Vec3 bottom_support((mBottomRadius * x) / o, -mHalfHeight, (mBottomRadius * z) / o);
			return inDirection.Dot(top_support) > inDirection.Dot(bottom_support)? top_support : bottom_support;
		}
		else
			return Vec3(0, Sign(y) * mHalfHeight, 0);
	}

	virtual float	GetConvexRadius() const override
	{
		return mConvexRadius;
	}

private:
	float			mHalfHeight;
	float			mTopRadius;
	float			mBottomRadius;
	float			mConvexRadius;
};

JPH_INLINE void TaperedCylinderShape::GetScaled(Vec3Arg inScale, float &outHalfHeight, float &outTopRadius, float &outBottomRadius, float &outConvexRadius) const
{
	Vec3 abs_scale = inScale.Abs();
	float scale_xz = abs_scale.GetX();

	outHalfHeight = abs_scale.GetY() * mHalfHeight;
	outTopRadius = scale_xz * mTopRadius;
	outBottomRadius = scale_xz * mBottomRadius;
	outConvexRadius = scale_xz * mConvexRadius;

	// Negative Y-scale flips the top and bottom
	if (inScale.GetY() < 0.0f)
		swap(outTopRadius, outBottomRadius);
}

const ConvexShape::Support *TaperedCylinderShape::GetSupportFunction(ESupportMode inMode, SupportBuffer &inBuffer, Vec3Arg inScale) const
{
	JPH_ASSERT(IsValidScale(inScale));

	// Get scaled tapered cylinder
	float half_height, top_radius, bottom_radius, convex_radius;
	GetScaled(inScale, half_height, top_radius, bottom_radius, convex_radius);

	switch (inMode)
	{
	case ESupportMode::IncludeConvexRadius:
	case ESupportMode::Default:
		return new (&inBuffer) TaperedCylinder(half_height, top_radius, bottom_radius, 0.0f);

	case ESupportMode::ExcludeConvexRadius:
		return new (&inBuffer) TaperedCylinder(half_height - convex_radius, top_radius - convex_radius, bottom_radius - convex_radius, convex_radius);
	}

	JPH_ASSERT(false);
	return nullptr;
}

void TaperedCylinderShape::GetSupportingFace(const SubShapeID &inSubShapeID, Vec3Arg inDirection, Vec3Arg inScale, Mat44Arg inCenterOfMassTransform, SupportingFace &outVertices) const
{
	JPH_ASSERT(inSubShapeID.IsEmpty(), "Invalid subshape ID");
	JPH_ASSERT(IsValidScale(inScale));

	// Get scaled tapered cylinder
	float half_height, top_radius, bottom_radius, convex_radius;
	GetScaled(inScale, half_height, top_radius, bottom_radius, convex_radius);

	// Get the normal of the side of the cylinder in the horizontal plane
	Vec3 horizontal_normal = (Vec3(-1, 0, -1) * inDirection).NormalizedOr(Vec3::sAxisX());

	// Get the normal of the side of the cylinder
	float tan_alpha = (bottom_radius - top_radius) / (2.0f * half_height);
	Vec3 normal = Vec3(horizontal_normal.GetX(), tan_alpha, horizontal_normal.GetZ()).Normalized();

	// Check if the normal is closer to the side than to the top or bottom
	Vec3 half_height_3d(0, half_height, 0);
	if (abs(normal.Dot(inDirection)) > abs(inDirection.GetY()))
	{		
		// Return the side of the cylinder
		outVertices.push_back(inCenterOfMassTransform * (horizontal_normal * top_radius + half_height_3d));
		outVertices.push_back(inCenterOfMassTransform * (horizontal_normal * bottom_radius - half_height_3d));
	}
	else if (inDirection.GetY() < 0.0f)
	{
		// Top of the cylinder
		for (Vec3 v : cTaperedCapsuleFace)
			outVertices.push_back(inCenterOfMassTransform * (top_radius * v + half_height_3d));
	}
	else
	{
		// Bottom of the cylinder
		for (int i = (int)std::size(cTaperedCapsuleFace) - 1; i >= 0; --i)
			outVertices.push_back(inCenterOfMassTransform * (bottom_radius * cTaperedCapsuleFace[i] - half_height_3d));
	}
}

MassProperties TaperedCylinderShape::GetMassProperties() const
{
	MassProperties p;
	p.mMass = GetVolume() * GetDensity();

	// TODO this is the inertia of a cylinder, not a tapered cylinder
	float height = 2.0f * mHalfHeight;
	float inertia_y = 0.5f * (mBottomRadius + mTopRadius) * p.mMass * 0.5f;
	float inertia_x = inertia_y * 0.5f + p.mMass * height * height / 12.0f;
	float inertia_z = inertia_x;
	p.mInertia = Mat44::sScale(Vec3(inertia_x, inertia_y, inertia_z));
	return p;
}

Vec3 TaperedCylinderShape::GetSurfaceNormal(const SubShapeID &inSubShapeID, Vec3Arg inLocalSurfacePosition) const
{
	JPH_ASSERT(inSubShapeID.IsEmpty(), "Invalid subshape ID");

	return Vec3::sAxisX();
}

AABox TaperedCylinderShape::GetLocalBounds() const
{
	float max_radius = max(mTopRadius, mBottomRadius);
	return AABox(Vec3(-max_radius, -mHalfHeight, -max_radius), Vec3(max_radius, mHalfHeight, max_radius));
}

void TaperedCylinderShape::CollideSoftBodyVertices(Mat44Arg inCenterOfMassTransform, Vec3Arg inScale, SoftBodyVertex *ioVertices, uint inNumVertices, [[maybe_unused]] float inDeltaTime, [[maybe_unused]] Vec3Arg inDisplacementDueToGravity, int inCollidingShapeIndex) const
{
	JPH_ASSERT(IsValidScale(inScale));

	// TODO implement
}

#ifdef JPH_DEBUG_RENDERER
void TaperedCylinderShape::Draw(DebugRenderer *inRenderer, RMat44Arg inCenterOfMassTransform, Vec3Arg inScale, ColorArg inColor, bool inUseMaterialColors, bool inDrawWireframe) const
{
	if (mGeometry == nullptr)
	{
		SupportBuffer buffer;
		const Support *support = GetSupportFunction(ESupportMode::IncludeConvexRadius, buffer, Vec3::sReplicate(1.0f));
		mGeometry = inRenderer->CreateTriangleGeometryForConvex([support](Vec3Arg inDirection) { return support->GetSupport(inDirection); });
	}

	// Preserve flip along y axis but make sure we're not inside out
	Vec3 scale = ScaleHelpers::IsInsideOut(inScale)? Vec3(-1, 1, 1) * inScale : inScale;
	RMat44 world_transform = inCenterOfMassTransform * Mat44::sScale(scale);

	AABox bounds = Shape::GetWorldSpaceBounds(inCenterOfMassTransform, inScale);

	float lod_scale_sq = Square(max(mTopRadius, mBottomRadius));

	Color color = inUseMaterialColors? GetMaterial()->GetDebugColor() : inColor;

	DebugRenderer::EDrawMode draw_mode = inDrawWireframe? DebugRenderer::EDrawMode::Wireframe : DebugRenderer::EDrawMode::Solid;

	inRenderer->DrawGeometry(world_transform, bounds, lod_scale_sq, color, mGeometry, DebugRenderer::ECullMode::CullBackFace, DebugRenderer::ECastShadow::On, draw_mode);
}
#endif // JPH_DEBUG_RENDERER

void TaperedCylinderShape::SaveBinaryState(StreamOut &inStream) const
{
	ConvexShape::SaveBinaryState(inStream);

	inStream.Write(mHalfHeight);
	inStream.Write(mTopRadius);
	inStream.Write(mBottomRadius);
	inStream.Write(mConvexRadius);
}

void TaperedCylinderShape::RestoreBinaryState(StreamIn &inStream)
{
	ConvexShape::RestoreBinaryState(inStream);

	inStream.Read(mHalfHeight);
	inStream.Read(mTopRadius);
	inStream.Read(mBottomRadius);
	inStream.Read(mConvexRadius);
}

float TaperedCylinderShape::GetVolume() const
{
	// Volume of a tapered cylinder is: integrate(%pi*(r1+x*(r2-r1)/(2*h))^2,x,0,2*h) where r1 is the top radius, r2 is the bottom radius and h is the half height
	return (2.0f * JPH_PI / 3.0f) * mHalfHeight * (Square(mTopRadius) + mTopRadius * mBottomRadius + Square(mBottomRadius));
}

bool TaperedCylinderShape::IsValidScale(Vec3Arg inScale) const
{
	return ConvexShape::IsValidScale(inScale) && ScaleHelpers::IsUniformScale(inScale.Abs());
}

Vec3 TaperedCylinderShape::MakeScaleValid(Vec3Arg inScale) const
{
	Vec3 scale = ScaleHelpers::MakeNonZeroScale(inScale);

	return scale.GetSign() * ScaleHelpers::MakeUniformScale(scale.Abs());
}

void TaperedCylinderShape::sRegister()
{
	ShapeFunctions &f = ShapeFunctions::sGet(EShapeSubType::TaperedCylinder);
	f.mConstruct = []() -> Shape * { return new TaperedCylinderShape; };
	f.mColor = Color::sGreen;
}

JPH_NAMESPACE_END
