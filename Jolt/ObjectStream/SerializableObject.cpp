// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt/Jolt.h>

#include <Jolt/ObjectStream/SerializableObject.h>

JPH_NAMESPACE_BEGIN

JPH_IMPLEMENT_SERIALIZABLE_ABSTRACT(SerializableObject)
{
}

void OSVisitCompounds(const void *inObject, const RTTI *inRTTI, const CompoundVisitor &inVisitor)
{
	JPH_ASSERT(inObject != nullptr);

	// Visit attributes
	for (int i = 0; i < inRTTI->GetAttributeCount(); ++i)
	{
		const SerializableAttribute *attr = DynamicCast<SerializableAttribute>(inRTTI->GetAttribute(i));
		if (attr != nullptr)
			attr->VisitCompounds(inObject, inVisitor);
	}

	// Call visitor
	inVisitor(inObject, inRTTI);
}

JPH_NAMESPACE_END
