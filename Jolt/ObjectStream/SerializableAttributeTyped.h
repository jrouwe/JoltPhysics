// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/ObjectStream/SerializableAttribute.h>
#include <Jolt/ObjectStream/GetPrimitiveTypeOfType.h>

JPH_NAMESPACE_BEGIN

/// Contains a serializable attribute of any type (except enum)
template <class Class, class T>
class SerializableAttributeTyped : public SerializableAttribute
{
public:
	/// Constructor
								SerializableAttributeTyped(T Class::*inMember, const char *inName)			: SerializableAttribute(inName), mMember(inMember) { }

	///@name Serialization operations
	virtual const RTTI *		GetMemberPrimitiveType() const override
	{ 
		return GetPrimitiveTypeOfType((T *)nullptr);
	}

	virtual const void *		GetMemberPointer(const void *inObject) const override
	{
		return &(((const Class *)inObject)->*mMember);
	}

	virtual bool				IsType(int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName) const override
	{
		return OSIsType((T *)nullptr, inArrayDepth, inDataType, inClassName);
	}

	virtual bool				ReadData(ObjectStreamIn &ioStream, void *inObject) const override
	{
		return OSReadData(ioStream, ((Class *)inObject)->*mMember);
	}

	virtual void				WriteData(ObjectStreamOut &ioStream, const void *inObject) const override
	{
		OSWriteData(ioStream, ((const Class *)inObject)->*mMember);
	}

	virtual void				WriteDataType(ObjectStreamOut &ioStream) const override
	{
		OSWriteDataType(ioStream, (T *)nullptr);
	}

	virtual void				VisitCompounds(const void *inObject, const CompoundVisitor &inVisitor) const override
	{
		OSVisitCompounds(((const Class *)inObject)->*mMember, inVisitor);
	}

private:
	T Class::*					mMember;
};

//////////////////////////////////////////////////////////////////////////////////////////
// Macros to add properties to be serialized
//////////////////////////////////////////////////////////////////////////////////////////

template <class Class, class T>
inline void AddSerializableAttributeTyped(RTTI &inRTTI, T Class::*inMember, const char *inName)
{
	inRTTI.AddAttribute(new SerializableAttributeTyped<Class, T>(inMember, inName));
}

// JPH_ADD_ATTRIBUTE
#define JPH_ADD_ATTRIBUTE(class_name, member_name)																	\
								AddSerializableAttributeTyped(inRTTI, &class_name::member_name, #member_name);

JPH_NAMESPACE_END
