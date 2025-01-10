#include <metal_stdlib>

using namespace metal;

#include "VertexConstants.h"

constexpr sampler depthSampler(mag_filter::nearest, min_filter::nearest);

struct Vertex
{
	float3		vPos [[attribute(0)]];
	float3		vNorm [[attribute(1)]];
	float2		vTex [[attribute(2)]];
	uchar4		vCol [[attribute(3)]];
	float4		iModel0 [[attribute(4)]];
	float4		iModel1 [[attribute(5)]];
	float4		iModel2 [[attribute(6)]];
	float4		iModel3 [[attribute(7)]];
	float4		iModelInvTrans0 [[attribute(8)]];
	float4		iModelInvTrans1 [[attribute(9)]];
	float4		iModelInvTrans2 [[attribute(10)]];
	float4		iModelInvTrans3 [[attribute(11)]];
	uchar4		iCol [[attribute(12)]];
};

struct TriangleOut
{
	float4		oPosition [[position]];
	float3		oNormal;
	float3		oWorldPos;
	float2		oTex;
	float4		oPositionL;
	float4		oColor;
};

vertex TriangleOut TriangleVertexShader(Vertex vert [[stage_in]], constant VertexShaderConstantBuffer *constants [[buffer(2)]])
{
	TriangleOut out;

	// Convert input matrices
	float4x4 iModel(vert.iModel0, vert.iModel1, vert.iModel2, vert.iModel3);
	float4x4 iModelInvTrans(vert.iModelInvTrans0, vert.iModelInvTrans1, vert.iModelInvTrans2, vert.iModelInvTrans3);

	// Get world position
	float4 pos = float4(vert.vPos, 1.0f);
	float4 world_pos = iModel * pos;

	// Transform the position from world space to homogeneous projection space
	float4 proj_pos = constants->View * world_pos;
	proj_pos = constants->Projection * proj_pos;
	out.oPosition = proj_pos;

	// Transform the position from world space to projection space of the light
	float4 proj_lpos = constants->LightView * world_pos;
	proj_lpos = constants->LightProjection * proj_lpos;
	out.oPositionL = proj_lpos;

	// output normal
	float4 norm = float4(vert.vNorm, 0.0f);
	out.oNormal = normalize(iModelInvTrans * norm).xyz;

	// output world position of the vertex
	out.oWorldPos = world_pos.xyz;

	// output texture coordinates
	out.oTex = vert.vTex;

	// output color
	out.oColor = float4(vert.vCol) * float4(vert.iCol) / (255.0 * 255.0);

	return out;
}

fragment float4 TrianglePixelShader(TriangleOut vert [[stage_in]], constant PixelShaderConstantBuffer *constants, texture2d<float> depthTexture [[texture(0)]])
{
	// Constants
	float AmbientFactor = 0.3;
	float3 DiffuseColor = float3(vert.oColor.r, vert.oColor.g, vert.oColor.b);
	float3 SpecularColor = float3(1, 1, 1);
	float SpecularPower = 100.0;
	float bias = 1.0e-7;

	// Homogenize position in light space
	float3 position_l = vert.oPositionL.xyz / vert.oPositionL.w;

	// Calculate dot product between direction to light and surface normal and clamp between [0, 1]
	float3 view_dir = normalize(constants->CameraPos - vert.oWorldPos);
	float3 world_to_light = constants->LightPos - vert.oWorldPos;
	float3 light_dir = normalize(world_to_light);
	float3 normal = normalize(vert.oNormal);
	if (dot(view_dir, normal) < 0) // If we're viewing the triangle from the back side, flip the normal to get the correct lighting
		normal = -normal;
	float normal_dot_light_dir = clamp(dot(normal, light_dir), 0.0, 1.0);

	// Calculate texture coordinates in light depth texture
	float2 tex_coord;
	tex_coord.x = position_l.x / 2.0 + 0.5;
	tex_coord.y = -position_l.y / 2.0 + 0.5;

	// Check that the texture coordinate is inside the depth texture, if not we don't know if it is lit or not so we assume lit
	float shadow_factor = 1.0;
	if (vert.oColor.a > 0 // Alpha = 0 means don't receive shadows
		&& tex_coord.x == clamp(tex_coord.x, 0.0, 1.0) && tex_coord.y == clamp(tex_coord.y, 0.0, 1.0))
	{
		// Modify shadow bias according to the angle between the normal and the light dir
		float modified_bias = bias * tan(acos(normal_dot_light_dir));
		modified_bias = min(modified_bias, 10.0 * bias);
		
		// Get texture size
		float width = 1.0 / 4096;
		float height = 1.0 / 4096;

		// Samples to take
		uint num_samples = 16;
		float2 offsets[] = { 
			float2(-1.5 * width, -1.5 * height),
			float2(-0.5 * width, -1.5 * height),
			float2(0.5 * width, -1.5 * height),
			float2(1.5 * width, -1.5 * height),

			float2(-1.5 * width, -0.5 * height),
			float2(-0.5 * width, -0.5 * height),
			float2(0.5 * width, -0.5 * height),
			float2(1.5 * width, -0.5 * height),

			float2(-1.5 * width, 0.5 * height),
			float2(-0.5 * width, 0.5 * height),
			float2(0.5 * width, 0.5 * height),
			float2(1.5 * width, 0.5 * height),

			float2(-1.5 * width, 1.5 * height),
			float2(-0.5 * width, 1.5 * height),
			float2(0.5 * width, 1.5 * height),
			float2(1.5 * width, 1.5 * height),
		};

		// Calculate depth of this pixel relative to the light
		float light_depth = position_l.z + modified_bias;

		// Sample shadow factor
		shadow_factor = 0.0;
		for (uint i = 0; i < num_samples; ++i)
			shadow_factor += depthTexture.sample(depthSampler, tex_coord + offsets[i]).x <= light_depth? 1.0 : 0.0;
		shadow_factor /= num_samples;
	}

	// Calculate diffuse and specular
	float diffuse = normal_dot_light_dir;
	float specular = diffuse > 0.0? pow(clamp(-dot(reflect(light_dir, normal), view_dir), 0.0, 1.0), SpecularPower) : 0.0;

	// Apply procedural pattern based on the uv coordinates
	bool2 less_half = (vert.oTex - floor(vert.oTex)) < float2(0.5, 0.5);
	float darken_factor = less_half.r ^ less_half.g? 0.5 : 1.0;

	// Fade out checkerboard pattern when it tiles too often
	float2 dx = dfdx(vert.oTex), dy = dfdy(vert.oTex);
	float texel_distance = sqrt(dot(dx, dx) + dot(dy, dy));
	darken_factor = mix(darken_factor, 0.75, clamp(5.0 * texel_distance - 1.5, 0.0, 1.0));

	// Calculate color
	return float4(clamp((AmbientFactor + diffuse * shadow_factor) * darken_factor * DiffuseColor + SpecularColor * specular * shadow_factor, 0, 1), 1);
}

struct DepthOut
{
	float4		oPosition [[position]];
};

vertex DepthOut TriangleDepthVertexShader(Vertex vert [[stage_in]], constant VertexShaderConstantBuffer *constants [[buffer(2)]])
{
	DepthOut out;

	// Check if the alpha = 0
	if (vert.vCol.a * vert.iCol.a == 0.0)
	{
		// Don't draw the triangle by moving it to an invalid location
		out.oPosition = float4(0, 0, 0, 0);
	}
	else
	{
		// Convert input matrix
		float4x4 iModel(vert.iModel0, vert.iModel1, vert.iModel2, vert.iModel3);

		// Transform the position from world space to homogeneous projection space for the light
		float4 pos = float4(vert.vPos, 1.0f);
		pos = iModel * pos;
		pos = constants->LightView * pos;
		pos = constants->LightProjection * pos;
		out.oPosition = pos;
	}

	return out;
}

fragment float4 TriangleDepthPixelShader(DepthOut in [[stage_in]])
{
	// We only write depth, so this shader does nothing
	return float4(0.0, 0.0, 0.0, 1.0);
}
