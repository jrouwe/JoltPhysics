// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2025 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#ifndef JPH_SHADER_OVERRIDE_MACROS

#ifdef __cplusplus
	JPH_SUPPRESS_WARNING_PUSH
	JPH_SUPPRESS_WARNINGS

	using JPH_float = float;
	using JPH_float3 = JPH::Float3;
	using JPH_float4 = JPH::Float4;
	using JPH_uint = JPH::uint32;
	using JPH_uint3 = JPH::uint32[3];
	using JPH_uint4 = JPH::uint32[4];
	using JPH_int = int;
	using JPH_int3 = int[3];
	using JPH_int4 = int[4];
	using JPH_Quat = JPH::Float4;
	using JPH_Plane = JPH::Float4;
	using JPH_Mat44 = JPH::Float4[4]; // matrix, column major

	#define JPH_SHADER_CONSTANTS_BEGIN(type, name)	struct type {
	#define JPH_SHADER_CONSTANTS_MEMBER(type, name)	type c##name;
	#define JPH_SHADER_CONSTANTS_END				};

	#define JPH_SHADER_BIND_BEGIN(name)
	#define JPH_SHADER_BIND_END
	#define JPH_SHADER_BIND_BUFFER(type, name)
	#define JPH_SHADER_BIND_RW_BUFFER(type, name)

	JPH_SUPPRESS_WARNING_POP
#else
	#pragma pack_matrix(column_major)

	typedef float JPH_float;
	typedef float3 JPH_float3;
	typedef float4 JPH_float4;
	typedef uint JPH_uint;
	typedef uint3 JPH_uint3;
	typedef uint4 JPH_uint4;
	typedef int JPH_int;
	typedef int3 JPH_int3;
	typedef int4 JPH_int4;
	typedef float4 JPH_Quat; // xyz = imaginary part, w = real part
	typedef float4 JPH_Plane; // xyz = normal, w = constant
	typedef float4x4 JPH_Mat44; // matrix, column major

	#define JPH_SHADER_CONSTANTS_BEGIN(type, name)	cbuffer name {
	#define JPH_SHADER_CONSTANTS_MEMBER(type, name)	type c##name;
	#define JPH_SHADER_CONSTANTS_END				};

	#define JPH_SHADER_FUNCTION_BEGIN(return_type, name, group_size_x, group_size_y, group_size_z) \
		[numthreads(group_size_x, group_size_y, group_size_z)] \
		return_type name(
	#define JPH_SHADER_PARAM_THREAD_ID(name)		uint3 name : SV_DispatchThreadID
	#define JPH_SHADER_FUNCTION_END					)

	#define JPH_SHADER_BUFFER(type)					StructuredBuffer<type>
	#define JPH_SHADER_RW_BUFFER(type)				RWStructuredBuffer<type>

	#define JPH_SHADER_BIND_BEGIN(name)
	#define JPH_SHADER_BIND_END
	#define JPH_SHADER_BIND_BUFFER(type, name)		JPH_SHADER_BUFFER(type) name;
	#define JPH_SHADER_BIND_RW_BUFFER(type, name)	JPH_SHADER_RW_BUFFER(type) name;

	#define JPH_AtomicAdd							InterlockedAdd
#endif

#define JPH_SHADER_STRUCT_BEGIN(name)				struct name {
#define JPH_SHADER_STRUCT_MEMBER(type, name)		type m##name;
#define JPH_SHADER_STRUCT_END						};

#endif // JPH_OVERRIDE_SHADER_MACROS
