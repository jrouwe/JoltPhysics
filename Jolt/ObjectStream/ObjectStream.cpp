// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <ObjectStream/ObjectStream.h>

namespace JPH {

// Define macro to declare functions for a specific primitive type
#define JPH_DECLARE_PRIMITIVE(name)														\
	bool				OSIsType(name *inNull, int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName) \
	{																					\
		return inArrayDepth == 0 && inDataType == ObjectStream::EDataType::T_##name;	\
	}

// This file uses the JPH_DECLARE_PRIMITIVE macro to define all types
#include <ObjectStream/ObjectStreamTypes.h>

} // JPH