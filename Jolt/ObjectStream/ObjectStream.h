// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/StaticArray.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/ObjectStream/SerializableAttribute.h>

JPH_NAMESPACE_BEGIN

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
	bool						OSIsType(name *, int inArrayDepth, EOSDataType inDataType, const char *inClassName);

// This file uses the JPH_DECLARE_PRIMITIVE macro to define all types
#include <Jolt/ObjectStream/ObjectStreamTypes.h>

// Define serialization templates
template <class T>
bool OSIsType(vector<T> *, int inArrayDepth, EOSDataType inDataType, const char *inClassName)	
{ 
	return (inArrayDepth > 0 && OSIsType((T *)nullptr, inArrayDepth - 1, inDataType, inClassName)); 
}

template <class T, uint N>
bool OSIsType(StaticArray<T, N> *, int inArrayDepth, EOSDataType inDataType, const char *inClassName)	
{ 
	return (inArrayDepth > 0 && OSIsType((T *)nullptr, inArrayDepth - 1, inDataType, inClassName)); 
}

template <class T, uint N>
bool OSIsType(T (*)[N], int inArrayDepth, EOSDataType inDataType, const char *inClassName)	
{ 
	return (inArrayDepth > 0 && OSIsType((T *)nullptr, inArrayDepth - 1, inDataType, inClassName)); 
}

template <class T>
bool OSIsType(Ref<T> *, int inArrayDepth, EOSDataType inDataType, const char *inClassName)
{
	return OSIsType((T *)nullptr, inArrayDepth, inDataType, inClassName);
}

template <class T>
bool OSIsType(RefConst<T> *, int inArrayDepth, EOSDataType inDataType, const char *inClassName)
{
	return OSIsType((T *)nullptr, inArrayDepth, inDataType, inClassName);
}

JPH_NAMESPACE_END
