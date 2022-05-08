// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/ObjectStream/ObjectStreamOut.h>

JPH_NAMESPACE_BEGIN

/// Implementation of ObjectStream binary output stream.
class ObjectStreamBinaryOut : public ObjectStreamOut
{
public:
	/// Constructor and destructor
	explicit 					ObjectStreamBinaryOut(ostream &inStream);

	///@name Output type specific operations
	virtual void				WriteDataType(EOSDataType inType) override;
	virtual void				WriteName(const char *inName) override;
	virtual void				WriteIdentifier(Identifier inIdentifier) override;
	virtual void				WriteCount(uint32 inCount) override;
								
	virtual void				WritePrimitiveData(const uint8 &inPrimitive) override;
	virtual void				WritePrimitiveData(const uint16 &inPrimitive) override;
	virtual void				WritePrimitiveData(const int &inPrimitive) override;
	virtual void				WritePrimitiveData(const uint32 &inPrimitive) override;
	virtual void				WritePrimitiveData(const uint64 &inPrimitive) override;
	virtual void				WritePrimitiveData(const float &inPrimitive) override;
	virtual void				WritePrimitiveData(const bool &inPrimitive) override;
	virtual void				WritePrimitiveData(const string &inPrimitive) override;
	virtual void				WritePrimitiveData(const Float3 &inPrimitive) override;
	virtual void				WritePrimitiveData(const Vec3 &inPrimitive) override;
	virtual void				WritePrimitiveData(const Vec4 &inPrimitive) override;
	virtual void				WritePrimitiveData(const Quat &inPrimitive) override;
	virtual void				WritePrimitiveData(const Mat44 &inPrimitive) override;

private:
	using StringTable = unordered_map<string, uint32>;

	StringTable					mStringTable;
	uint32						mNextStringID = 0x80000000;
};

JPH_NAMESPACE_END
