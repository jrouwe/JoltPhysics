// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/ObjectStream/ObjectStreamIn.h>

JPH_NAMESPACE_BEGIN

/// Implementation of ObjectStream binary input stream.
class ObjectStreamBinaryIn : public ObjectStreamIn
{
public:
	/// Constructor
	explicit 					ObjectStreamBinaryIn(istream &inStream);

	///@name Input type specific operations
	virtual bool				ReadDataType(EOSDataType &outType) override;
	virtual bool				ReadName(string &outName) override;
	virtual bool				ReadIdentifier(Identifier &outIdentifier) override;
	virtual bool				ReadCount(uint32 &outCount) override;

	virtual bool				ReadPrimitiveData(uint8 &outPrimitive) override;
	virtual bool				ReadPrimitiveData(uint16 &outPrimitive) override;
	virtual bool				ReadPrimitiveData(int &outPrimitive) override;
	virtual bool				ReadPrimitiveData(uint32 &outPrimitive) override;
	virtual bool				ReadPrimitiveData(uint64 &outPrimitive) override;
	virtual bool				ReadPrimitiveData(float &outPrimitive) override;
	virtual bool				ReadPrimitiveData(bool &outPrimitive) override;
	virtual bool				ReadPrimitiveData(string &outPrimitive) override;
	virtual bool				ReadPrimitiveData(Float3 &outPrimitive) override;
	virtual bool				ReadPrimitiveData(Vec3 &outPrimitive) override;
	virtual bool				ReadPrimitiveData(Vec4 &outPrimitive) override;
	virtual bool				ReadPrimitiveData(Quat &outPrimitive) override;
	virtual bool				ReadPrimitiveData(Mat44 &outPrimitive) override;

private:
	using StringTable = unordered_map<uint32, string>;

	StringTable					mStringTable;
	uint32						mNextStringID = 0x80000000;
};

JPH_NAMESPACE_END
