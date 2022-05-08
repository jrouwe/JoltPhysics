// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/ObjectStream/SerializableAttribute.h>
#include <Jolt/ObjectStream/GetPrimitiveTypeOfType.h>

JPH_NAMESPACE_BEGIN

///@name Serialization operations
template <class T>
const RTTI *			TypedAttrGetMemberPrimitiveType()
{ 
	return GetPrimitiveTypeOfType((T *)nullptr);
}

template <class T>
bool					TypedAttrIsType(int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName)
{
	return OSIsType((T *)nullptr, inArrayDepth, inDataType, inClassName);
}

template <class T>
bool					TypedAttrReadData(ObjectStreamIn &ioStream, void *inObject)
{
	return OSReadData(ioStream, *reinterpret_cast<T *>(inObject));
}

template <class T>
void					TypedAttrWriteData(ObjectStreamOut &ioStream, const void *inObject)
{
	OSWriteData(ioStream, *reinterpret_cast<const T *>(inObject));
}

template <class T>
void					TypedAttrWriteDataType(ObjectStreamOut &ioStream)
{
	OSWriteDataType(ioStream, (T *)nullptr);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Macros to add properties to be serialized
//////////////////////////////////////////////////////////////////////////////////////////

template <class Class, class T>
inline void AddSerializableAttributeTyped(RTTI &inRTTI, T Class::*inMember, const char *inName)
{
	inRTTI.AddAttribute(SerializableAttribute(inName, intptr_t(&(reinterpret_cast<Class *>(0)->*inMember)), TypedAttrGetMemberPrimitiveType<T>, TypedAttrIsType<T>, TypedAttrReadData<T>, TypedAttrWriteData<T>, TypedAttrWriteDataType<T>));
}

// JPH_ADD_ATTRIBUTE
#define JPH_ADD_ATTRIBUTE(class_name, member_name)																	\
						AddSerializableAttributeTyped(inRTTI, &class_name::member_name, #member_name);

JPH_NAMESPACE_END
