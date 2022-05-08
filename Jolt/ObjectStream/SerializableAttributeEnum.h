// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/ObjectStream/SerializableAttribute.h>
#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Jolt/ObjectStream/ObjectStreamOut.h>

JPH_NAMESPACE_BEGIN

inline bool				EnumAttrIsType(int inArrayDepth, EOSDataType inDataType, const char *inClassName)
{
	return (inArrayDepth == 0 && inDataType == EOSDataType::T_uint32);
}

template <class T>
bool					EnumAttrReadData(ObjectStreamIn &ioStream, void *inObject)
{
	uint32 temporary;
	if (OSReadData(ioStream, temporary)) 
	{
		*reinterpret_cast<T *>(inObject) = (T)temporary;
		return true;
	}

	return false;
}

template <class T>
void					EnumAttrWriteData(ObjectStreamOut &ioStream, const void *inObject)
{
	static_assert(sizeof(T) <= sizeof(uint32));
	uint32 temporary = uint32(*reinterpret_cast<const T *>(inObject));
	OSWriteData(ioStream, temporary);
}

inline void				EnumAttrWriteDataType(ObjectStreamOut &ioStream)
{
	ioStream.WriteDataType(EOSDataType::T_uint32);
}

//////////////////////////////////////////////////////////////////////////////////////////
// Macros to add properties to be serialized
//////////////////////////////////////////////////////////////////////////////////////////

template <class Class, class T>
inline void AddSerializableAttributeEnum(RTTI &inRTTI, T Class::*inMember, const char *inName)
{
	inRTTI.AddAttribute(SerializableAttribute(inName, intptr_t(&(reinterpret_cast<Class *>(0)->*inMember)), nullptr, EnumAttrIsType, EnumAttrReadData<T>, EnumAttrWriteData<T>, EnumAttrWriteDataType));
}

// JPH_ADD_ENUM_ATTRIBUTE
#define JPH_ADD_ENUM_ATTRIBUTE(class_name, member_name)																\
						AddSerializableAttributeEnum(inRTTI, &class_name::member_name, #member_name);

JPH_NAMESPACE_END
