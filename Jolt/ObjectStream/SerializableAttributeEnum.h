// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/ObjectStream/SerializableAttribute.h>

JPH_NAMESPACE_BEGIN

/// Contains an serialize attribute of type enum
template <class Class, class T>
class SerializableAttributeEnum : public SerializableAttribute
{
public:
	/// Constructor
								SerializableAttributeEnum(T Class::*inMember, const char *inName)			: SerializableAttribute(inName), mMember(inMember) { }

	///@name Serialization operations
	virtual const void *		GetMemberPointer(const void *inObject) const override
	{
		return &(((const Class *)inObject)->*mMember);
	}

	virtual bool				IsType(int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName) const override
	{
		return (inArrayDepth == 0 && inDataType == ObjectStream::EDataType::T_uint32);
	}

	virtual bool				ReadData(ObjectStreamIn &ioStream, void *inObject) const override
	{
		uint32 temporary;
		if (OSReadData(ioStream, temporary)) 
		{
			((Class *)inObject)->*mMember = (T)temporary;
			return true;
		}

		return false;
	}

	virtual void				WriteData(ObjectStreamOut &ioStream, const void *inObject) const override
	{
		static_assert(sizeof(T) <= sizeof(uint32));
		uint32 temporary = uint32(((const Class *)inObject)->*mMember);
		OSWriteData(ioStream, temporary);
	}

	virtual void				WriteDataType(ObjectStreamOut &ioStream) const override
	{
		ioStream.WriteDataType(ObjectStream::EDataType::T_uint32);
	}

	virtual void				VisitCompounds(const void *inObject, const CompoundVisitor &inVisitor) const override
	{
		// An enum is not a compound, do nothing
	}

private:
	T Class::*					mMember;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Macros to add properties to be serialized
//////////////////////////////////////////////////////////////////////////////////////////

template <class Class, class T>
inline void AddSerializableAttributeEnum(RTTI &inRTTI, T Class::*inMember, const char *inName)
{
	inRTTI.AddAttribute(new SerializableAttributeEnum<Class, T>(inMember, inName));
}

// JPH_ADD_ENUM_ATTRIBUTE
#define JPH_ADD_ENUM_ATTRIBUTE(class_name, member_name)																\
								AddSerializableAttributeEnum(inRTTI, &class_name::member_name, #member_name);

JPH_NAMESPACE_END
