// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Core/HashCombine.h>
#include <Jolt/Compute/CPU/ComputeSystemCPU.h>
#include <Jolt/Compute/CPU/ShaderWrapper.h>
#include <Jolt/Shaders/HLSLToCPP.h>

JPH_NAMESPACE_BEGIN

#define JPH_SHADER_OVERRIDE_MACROS

using namespace HLSLToCPP;

#define JPH_SHADER_CONSTANT(type, name, value)	inline static constexpr type name = value;

#define JPH_SHADER_CONSTANTS_BEGIN(type, name)	struct type { alignas(16) int dummy; } name;
#define JPH_SHADER_CONSTANTS_MEMBER(type, name)	type c##name;
#define JPH_SHADER_CONSTANTS_END

#define JPH_SHADER_BUFFER(type)					const type *
#define JPH_SHADER_RW_BUFFER(type)				type *

#define JPH_SHADER_BIND_BEGIN(name)
#define JPH_SHADER_BIND_END
#define JPH_SHADER_BIND_BUFFER(type, name)		const type *name = nullptr;
#define JPH_SHADER_BIND_RW_BUFFER(type, name)	type *name = nullptr;

#define JPH_SHADER_FUNCTION_BEGIN(return_type, name, group_size_x, group_size_y, group_size_z) \
		virtual void Main(
#define JPH_SHADER_PARAM_THREAD_ID(name)		const HLSLToCPP::uint3 &name
#define JPH_SHADER_FUNCTION_END					) override

#define JPH_SHADER_STRUCT_BEGIN(name)			struct name {
#define JPH_SHADER_STRUCT_MEMBER(type, name)	type m##name;
#define JPH_SHADER_STRUCT_END					};

#define JPH_TO_STRING(name)						JPH_TO_STRING2(name)
#define JPH_TO_STRING2(name)					#name

#define JPH_SHADER_CLASS_NAME(name)				JPH_SHADER_CLASS_NAME2(name)
#define JPH_SHADER_CLASS_NAME2(name)			name##ShaderWrapper

#define JPH_SHADER_HEADER_NAME(name)			JPH_TO_STRING(name.hlsl)

#define JPH_BINDINGS_HEADER_NAME(name)			JPH_BINDINGS_HEADER_NAME2(name)
#define JPH_BINDINGS_HEADER_NAME2(name)			JPH_TO_STRING(name##Bindings.h)

#define JPH_IN(type)							const type &
#define JPH_OUT(type)							type &
#define JPH_IN_OUT(type)						type &

/// @cond INTERNAL
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

	// Include the actual shader
	#include JPH_SHADER_HEADER_NAME(JPH_SHADER_NAME)

	/// Bind a buffer to the shader
	virtual void			Bind(const char *inName, void *inData, uint64 inSize) override
	{
		// Don't redefine constants
		#undef JPH_SHADER_CONSTANT
		#define JPH_SHADER_CONSTANT(type, name, value)

		// Don't redefine structs
		#undef JPH_SHADER_STRUCT_BEGIN
		#undef JPH_SHADER_STRUCT_MEMBER
		#undef JPH_SHADER_STRUCT_END
		#define JPH_SHADER_STRUCT_BEGIN(name)
		#define JPH_SHADER_STRUCT_MEMBER(type, name)
		#define JPH_SHADER_STRUCT_END

		// When a constant buffer is bound, copy the data into the members
		#undef JPH_SHADER_CONSTANTS_BEGIN
		#undef JPH_SHADER_CONSTANTS_MEMBER
		#define JPH_SHADER_CONSTANTS_BEGIN(type, name)	case HashString(#name): memcpy(&name + 1, inData, size_t(inSize));	break;
		#define JPH_SHADER_CONSTANTS_MEMBER(type, name)

		// When a buffer is bound, set the pointer
		#undef JPH_SHADER_BIND_BUFFER
		#undef JPH_SHADER_BIND_RW_BUFFER
		#define JPH_SHADER_BIND_BUFFER(type, name)		case HashString(#name): name = (const type *)inData;		break;
		#define JPH_SHADER_BIND_RW_BUFFER(type, name)	case HashString(#name): name = (type *)inData;				break;

		switch (HashString(inName))
		{
		// Include the bindings header only
		#include JPH_BINDINGS_HEADER_NAME(JPH_SHADER_NAME)

		default:
			JPH_ASSERT(false, "Buffer cannot be bound to this shader");
			break;
		}
	}

	/// Factory function to create a shader wrapper for this shader
	static ShaderWrapper *	sCreate()
	{
		return new JPH_SHADER_CLASS_NAME(JPH_SHADER_NAME)();
	}
};
/// @endcond

// Stop clang from complaining that the register function is missing a prototype
JPH_SHADER_WRAPPER_FUNCTION(, JPH_SHADER_NAME);

/// Register this wrapper
JPH_SHADER_WRAPPER_FUNCTION(inComputeSystem, JPH_SHADER_NAME)
{
	inComputeSystem->RegisterShader(JPH_TO_STRING(JPH_SHADER_NAME), JPH_SHADER_CLASS_NAME(JPH_SHADER_NAME)::sCreate);
}

#undef JPH_SHADER_CONSTANT
#undef JPH_SHADER_CONSTANTS_BEGIN
#undef JPH_SHADER_CONSTANTS_MEMBER
#undef JPH_SHADER_CONSTANTS_END
#undef JPH_SHADER_BUFFER
#undef JPH_SHADER_RW_BUFFER
#undef JPH_SHADER_BIND_BEGIN
#undef JPH_SHADER_BIND_END
#undef JPH_SHADER_BIND_BUFFER
#undef JPH_SHADER_BIND_RW_BUFFER
#undef JPH_SHADER_FUNCTION_BEGIN
#undef JPH_SHADER_PARAM_THREAD_ID
#undef JPH_SHADER_FUNCTION_END
#undef JPH_SHADER_STRUCT_BEGIN
#undef JPH_SHADER_STRUCT_MEMBER
#undef JPH_SHADER_STRUCT_END
#undef JPH_TO_STRING
#undef JPH_TO_STRING2
#undef JPH_SHADER_CLASS_NAME
#undef JPH_SHADER_CLASS_NAME2
#undef JPH_SHADER_HEADER_NAME
#undef JPH_BINDINGS_HEADER_NAME
#undef JPH_BINDINGS_HEADER_NAME2
#undef JPH_OUT
#undef JPH_IN_OUT

JPH_NAMESPACE_END
