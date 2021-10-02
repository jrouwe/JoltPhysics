// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Core/StaticArray.h>
#include <Core/Reference.h>

namespace JPH {

/// Base class for object stream input and output streams.
class ObjectStream
{
public:
	/// Stream type
	enum class EStreamType
	{
		Text,
		Binary,
	};

	/// Data type
	enum class EDataType
	{
		/// Control codes
		Declare,																		///< Used to declare the attributes of a new object type
		Object,																			///< Start of a new object
		Instance,																		///< Used in attribute declaration, indicates that an object is an instanced attribute (no pointer)
		Pointer,																		///< Used in attribute declaration, indicates that an object is a pointer attribute
		Array,																			///< Used in attribute declaration, indicates that this is an array of objects
		
		// Basic types (primitives)
		#define JPH_DECLARE_PRIMITIVE(name)	T_##name,

		// This file uses the JPH_DECLARE_PRIMITIVE macro to define all types
		#include <ObjectStream/ObjectStreamTypes.h>

		// Error values for read functions
		Invalid,																		///< Next token on the stream was not a valid data type
	};

protected:
	/// Constructor
	virtual							~ObjectStream() = default;

	/// Identifier for objects
	using Identifier = uint32;

	static constexpr int			sVersion = 1;
	static constexpr int			sRevision = 0;
	static constexpr Identifier		sNullIdentifier = 0;
};

// Define macro to declare functions for a specific primitive type
#define JPH_DECLARE_PRIMITIVE(name)													\
	bool						OSIsType(name *inNull, int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName);

// This file uses the JPH_DECLARE_PRIMITIVE macro to define all types
#include <ObjectStream/ObjectStreamTypes.h>

// Define serialization templates
template <class T>
bool OSIsType(vector<T> *inArray, int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName)	
{ 
	return (inArrayDepth > 0 && OSIsType((T *)nullptr, inArrayDepth - 1, inDataType, inClassName)); 
}

template <class T, uint N>
bool OSIsType(StaticArray<T, N> *inArray, int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName)	
{ 
	return (inArrayDepth > 0 && OSIsType((T *)nullptr, inArrayDepth - 1, inDataType, inClassName)); 
}

template <class T, uint N>
bool OSIsType(T (*inArray)[N], int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName)	
{ 
	return (inArrayDepth > 0 && OSIsType((T *)nullptr, inArrayDepth - 1, inDataType, inClassName)); 
}

template <class T>
bool OSIsType(Ref<T> *inNull, int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName)
{
	return OSIsType((T *)nullptr, inArrayDepth, inDataType, inClassName);
}

template <class T>
bool OSIsType(RefConst<T> *inNull, int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName)
{
	return OSIsType((T *)nullptr, inArrayDepth, inDataType, inClassName);
}

} // JPH