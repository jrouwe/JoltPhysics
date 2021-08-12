// Shader that uses a shadow map for rendering shadows, see:
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/
// https://takinginitiative.wordpress.com/2011/05/25/directx10-tutorial-10-shadow-mapping-part-2/

Texture2D LightDepthTexture : register(t2);
SamplerComparisonState LightDepthSampler : register(s2);

cbuffer PixelShaderConstantBuffer : register(b1)
{
	float3	CameraPos;
	float3	LightPos;
};

struct PS_INPUT
{
    float4 Position  : SV_POSITION; // interpolated vertex position
    float3 Normal	 : TEXCOORD0;
	float3 WorldPos  : TEXCOORD1;
	float2 Tex		 : TEXCOORD2;
	float4 PositionL : TEXCOORD3; // interpolated vertex position in light space
	float4 Color	 : COLOR0;
};

struct PS_OUTPUT
{
    float4 RGBColor : SV_TARGET;
};

PS_OUTPUT main(PS_INPUT input)
{
	// Constants
	float AmbientFactor = 0.3;
	float3 DiffuseColor = float3(input.Color.r, input.Color.g, input.Color.b);
	float3 SpecularColor = float3(1, 1, 1);
	float SpecularPower = 100.0;
	float bias = 1.0e-7;

	// Homogenize position in light space
	input.PositionL.xyz /= input.PositionL.w;

	// Calculate dot product between direction to light and surface normal and clamp between [0, 1]
	float3 view_dir = normalize(CameraPos - input.WorldPos);
	float3 world_to_light = LightPos - input.WorldPos;
	float3 light_dir = normalize(world_to_light);
	float3 normal = normalize(input.Normal);
	if (dot(view_dir, normal) < 0) // If we're viewing the triangle from the back side, flip the normal to get the correct lighting
		normal = -normal;
	float normal_dot_light_dir = saturate(dot(normal, light_dir));

	// Calculate texture coordinates in light depth texture
	float2 tex_coord;
	tex_coord.x =  input.PositionL.x / 2.0 + 0.5;
	tex_coord.y = -input.PositionL.y / 2.0 + 0.5;

	// Check that the texture coordinate is inside the depth texture, if not we don't know if it is lit or not so we assume lit
	float shadow_factor = 1.0;
	if (input.Color.a > 0 // Alpha = 0 means don't receive shadows
		&& tex_coord.x == saturate(tex_coord.x) && tex_coord.y == saturate(tex_coord.y))
	{
		// Modify shadow bias according to the angle between the normal and the light dir
		float modified_bias = bias * tan(acos(normal_dot_light_dir));
		modified_bias = min(modified_bias, 10.0 * bias);
		
		// Get texture size
		float width, height, levels; 
		LightDepthTexture.GetDimensions(0, width, height, levels); 
		width = 1.0 / width;
		height = 1.0 / height;

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
		float light_depth = input.PositionL.z + modified_bias;

		// Sample shadow factor
		shadow_factor = 0.0;
		[unroll] for (uint i = 0; i < num_samples; ++i)
			shadow_factor += LightDepthTexture.SampleCmp(LightDepthSampler, tex_coord + offsets[i], light_depth);
		shadow_factor /= num_samples;
	}

	// Calculate diffuse and specular
	float diffuse = normal_dot_light_dir;
	float specular = diffuse > 0.0? pow(saturate(-dot(reflect(light_dir, normal), view_dir)), SpecularPower) : 0.0;

	// Apply procedural pattern based on the uv coordinates
	bool2 less_half = input.Tex - floor(input.Tex) < float2(0.5, 0.5);
	float darken_factor = less_half.r ^ less_half.g? 0.5 : 1.0;

	// Fade out checkerboard pattern when it tiles too often
	float2 dx = ddx(input.Tex), dy = ddy(input.Tex);
	float texel_distance = sqrt(dot(dx, dx) + dot(dy, dy));
	darken_factor = lerp(darken_factor, 0.75, clamp(5.0 * texel_distance - 1.5, 0.0, 1.0));

	// Calculate color
    PS_OUTPUT output;
	output.RGBColor = float4(saturate((AmbientFactor + diffuse * shadow_factor) * darken_factor * DiffuseColor + SpecularColor * specular * shadow_factor), 1);
    return output;
}
