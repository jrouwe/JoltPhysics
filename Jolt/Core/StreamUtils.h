// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/Core/Result.h>
#include <Jolt/Core/UnorderedMap.h>

JPH_NAMESPACE_BEGIN

namespace StreamUtils {

template <class Type>
using ObjectToIDMap = UnorderedMap<const Type *, uint32>;

template <class Type>
using IDToObjectMap = Array<Ref<Type>>;

/// Save an object reference to a stream. Uses a map to map objects to IDs which is also used to prevent writing duplicates.
template <class Type>
void				sSaveObjectReference(StreamOut &inStream, const Type *inObject, ObjectToIDMap<Type> *ioObjectToIDMap)
{
	// Save group filter
	if (ioObjectToIDMap == nullptr || inObject == nullptr)
	{
		// Write null ID
		inStream.Write(~uint32(0));
	}
	else
	{
		ObjectToIDMap<Type>::const_iterator id = ioObjectToIDMap->find(inObject);
		if (id != ioObjectToIDMap->end())
		{
			// Existing group filter, write ID
			inStream.Write(id->second);
		}
		else
		{
			// New group filter, write the ID
			uint32 new_id = (uint32)ioObjectToIDMap->size();
			(*ioObjectToIDMap)[inObject] = new_id;
			inStream.Write(new_id);

			// Write the object
			inObject->SaveBinaryState(inStream);
		}
	}
}

/// Restore an object reference from stream.
template <class Type>
Result<Ref<Type>>	sRestoreObjectReference(StreamIn &inStream, IDToObjectMap<Type> &ioIDToObjectMap)
{
	Result<Ref<Type>> result;

	// Read group filter
	uint32 id = ~uint32(0);
	inStream.Read(id);
	if (id == ~uint32(0))
	{
		result.Set(nullptr);
		return result;
	}

	// Check if it already exists
	if (id >= ioIDToObjectMap.size())
	{
		// New object, restore it
		result = Type::sRestoreFromBinaryState(inStream);
		if (result.HasError())
			return result;
		JPH_ASSERT(id == ioIDToObjectMap.size());
		ioIDToObjectMap.push_back(result.Get());
	}
	else
	{
		// Existing group filter
		result.Set(ioIDToObjectMap[id].GetPtr());
	}

	return result;
}

template <class ArrayType, class ValueType>
void				sSaveObjectArray(StreamOut &inStream, const ArrayType &inArray, ObjectToIDMap<ValueType> *ioObjectToIDMap)
{
	inStream.Write((size_t)inArray.size());
	for (const ValueType *value: inArray)
		sSaveObjectReference(inStream, value, ioObjectToIDMap);
}

template <class ArrayType, class ValueType>
Result<ArrayType>	sRestoreObjectArray(StreamIn &inStream, IDToObjectMap<ValueType> &ioIDToObjectMap)
{
	Result<ArrayType> result;

	size_t len;
	inStream.Read(len);
	if (inStream.IsEOF() || inStream.IsFailed())
	{
		result.SetError("Failed to read stream");
		return result;
	}

	ArrayType values;
	values.reserve(len);
	for (size_t i = 0; i < len; ++i)
	{
		Result value = sRestoreObjectReference(inStream, ioIDToObjectMap);
		if (value.HasError())
		{
			result.SetError(value.GetError());
			return result;
		}
		values.push_back(std::move(value.Get()));
	}

	result.Set(values);
	return result;
}

} // StreamUtils

JPH_NAMESPACE_END
