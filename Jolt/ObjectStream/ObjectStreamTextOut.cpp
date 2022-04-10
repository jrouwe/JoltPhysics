// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/ObjectStream/ObjectStreamTextOut.h>
#include <Jolt/Core/StringTools.h>

JPH_NAMESPACE_BEGIN

ObjectStreamTextOut::ObjectStreamTextOut(ostream &inStream) :
	ObjectStreamOut(inStream)
{
	WriteWord(StringFormat("TOS%2d.%02d", ObjectStream::sVersion, ObjectStream::sRevision));
}

void ObjectStreamTextOut::WriteDataType(EDataType inType)
{
	switch (inType) 
	{
	case EDataType::Declare:		WriteWord("declare ");		break;
	case EDataType::Object:			WriteWord("object ");		break;
	case EDataType::Instance:		WriteWord("instance ");		break;
	case EDataType::Pointer:		WriteWord("pointer ");		break;
	case EDataType::Array:			WriteWord("array ");		break;
	case EDataType::T_uint8:		WriteWord("uint8");			break;
	case EDataType::T_uint16:		WriteWord("uint16");		break;
	case EDataType::T_int:			WriteWord("int");			break;
	case EDataType::T_uint32:		WriteWord("uint32");		break;
	case EDataType::T_uint64:		WriteWord("uint64");		break;
	case EDataType::T_float:		WriteWord("float");			break;
	case EDataType::T_bool:			WriteWord("bool");			break;
	case EDataType::T_string:		WriteWord("string");		break;
	case EDataType::T_Float3:		WriteWord("float3");		break;
	case EDataType::T_Vec3:			WriteWord("vec3");			break;
	case EDataType::T_Vec4:			WriteWord("vec4");			break;
	case EDataType::T_Quat:			WriteWord("quat");			break;
	case EDataType::T_Mat44:		WriteWord("mat44");			break;
	case EDataType::Invalid:
	default:						JPH_ASSERT(false);			break;
	}
}

void ObjectStreamTextOut::WriteName(const char *inName)
{
	WriteWord(string(inName) + " ");
}

void ObjectStreamTextOut::WriteIdentifier(Identifier inIdentifier)
{
	WriteWord(StringFormat("%08X", inIdentifier));
}

void ObjectStreamTextOut::WriteCount(uint32 inCount)
{
	WriteWord(to_string(inCount));
}

void ObjectStreamTextOut::WritePrimitiveData(const uint8 &inPrimitive)
{
	WriteWord(to_string(inPrimitive));
}

void ObjectStreamTextOut::WritePrimitiveData(const uint16 &inPrimitive)
{
	WriteWord(to_string(inPrimitive));
}

void ObjectStreamTextOut::WritePrimitiveData(const int &inPrimitive)
{
	WriteWord(to_string(inPrimitive));
}

void ObjectStreamTextOut::WritePrimitiveData(const uint32 &inPrimitive)
{
	WriteWord(to_string(inPrimitive));
}

void ObjectStreamTextOut::WritePrimitiveData(const uint64 &inPrimitive)
{
	WriteWord(to_string(inPrimitive));
}

void ObjectStreamTextOut::WritePrimitiveData(const float &inPrimitive)
{
	ostringstream stream;
	stream.precision(9);
	stream << inPrimitive;
	WriteWord(stream.str());
}

void ObjectStreamTextOut::WritePrimitiveData(const bool &inPrimitive)
{
	WriteWord(inPrimitive? "true" : "false");
}

void ObjectStreamTextOut::WritePrimitiveData(const Float3 &inPrimitive)
{
	WritePrimitiveData(inPrimitive.x);
	WriteChar(' ');
	WritePrimitiveData(inPrimitive.y);
	WriteChar(' ');
	WritePrimitiveData(inPrimitive.z);
}

void ObjectStreamTextOut::WritePrimitiveData(const Vec3 &inPrimitive)
{
	WritePrimitiveData(inPrimitive.GetX());
	WriteChar(' ');
	WritePrimitiveData(inPrimitive.GetY());
	WriteChar(' ');
	WritePrimitiveData(inPrimitive.GetZ());
}

void ObjectStreamTextOut::WritePrimitiveData(const Vec4 &inPrimitive)
{
	WritePrimitiveData(inPrimitive.GetX());
	WriteChar(' ');
	WritePrimitiveData(inPrimitive.GetY());
	WriteChar(' ');
	WritePrimitiveData(inPrimitive.GetZ());
	WriteChar(' ');
	WritePrimitiveData(inPrimitive.GetW());
}

void ObjectStreamTextOut::WritePrimitiveData(const Quat &inPrimitive)
{
	WritePrimitiveData(inPrimitive.GetX());
	WriteChar(' ');
	WritePrimitiveData(inPrimitive.GetY());
	WriteChar(' ');
	WritePrimitiveData(inPrimitive.GetZ());
	WriteChar(' ');
	WritePrimitiveData(inPrimitive.GetW());
}

void ObjectStreamTextOut::WritePrimitiveData(const Mat44 &inPrimitive)
{
	WritePrimitiveData(inPrimitive.GetColumn4(0));
	WriteChar(' ');
	WritePrimitiveData(inPrimitive.GetColumn4(1));
	WriteChar(' ');
	WritePrimitiveData(inPrimitive.GetColumn4(2));
	WriteChar(' ');
	WritePrimitiveData(inPrimitive.GetColumn4(3));
}

void ObjectStreamTextOut::WritePrimitiveData(const string &inPrimitive)
{
	string temporary(inPrimitive);
	StringReplace(temporary, "\\", "\\\\");
	StringReplace(temporary, "\n", "\\n");
	StringReplace(temporary, "\t", "\\t");
	StringReplace(temporary, "\"", "\\\"");
	WriteWord(string("\"") + temporary + string("\""));
}

void ObjectStreamTextOut::HintNextItem()
{
	WriteWord("\r\n");
	for (int i = 0; i < mIndentation; ++i)
		WriteWord("  ");
}

void ObjectStreamTextOut::HintIndentUp()
{
	++mIndentation;
}

void ObjectStreamTextOut::HintIndentDown()
{
	--mIndentation;
}

void ObjectStreamTextOut::WriteChar(char inChar)
{
	mStream.put(inChar);
}

void ObjectStreamTextOut::WriteWord(const string &inWord)
{
	mStream << inWord;
}

JPH_NAMESPACE_END
