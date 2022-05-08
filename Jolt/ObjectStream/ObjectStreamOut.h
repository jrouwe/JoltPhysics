// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/ObjectStream/ObjectStream.h>
#include <Jolt/Core/RTTI.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <queue>
#include <fstream>
#include <unordered_set>
#include <unordered_map>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

/// ObjectStreamOut contains all logic for writing an object to disk. It is the base 
/// class for the text and binary output streams (ObjectStreamTextOut and ObjectStreamBinaryOut).
class ObjectStreamOut : public ObjectStream
{
private:
	struct ObjectInfo;

public:
	/// Main function to write an object to a stream
	template <class T> 
	static bool	sWriteObject(ostream &inStream, ObjectStream::EStreamType inType, const T &inObject)
	{
		// Create the output stream
		bool result = false;
		ObjectStreamOut *stream = ObjectStreamOut::Open(inType, inStream);
		if (stream) 
		{
			// Write the object to the stream
			result = stream->Write((void *)&inObject, GetRTTI(&inObject));
			delete stream;
		}

		return result;		
	}

	/// Main function to write an object to a file
	template <class T>
	static bool	sWriteObject(const char *inFileName, ObjectStream::EStreamType inType, const T &inObject)
	{
		ofstream stream;
		stream.open(inFileName, ofstream::out | ofstream::trunc | ofstream::binary);
		if (!stream.is_open()) 
			return false;
		return sWriteObject(stream, inType, inObject);
	}

	//////////////////////////////////////////////////////
	// EVERYTHING BELOW THIS SHOULD NOT DIRECTLY BE CALLED
	//////////////////////////////////////////////////////

	///@name Serialization operations
	bool						Write(const void *inObject, const RTTI *inRTTI);
	void						WriteObject(const void *inObject);
	void						QueueRTTI(const RTTI *inRTTI);
	void						WriteRTTI(const RTTI *inRTTI);
	void						WriteClassData(const RTTI *inRTTI, const void *inInstance);
	void						WritePointerData(const RTTI *inRTTI, const void *inPointer);

	///@name Output type specific operations
	virtual void				WriteDataType(EOSDataType inType) = 0;
	virtual void				WriteName(const char *inName) = 0;
	virtual void				WriteIdentifier(Identifier inIdentifier) = 0;
	virtual void				WriteCount(uint32 inCount) = 0;

	virtual void				WritePrimitiveData(const uint8 &inPrimitive) = 0;
	virtual void				WritePrimitiveData(const uint16 &inPrimitive) = 0;
	virtual void				WritePrimitiveData(const int &inPrimitive) = 0;
	virtual void				WritePrimitiveData(const uint32 &inPrimitive) = 0;
	virtual void				WritePrimitiveData(const uint64 &inPrimitive) = 0;
	virtual void				WritePrimitiveData(const float &inPrimitive) = 0;
	virtual void				WritePrimitiveData(const bool &inPrimitive) = 0;
	virtual void				WritePrimitiveData(const string &inPrimitive) = 0;
	virtual void				WritePrimitiveData(const Float3 &inPrimitive) = 0;
	virtual void				WritePrimitiveData(const Vec3 &inPrimitive) = 0;
	virtual void				WritePrimitiveData(const Vec4 &inPrimitive) = 0;
	virtual void				WritePrimitiveData(const Quat &inPrimitive) = 0;
	virtual void				WritePrimitiveData(const Mat44 &inPrimitive) = 0;

	///@name Layout hints (for text output)
	virtual void				HintNextItem()												{ /* Default is do nothing */ }
	virtual void				HintIndentUp()												{ /* Default is do nothing */ }
	virtual void				HintIndentDown()											{ /* Default is do nothing */ }

protected:
	/// Static constructor
	static ObjectStreamOut *	Open(EStreamType inType, ostream &inStream);

	/// Constructor
	explicit 					ObjectStreamOut(ostream &inStream);

	ostream &					mStream;

private:
	struct ObjectInfo
	{
								ObjectInfo()												: mIdentifier(0), mRTTI(nullptr) { }
								ObjectInfo(Identifier inIdentifier, const RTTI *inRTTI)		: mIdentifier(inIdentifier), mRTTI(inRTTI) { }

		Identifier				mIdentifier;
		const RTTI *			mRTTI;
	};

	using IdentifierMap = unordered_map<const void *, ObjectInfo>;
	using ClassSet = unordered_set<const RTTI *>;
	using ObjectQueue = queue<const void *>;
	using ClassQueue = queue<const RTTI *>;

	Identifier					mNextIdentifier = sNullIdentifier + 1;						///< Next free identifier for this stream
	IdentifierMap				mIdentifierMap;												///< Links object pointer to an identifier
	ObjectQueue					mObjectQueue;												///< Queue of objects to be written
	ClassSet					mClassSet;													///< List of classes already written
	ClassQueue					mClassQueue;												///< List of classes waiting to be written
};	

// Define macro to declare functions for a specific primitive type
#define JPH_DECLARE_PRIMITIVE(name)															\
	void	OSWriteDataType(ObjectStreamOut &ioStream, name *);								\
	void	OSWriteData(ObjectStreamOut &ioStream, const name &inPrimitive);

// This file uses the JPH_DECLARE_PRIMITIVE macro to define all types
#include <Jolt/ObjectStream/ObjectStreamTypes.h>

// Define serialization templates for dynamic arrays
template <class T>
void OSWriteDataType(ObjectStreamOut &ioStream, vector<T> *)		
{ 
	ioStream.WriteDataType(EOSDataType::Array); 
	OSWriteDataType(ioStream, (T *)nullptr); 
}

template <class T>
void OSWriteData(ObjectStreamOut &ioStream, const vector<T> &inArray)
{
	// Write size of array
	ioStream.HintNextItem();
	ioStream.WriteCount((uint32)inArray.size());

	// Write data in array
	ioStream.HintIndentUp();	
	for (const T &v : inArray)
		OSWriteData(ioStream, v);
	ioStream.HintIndentDown();
}

/// Define serialization templates for static arrays
template <class T, uint N>
void OSWriteDataType(ObjectStreamOut &ioStream, StaticArray<T, N> *)		
{ 
	ioStream.WriteDataType(EOSDataType::Array); 
	OSWriteDataType(ioStream, (T *)nullptr); 
}

template <class T, uint N>
void OSWriteData(ObjectStreamOut &ioStream, const StaticArray<T, N> &inArray)
{
	// Write size of array
	ioStream.HintNextItem();
	ioStream.WriteCount(inArray.size());

	// Write data in array
	ioStream.HintIndentUp();	
	for (const typename StaticArray<T, N>::value_type &v : inArray)
		OSWriteData(ioStream, v);
	ioStream.HintIndentDown();
}

/// Define serialization templates for C style arrays
template <class T, uint N>
void OSWriteDataType(ObjectStreamOut &ioStream, T (*)[N])		
{ 
	ioStream.WriteDataType(EOSDataType::Array); 
	OSWriteDataType(ioStream, (T *)nullptr); 
}

template <class T, uint N>
void OSWriteData(ObjectStreamOut &ioStream, const T (&inArray)[N])
{
	// Write size of array
	ioStream.HintNextItem();
	ioStream.WriteCount((uint32)N);

	// Write data in array
	ioStream.HintIndentUp();	
	for (const T &v : inArray)
		OSWriteData(ioStream, v);
	ioStream.HintIndentDown();
}

/// Define serialization templates for references
template <class T>
void OSWriteDataType(ObjectStreamOut &ioStream, Ref<T> *)
{
	OSWriteDataType(ioStream, (T *)nullptr);
}

template <class T>
void OSWriteData(ObjectStreamOut &ioStream, const Ref<T> &inRef)
{
	if (inRef != nullptr)
		ioStream.WritePointerData(GetRTTI(inRef.GetPtr()), inRef.GetPtr());
	else
		ioStream.WritePointerData(nullptr, nullptr);
}

template <class T>
void OSWriteDataType(ObjectStreamOut &ioStream, RefConst<T> *)
{
	OSWriteDataType(ioStream, (T *)nullptr);
}

template <class T>
void OSWriteData(ObjectStreamOut &ioStream, const RefConst<T> &inRef)
{
	if (inRef != nullptr)
		ioStream.WritePointerData(GetRTTI(inRef.GetPtr()), inRef.GetPtr());
	else
		ioStream.WritePointerData(nullptr, nullptr);
}

JPH_NAMESPACE_END
