// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include <Jolt.h>

#include <Core/Factory.h>

namespace JPH {

Factory Factory::sInstance;

void *Factory::CreateObject(const char *inName)
{ 
	const RTTI *ci = Find(inName); 
	return ci != nullptr? ci->CreateObject() : nullptr; 
}

const RTTI *Factory::Find(const char *inName)
{ 
	ClassNameMap::iterator c = mClassNameMap.find(inName); 
	return c != mClassNameMap.end()? c->second : nullptr; 
}

const RTTI *Factory::Find(uint32 inHash)
{ 
	ClassHashMap::iterator c = mClassHashMap.find(inHash); 
	return c != mClassHashMap.end()? c->second : nullptr; 
}

bool Factory::Register(const RTTI *inRTTI)
{ 
	// Check if we already know the type
	if (Find(inRTTI->GetName()) != nullptr)
		return true;

	// Insert this class by name
	mClassNameMap.insert(ClassNameMap::value_type(inRTTI->GetName(), inRTTI));

	// Insert this class by hash
	if (!mClassHashMap.insert(ClassHashMap::value_type(inRTTI->GetHash(), inRTTI)).second)
	{
		JPH_ASSERT(false, "Hash collision registering type!");
		return false;
	}

	// Register base classes
	for (int i = 0; i < inRTTI->GetBaseClassCount(); ++i)
		if (!Register(inRTTI->GetBaseClass(i)))
			return false;

	// Register attribute classes
	for (int i = 0; i < inRTTI->GetAttributeCount(); ++i)
	{
		const RTTI *rtti = inRTTI->GetAttribute(i)->GetMemberPrimitiveType();
		if (rtti != nullptr)
			if (!Register(rtti))
				return false;
	}

	return true;
}

vector<const RTTI *> Factory::GetAllClasses()
{
	vector<const RTTI *> all_classes;
	all_classes.reserve(mClassNameMap.size());
	for (ClassNameMap::iterator c = mClassNameMap.begin(); c != mClassNameMap.end(); ++c)
		all_classes.push_back(c->second);
	return all_classes;
}

} // JPH
