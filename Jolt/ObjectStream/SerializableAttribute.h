// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/ObjectStream/ObjectStreamIn.h>
#include <Jolt/ObjectStream/ObjectStreamOut.h>

JPH_NAMESPACE_BEGIN

using CompoundVisitor = function<void(const void *, const RTTI *)>;

/// Attributes are members of classes that need to be serialized. This extends the
/// basic attribute defined in RTTI.h
class SerializableAttribute : public RTTIAttribute
{
public:
	/// Constructor
	using RTTIAttribute::RTTIAttribute;

	///@name Serialization operations
	virtual bool				IsType(int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName) const = 0;
	virtual bool				ReadData(ObjectStreamIn &ioStream, void *inObject) const = 0;
	virtual void				WriteData(ObjectStreamOut &ioStream, const void *inObject) const = 0;
	virtual void				WriteDataType(ObjectStreamOut &ioStream) const = 0;
	virtual void				VisitCompounds(const void *inObject, const CompoundVisitor &inVisitor) const = 0;
};

JPH_NAMESPACE_END
