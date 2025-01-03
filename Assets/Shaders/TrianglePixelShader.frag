#version 450

layout(binding = 1) uniform PixelShaderConstantBuffer
{
	vec3	CameraPos;
	vec3	LightPos;
} c;

layout(location = 0) in vec3 iNormal;
layout(location = 1) in vec3 iWorldPos;
layout(location = 2) in vec2 iTex;
layout(location = 3) in vec4 iPositionL;
layout(location = 4) in vec4 iColor;

layout(location = 0) out vec4 oColor;

layout(set = 1, binding = 0) uniform sampler2D LightDepthSampler;

void main()
{
	// Constants
	float AmbientFactor = 0.3;
	vec3 DiffuseColor = vec3(iColor.r, iColor.g, iColor.b);
	vec3 SpecularColor = vec3(1, 1, 1);
	float SpecularPower = 100.0;
	float bias = 1.0e-7;

	// Homogenize position in light space
	vec3 position_l = iPositionL.xyz / iPositionL.w;

	// Calculate dot product between direction to light and surface normal and clamp between [0, 1]
	vec3 view_dir = normalize(c.CameraPos - iWorldPos);
	vec3 world_to_light = c.LightPos - iWorldPos;
	vec3 light_dir = normalize(world_to_light);
	vec3 normal = normalize(iNormal);
	if (dot(view_dir, normal) < 0) // If we're viewing the triangle from the back side, flip the normal to get the correct lighting
		normal = -normal;
	float normal_dot_light_dir = clamp(dot(normal, light_dir), 0, 1);

	// Calculate texture coordinates in light depth texture
	vec2 tex_coord;
	tex_coord.x = position_l.x / 2.0 + 0.5;
	tex_coord.y = position_l.y / 2.0 + 0.5;

	// Check that the texture coordinate is inside the depth texture, if not we don't know if it is lit or not so we assume lit
	float shadow_factor = 1.0;
	if (iColor.a > 0 // Alpha = 0 means don't receive shadows
		&& tex_coord.x == clamp(tex_coord.x, 0, 1) && tex_coord.y == clamp(tex_coord.y, 0, 1))
	{
		// Modify shadow bias according to the angle between the normal and the light dir
		float modified_bias = bias * tan(acos(normal_dot_light_dir));
		modified_bias = min(modified_bias, 10.0 * bias);
		
		// Get texture size
		float width = 1.0 / 4096;
		float height = 1.0 / 4096;

		// Samples to take
		uint num_samples = 16;
		vec2 offsets[] = { 
			vec2(-1.5 * width, -1.5 * height),
			vec2(-0.5 * width, -1.5 * height),
			vec2(0.5 * width, -1.5 * height),
			vec2(1.5 * width, -1.5 * height),

			vec2(-1.5 * width, -0.5 * height),
			vec2(-0.5 * width, -0.5 * height),
			vec2(0.5 * width, -0.5 * height),
			vec2(1.5 * width, -0.5 * height),

			vec2(-1.5 * width, 0.5 * height),
			vec2(-0.5 * width, 0.5 * height),
			vec2(0.5 * width, 0.5 * height),
			vec2(1.5 * width, 0.5 * height),

			vec2(-1.5 * width, 1.5 * height),
			vec2(-0.5 * width, 1.5 * height),
			vec2(0.5 * width, 1.5 * height),
			vec2(1.5 * width, 1.5 * height),
		};

		// Calculate depth of this pixel relative to the light
		float light_depth = position_l.z + modified_bias;

		// Sample shadow factor
		shadow_factor = 0.0;
		for (uint i = 0; i < num_samples; ++i)
			shadow_factor += texture(LightDepthSampler, tex_coord + offsets[i]).x <= light_depth? 1.0 : 0.0;
		shadow_factor /= num_samples;
	}

	// Calculate diffuse and specular
	float diffuse = normal_dot_light_dir;
	float specular = diffuse > 0.0? pow(clamp(-dot(reflect(light_dir, normal), view_dir), 0, 1), SpecularPower) : 0.0;

	// Apply procedural pattern based on the uv coordinates
	bvec2 less_half = lessThan(iTex - floor(iTex), vec2(0.5, 0.5));
	float darken_factor = less_half.r ^^ less_half.g? 0.5 : 1.0;

	// Fade out checkerboard pattern when it tiles too often
	vec2 dx = dFdx(iTex), dy = dFdy(iTex);
	float texel_distance = sqrt(dot(dx, dx) + dot(dy, dy));
	darken_factor = mix(darken_factor, 0.75, clamp(5.0 * texel_distance - 1.5, 0.0, 1.0));

	// Calculate color
	oColor = vec4(clamp((AmbientFactor + diffuse * shadow_factor) * darken_factor * DiffuseColor + SpecularColor * specular * shadow_factor, 0, 1), 1);
}