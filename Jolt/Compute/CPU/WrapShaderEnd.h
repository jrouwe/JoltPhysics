// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

/// @cond INTERNAL

		// WrapShaderBindings.h should have been included followed by the shader bindings

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

} // ShaderWrappers

/// @endcond

// Stop clang from complaining that the register function is missing a prototype
JPH_SHADER_WRAPPER_FUNCTION(, JPH_SHADER_NAME);

/// Register this wrapper
JPH_SHADER_WRAPPER_FUNCTION(inComputeSystem, JPH_SHADER_NAME)
{
	inComputeSystem->RegisterShader(JPH_TO_STRING(JPH_SHADER_NAME), ShaderWrappers::JPH_SHADER_CLASS_NAME(JPH_SHADER_NAME)::sCreate);
}

#undef JPH_SHADER_OVERRIDE_MACROS
#undef JPH_SHADER_GENERATE_WRAPPER
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
#undef JPH_OUT
#undef JPH_IN_OUT
#undef JPH_SHADER_NAME

JPH_NAMESPACE_END
