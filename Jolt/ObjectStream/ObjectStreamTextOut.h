// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/ObjectStream/ObjectStreamOut.h>

JPH_NAMESPACE_BEGIN

/// Implementation of ObjectStream text output stream.
class ObjectStreamTextOut : public ObjectStreamOut
{
public:
	/// Constructor and destructor
	explicit					ObjectStreamTextOut(ostream &inStream);

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

	///@name Layout hints (for text output)
	virtual void				HintNextItem() override;
	virtual void				HintIndentUp() override;
	virtual void				HintIndentDown() override;

private:
	void						WriteChar(char inChar);
	void						WriteWord(const string_view &inWord);

	int							mIndentation = 0;
};

JPH_NAMESPACE_END
