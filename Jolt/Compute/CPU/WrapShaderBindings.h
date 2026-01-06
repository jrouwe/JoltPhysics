// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

/// @cond INTERNAL

	// First WrapShaderBegin.h should have been included, then the shader code

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
		#define JPH_SHADER_STRUCT_END(name)

		// When a constant buffer is bound, copy the data into the members
		#undef JPH_SHADER_CONSTANTS_BEGIN
		#undef JPH_SHADER_CONSTANTS_MEMBER
		#define JPH_SHADER_CONSTANTS_BEGIN(type, name)	case HashString(#name): memcpy(&name + 1, inData, size_t(inSize));	break; // Very hacky way to get the address of the first constant and to copy the entire block of constants
		#define JPH_SHADER_CONSTANTS_MEMBER(type, name)

		// When a buffer is bound, set the pointer
		#undef JPH_SHADER_BIND_BUFFER
		#undef JPH_SHADER_BIND_RW_BUFFER
		#define JPH_SHADER_BIND_BUFFER(type, name)		case HashString(#name): name = (const type *)inData;		break;
		#define JPH_SHADER_BIND_RW_BUFFER(type, name)	case HashString(#name): name = (type *)inData;				break;

		switch (HashString(inName))
		{
			// Now include the shader bindings followed by WrapShaderEnd.h

/// @endcond
