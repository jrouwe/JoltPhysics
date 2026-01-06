// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Core/HashCombine.h>
#include <Jolt/Compute/CPU/ComputeSystemCPU.h>
#include <Jolt/Compute/CPU/ShaderWrapper.h>
#include <Jolt/Compute/CPU/HLSLToCPP.h>

/// @cond INTERNAL

JPH_NAMESPACE_BEGIN
JPH_MSVC_SUPPRESS_WARNING(5031) // #pragma warning(pop): likely mismatch, popping warning state pushed in different file

#define JPH_SHADER_OVERRIDE_MACROS
#define JPH_SHADER_GENERATE_WRAPPER

#define JPH_SHADER_CONSTANT(type, name, value)	inline static constexpr type name = value;

#define JPH_SHADER_CONSTANTS_BEGIN(type, name)	struct type { alignas(16) int dummy; } name; // Ensure that the first constant is 16 byte aligned
#define JPH_SHADER_CONSTANTS_MEMBER(type, name)	type c##name;
#define JPH_SHADER_CONSTANTS_END(type)

#define JPH_SHADER_BUFFER(type)					const type *
#define JPH_SHADER_RW_BUFFER(type)				type *

#define JPH_SHADER_BIND_BEGIN(name)
#define JPH_SHADER_BIND_END(name)
#define JPH_SHADER_BIND_BUFFER(type, name)		const type *name = nullptr;
#define JPH_SHADER_BIND_RW_BUFFER(type, name)	type *name = nullptr;

#define JPH_SHADER_FUNCTION_BEGIN(return_type, name, group_size_x, group_size_y, group_size_z) \
		virtual void Main(
#define JPH_SHADER_PARAM_THREAD_ID(name)		const HLSLToCPP::uint3 &name
#define JPH_SHADER_FUNCTION_END					) override

#define JPH_SHADER_STRUCT_BEGIN(name)			struct name {
#define JPH_SHADER_STRUCT_MEMBER(type, name)	type m##name;
#define JPH_SHADER_STRUCT_END(name)				};

#define JPH_TO_STRING(name)						JPH_TO_STRING2(name)
#define JPH_TO_STRING2(name)					#name

#define JPH_SHADER_CLASS_NAME(name)				JPH_SHADER_CLASS_NAME2(name)
#define JPH_SHADER_CLASS_NAME2(name)			name##ShaderWrapper

#define JPH_IN(type)							const type &
#define JPH_OUT(type)							type &
#define JPH_IN_OUT(type)						type &

// Namespace to prevent 'using' from leaking out
namespace ShaderWrappers {

using namespace HLSLToCPP;

class JPH_SHADER_CLASS_NAME(JPH_SHADER_NAME) : public ShaderWrapper
{
public:
	// Define types
	using JPH_float = float;
	using JPH_float3 = HLSLToCPP::float3;
	using JPH_float4 = HLSLToCPP::float4;
	using JPH_uint = uint;
	using JPH_uint3 = HLSLToCPP::uint3;
	using JPH_uint4 = HLSLToCPP::uint4;
	using JPH_int = int;
	using JPH_int3 = HLSLToCPP::int3;
	using JPH_int4 = HLSLToCPP::int4;
	using JPH_Quat = HLSLToCPP::Quat;
	using JPH_Plane = HLSLToCPP::Plane;
	using JPH_Mat44 = HLSLToCPP::Mat44;

	// Now the shader code should be included followed by WrapShaderBindings.h

/// @endcond
