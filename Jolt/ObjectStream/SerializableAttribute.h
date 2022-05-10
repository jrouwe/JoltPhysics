// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

class RTTI;
class ObjectStreamIn;
class ObjectStreamOut;

/// Data type
enum class EOSDataType
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
	#include <Jolt/ObjectStream/ObjectStreamTypes.h>

	// Error values for read functions
	Invalid,																		///< Next token on the stream was not a valid data type
};

/// Attributes are members of classes that need to be serialized.
class SerializableAttribute
{
public:
	///@ Serialization functions
	using pGetMemberPrimitiveType = const RTTI * (*)();
	using pIsType = bool (*)(int inArrayDepth, EOSDataType inDataType, const char *inClassName);
	using pReadData = bool (*)(ObjectStreamIn &ioStream, void *inObject);
	using pWriteData = void (*)(ObjectStreamOut &ioStream, const void *inObject);
	using pWriteDataType = void (*)(ObjectStreamOut &ioStream);

	/// Constructor
								SerializableAttribute(const char *inName, uint inMemberOffset, pGetMemberPrimitiveType inGetMemberPrimitiveType, pIsType inIsType, pReadData inReadData, pWriteData inWriteData, pWriteDataType inWriteDataType) : mName(inName), mMemberOffset(inMemberOffset), mGetMemberPrimitiveType(inGetMemberPrimitiveType), mIsType(inIsType), mReadData(inReadData), mWriteData(inWriteData), mWriteDataType(inWriteDataType) { }

	/// Construct from other attribute with base class offset
								SerializableAttribute(const SerializableAttribute &inOther, int inBaseOffset) : mName(inOther.mName), mMemberOffset(inOther.mMemberOffset + inBaseOffset), mGetMemberPrimitiveType(inOther.mGetMemberPrimitiveType), mIsType(inOther.mIsType), mReadData(inOther.mReadData), mWriteData(inOther.mWriteData), mWriteDataType(inOther.mWriteDataType) { }

	/// Name of the attribute
	void						SetName(const char *inName)							{ mName = inName; }
	const char *				GetName() const										{ return mName; }

	/// In case this attribute contains an RTTI type, return it (note that a vector<sometype> will return the rtti of sometype)
	const RTTI *				GetMemberPrimitiveType() const
	{
		return mGetMemberPrimitiveType();
	}

	/// Check if this attribute is of a specific type
	bool						IsType(int inArrayDepth, EOSDataType inDataType, const char *inClassName) const
	{
		return mIsType(inArrayDepth, inDataType, inClassName);
	}

	/// Read the data for this attribute into attribute containing class inObject
	bool						ReadData(ObjectStreamIn &ioStream, void *inObject) const
	{
		return mReadData(ioStream, reinterpret_cast<uint8 *>(inObject) + mMemberOffset);
	}

	/// Write the data for this attribute from attribute containing class inObject
	void						WriteData(ObjectStreamOut &ioStream, const void *inObject) const
	{
		mWriteData(ioStream, reinterpret_cast<const uint8 *>(inObject) + mMemberOffset);
	}

	/// Write the data type of this attribute to a stream
	void						WriteDataType(ObjectStreamOut &ioStream) const
	{
		mWriteDataType(ioStream);
	}

private:
	// Name of the attribute
	const char *				mName;

	// Offset of the member relative to the class
	uint						mMemberOffset;

	// In case this attribute contains an RTTI type, return it (note that a vector<sometype> will return the rtti of sometype)
	pGetMemberPrimitiveType		mGetMemberPrimitiveType;

	// Serialization operations
	pIsType						mIsType;
	pReadData					mReadData;
	pWriteData					mWriteData;
	pWriteDataType				mWriteDataType;
};

JPH_NAMESPACE_END
