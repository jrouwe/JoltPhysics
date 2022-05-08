// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/ObjectStream/ObjectStream.h>
#include <Jolt/Core/Reference.h>
#include <Jolt/Core/RTTI.h>

JPH_SUPPRESS_WARNINGS_STD_BEGIN
#include <fstream>
#include <unordered_map>
JPH_SUPPRESS_WARNINGS_STD_END

JPH_NAMESPACE_BEGIN

/// ObjectStreamIn contains all logic for reading an object from disk. It is the base
/// class for the text and binary input streams (ObjectStreamTextIn and ObjectStreamBinaryIn).
class ObjectStreamIn : public ObjectStream
{
private:
	struct ClassDescription;

public:
	/// Main function to read an object from a stream
	template <class T> 
	static bool sReadObject(istream &inStream, T *&outObject)
	{
		// Create the input stream
		bool result = false;
		ObjectStreamIn *stream = ObjectStreamIn::Open(inStream);
		if (stream) 
		{
			// Read the object
			outObject = (T *)stream->Read(JPH_RTTI(T));
			result = (outObject != nullptr);
			delete stream;
		}
		return result;
	}

	/// Main function to read an object from a stream (reference counting pointer version)
	template <class T> 
	static bool sReadObject(istream &inStream, Ref<T> &outObject)
	{
		T *object = nullptr;
		bool result = sReadObject(inStream, object);
		outObject = object;
		return result;
	}

	/// Main function to read an object from a file
	template <class T>
	static bool sReadObject(const char *inFileName, T *&outObject)
	{
		ifstream stream;
		stream.open(inFileName, ifstream::in | ifstream::binary);
		if (!stream.is_open()) 
			return false;
		return sReadObject(stream, outObject);
	}

	/// Main function to read an object from a file (reference counting pointer version)
	template <class T>
	static bool sReadObject(const char *inFileName, Ref<T> &outObject)
	{
		T *object = nullptr;
		bool result = sReadObject(inFileName, object);
		outObject = object;
		return result;
	}

	//////////////////////////////////////////////////////
	// EVERYTHING BELOW THIS SHOULD NOT DIRECTLY BE CALLED
	//////////////////////////////////////////////////////

	///@name Serialization operations
	void *						Read(const RTTI *inRTTI);
	void *						ReadObject(const RTTI *& outRTTI);
	bool						ReadRTTI();
	bool						ReadClassData(const char *inClassName, void *inInstance);
	bool						ReadClassData(const ClassDescription &inClassDesc, void *inInstance);
	bool						ReadPointerData(const RTTI *inRTTI, void **inPointer, int inRefCountOffset = -1);
	bool						SkipAttributeData(int inArrayDepth, EOSDataType inDataType, const char *inClassName);

	///@name Input type specific operations
	virtual bool				ReadDataType(EOSDataType &outType) = 0;
	virtual bool				ReadName(string &outName) = 0;
	virtual bool				ReadIdentifier(Identifier &outIdentifier) = 0;
	virtual bool				ReadCount(uint32 &outCount) = 0;

	virtual bool				ReadPrimitiveData(uint8 &outPrimitive) = 0;
	virtual bool				ReadPrimitiveData(uint16 &outPrimitive) = 0;
	virtual bool				ReadPrimitiveData(int &outPrimitive) = 0;
	virtual bool				ReadPrimitiveData(uint32 &outPrimitive) = 0;
	virtual bool				ReadPrimitiveData(uint64 &outPrimitive) = 0;
	virtual bool				ReadPrimitiveData(float &outPrimitive) = 0;
	virtual bool				ReadPrimitiveData(bool &outPrimitive) = 0;
	virtual bool				ReadPrimitiveData(string &outPrimitive) = 0;
	virtual bool				ReadPrimitiveData(Float3 &outPrimitive) = 0;
	virtual bool				ReadPrimitiveData(Vec3 &outPrimitive) = 0;
	virtual bool				ReadPrimitiveData(Vec4 &outPrimitive) = 0;
	virtual bool				ReadPrimitiveData(Quat &outPrimitive) = 0;
	virtual bool				ReadPrimitiveData(Mat44 &outPrimitive) = 0;

protected:
	/// Constructor
	explicit 					ObjectStreamIn(istream &inStream);

	/// Determine the type and version of an object stream
	static bool					GetInfo(istream &inStream, EStreamType &outType, int &outVersion, int &outRevision);

	/// Static constructor
	static ObjectStreamIn *		Open(istream &inStream);

	istream &					mStream;

private:
	/// Class descriptions
	struct AttributeDescription
	{
		int						mArrayDepth = 0;
		EOSDataType				mDataType = EOSDataType::Invalid;
		string					mClassName;
		int						mIndex = -1;
	};

	struct ClassDescription
	{
								ClassDescription() = default;
		explicit 				ClassDescription(const RTTI *inRTTI)					: mRTTI(inRTTI) { }

		const RTTI *			mRTTI = nullptr;
		vector<AttributeDescription>	mAttributes;
	};
	
	struct ObjectInfo
	{
								ObjectInfo() = default;
								ObjectInfo(void *inInstance, const RTTI *inRTTI)		: mInstance(inInstance), mRTTI(inRTTI) { }

		void *					mInstance = nullptr;
		const RTTI *			mRTTI = nullptr;
	};

	struct Link
	{
		void **					mPointer;
		int						mRefCountOffset;
		Identifier				mIdentifier;
		const RTTI *			mRTTI;
	};
	
	using IdentifierMap = unordered_map<Identifier, ObjectInfo>;
	using ClassDescriptionMap = unordered_map<string, ClassDescription>;

	ClassDescriptionMap			mClassDescriptionMap;
	IdentifierMap				mIdentifierMap;											///< Links identifier to an object pointer
	vector<Link>				mUnresolvedLinks;										///< All pointers (links) are resolved after reading the entire file, e.g. when all object exist
};

// Define macro to declare functions for a specific primitive type
#define JPH_DECLARE_PRIMITIVE(name)														\
	bool	OSReadData(ObjectStreamIn &ioStream, name &outPrimitive);

// This file uses the JPH_DECLARE_PRIMITIVE macro to define all types
#include <Jolt/ObjectStream/ObjectStreamTypes.h>

/// Define serialization templates for dynamic arrays
template <class T>
bool OSReadData(ObjectStreamIn &ioStream, vector<T> &inArray)
{
	bool continue_reading = true;

	// Read array length
	uint32 array_length;
	continue_reading = ioStream.ReadCount(array_length);

	// Read array items
	if (continue_reading) 
	{
		inArray.resize(array_length);
		for (uint32 el = 0; el < array_length && continue_reading; ++el) 
			continue_reading = OSReadData(ioStream, inArray[el]);
	}

	return continue_reading;
}

/// Define serialization templates for static arrays
template <class T, uint N>
bool OSReadData(ObjectStreamIn &ioStream, StaticArray<T, N> &inArray)
{
	bool continue_reading = true;

	// Read array length
	uint32 array_length;
	continue_reading = ioStream.ReadCount(array_length);

	// Check if we can fit this many elements
	if (array_length > N)
		return false;

	// Read array items
	if (continue_reading) 
	{
		inArray.resize(array_length);
		for (uint32 el = 0; el < array_length && continue_reading; ++el) 
			continue_reading = OSReadData(ioStream, inArray[el]);
	}

	return continue_reading;
}

/// Define serialization templates for C style arrays
template <class T, uint N>
bool OSReadData(ObjectStreamIn &ioStream, T (&inArray)[N])
{
	bool continue_reading = true;

	// Read array length
	uint32 array_length;
	continue_reading = ioStream.ReadCount(array_length);
	if (array_length != N)
		return false;

	// Read array items
	for (uint32 el = 0; el < N && continue_reading; ++el) 
		continue_reading = OSReadData(ioStream, inArray[el]);

	return continue_reading;
}

/// Define serialization templates for references
template <class T>
bool OSReadData(ObjectStreamIn &ioStream, Ref<T> &inRef)
{
	return ioStream.ReadPointerData(JPH_RTTI(T), inRef.InternalGetPointer(), T::sInternalGetRefCountOffset());
}

template <class T>
bool OSReadData(ObjectStreamIn &ioStream, RefConst<T> &inRef)
{
	return ioStream.ReadPointerData(JPH_RTTI(T), inRef.InternalGetPointer(), T::sInternalGetRefCountOffset());
}

JPH_NAMESPACE_END
