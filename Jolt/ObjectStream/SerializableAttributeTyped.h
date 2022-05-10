// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/ObjectStream/SerializableAttribute.h>
#include <Jolt/ObjectStream/GetPrimitiveTypeOfType.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Jolt/ObjectStream/ObjectStreamOut.h>

JPH_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////////////////////
// Macros to add properties to be serialized
//////////////////////////////////////////////////////////////////////////////////////////

template <class MemberType>
inline void AddSerializableAttributeTyped(RTTI &inRTTI, uint inOffset, const char *inName)
{
	inRTTI.AddAttribute(SerializableAttribute(inName, inOffset,
		[]()
		{ 
			return GetPrimitiveTypeOfType((MemberType *)nullptr);
		},
		[](int inArrayDepth, EOSDataType inDataType, const char *inClassName)
		{
			return OSIsType((MemberType *)nullptr, inArrayDepth, inDataType, inClassName);
		},
		[](ObjectStreamIn &ioStream, void *inObject)
		{
			return OSReadData(ioStream, *reinterpret_cast<MemberType *>(inObject));
		},
		[](ObjectStreamOut &ioStream, const void *inObject)
		{
			OSWriteData(ioStream, *reinterpret_cast<const MemberType *>(inObject));
		},
		[](ObjectStreamOut &ioStream)
		{
			OSWriteDataType(ioStream, (MemberType *)nullptr);
		}));
}

// JPH_ADD_ATTRIBUTE
#define JPH_ADD_ATTRIBUTE(class_name, member_name) \
	AddSerializableAttributeTyped<decltype(class_name::member_name)>(inRTTI, offsetof(class_name, member_name), #member_name);

JPH_NAMESPACE_END
