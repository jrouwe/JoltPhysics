// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <ObjectStream/ObjectStreamBinaryOut.h>
#include <Core/StringTools.h>

namespace JPH {

ObjectStreamBinaryOut::ObjectStreamBinaryOut(ostream &inStream) :
	ObjectStreamOut(inStream),
	mNextStringID(0x80000000)
{
	string header;
	header = StringFormat("BOS%2d.%02d", ObjectStream::sVersion, ObjectStream::sRevision);
	mStream.write(header.c_str(), header.size());
}

void ObjectStreamBinaryOut::WriteDataType(EDataType inType)
{
	mStream.write((const char *)&inType, sizeof(inType));
}

void ObjectStreamBinaryOut::WriteName(const char *inName)
{
	WritePrimitiveData(string(inName));
}

void ObjectStreamBinaryOut::WriteIdentifier(Identifier inIdentifier)
{
	mStream.write((const char *)&inIdentifier, sizeof(inIdentifier));
}

void ObjectStreamBinaryOut::WriteCount(uint32 inCount)
{
	mStream.write((const char *)&inCount, sizeof(inCount));
}

void ObjectStreamBinaryOut::WritePrimitiveData(const uint8 &inPrimitive)
{
	mStream.write((const char *)&inPrimitive, sizeof(inPrimitive));
}

void ObjectStreamBinaryOut::WritePrimitiveData(const uint16 &inPrimitive)
{
	mStream.write((const char *)&inPrimitive, sizeof(inPrimitive));
}

void ObjectStreamBinaryOut::WritePrimitiveData(const int &inPrimitive)
{
	mStream.write((const char *)&inPrimitive, sizeof(inPrimitive));
}

void ObjectStreamBinaryOut::WritePrimitiveData(const uint32 &inPrimitive)
{
	mStream.write((const char *)&inPrimitive, sizeof(inPrimitive));
}

void ObjectStreamBinaryOut::WritePrimitiveData(const uint64 &inPrimitive)
{
	mStream.write((const char *)&inPrimitive, sizeof(inPrimitive));
}

void ObjectStreamBinaryOut::WritePrimitiveData(const float &inPrimitive)
{
	mStream.write((const char *)&inPrimitive, sizeof(inPrimitive));
}

void ObjectStreamBinaryOut::WritePrimitiveData(const bool &inPrimitive)
{
	mStream.write((const char *)&inPrimitive, sizeof(inPrimitive));
}

void ObjectStreamBinaryOut::WritePrimitiveData(const string &inPrimitive)
{
	// Empty strings are trivial
	if (inPrimitive.empty())
	{
		WritePrimitiveData((uint32)0);
		return;
	}

	// Check if we've already written this string
	StringTable::iterator i = mStringTable.find(inPrimitive);
	if (i != mStringTable.end())
	{
		WritePrimitiveData(i->second);
		return;
	}

	// Insert string in table
	mStringTable.insert(StringTable::value_type(inPrimitive, mNextStringID++));

	// Write string
	uint32 len = min((uint32)inPrimitive.size(), (uint32)0x7fffffff);
	WritePrimitiveData(len);
	mStream.write(inPrimitive.c_str(), len);
}

void ObjectStreamBinaryOut::WritePrimitiveData(const Float3 &inPrimitive)
{
	mStream.write((const char *)&inPrimitive, sizeof(Float3));
}

void ObjectStreamBinaryOut::WritePrimitiveData(const Vec3 &inPrimitive)
{
	mStream.write((const char *)&inPrimitive, 3 * sizeof(float));
}

void ObjectStreamBinaryOut::WritePrimitiveData(const Vec4 &inPrimitive)
{
	mStream.write((const char *)&inPrimitive, sizeof(inPrimitive));
}

void ObjectStreamBinaryOut::WritePrimitiveData(const Quat &inPrimitive)
{
	mStream.write((const char *)&inPrimitive, sizeof(inPrimitive));
}

void ObjectStreamBinaryOut::WritePrimitiveData(const Mat44 &inPrimitive)
{
	mStream.write((const char *)&inPrimitive, sizeof(inPrimitive));
}

} // JPH