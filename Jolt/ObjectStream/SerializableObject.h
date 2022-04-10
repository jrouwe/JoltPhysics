// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/ObjectStream/SerializableAttribute.h>

JPH_NAMESPACE_BEGIN

//////////////////////////////////////////////////////////////////////////////////////////
// Helper macros
//////////////////////////////////////////////////////////////////////////////////////////

// JPH_DECLARE_SERIALIZATION_FUNCTIONS
#define JPH_DECLARE_SERIALIZATION_FUNCTIONS(prefix, class_name)														\
	prefix bool			OSReadData(ObjectStreamIn &ioStream, class_name &inInstance);								\
	prefix bool			OSReadData(ObjectStreamIn &ioStream, class_name *&inPointer);								\
	prefix bool			OSIsType(class_name *, int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName); \
	prefix bool			OSIsType(class_name **, int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName); \
	prefix void			OSWriteData(ObjectStreamOut &ioStream, const class_name &inInstance);						\
	prefix void			OSWriteData(ObjectStreamOut &ioStream, class_name *const &inPointer);						\
	prefix void			OSWriteDataType(ObjectStreamOut &ioStream, class_name *);									\
	prefix void			OSWriteDataType(ObjectStreamOut &ioStream, class_name **);									\
	prefix void			OSVisitCompounds(const class_name &inObject, const CompoundVisitor &inVisitor);				\
	prefix void			OSVisitCompounds(const class_name *inObject, const CompoundVisitor &inVisitor);

// JPH_IMPLEMENT_SERIALIZATION_FUNCTIONS
#define JPH_IMPLEMENT_SERIALIZATION_FUNCTIONS(class_name)															\
	bool				OSReadData(ObjectStreamIn &ioStream, class_name &inInstance)								\
	{																												\
		return ioStream.ReadClassData(#class_name, (void *)&inInstance);											\
	}																												\
	bool				OSReadData(ObjectStreamIn &ioStream, class_name *&inPointer)								\
	{																												\
		return ioStream.ReadPointerData(JPH_RTTI(class_name), (void **)&inPointer);									\
	}																												\
	bool				OSIsType(class_name *, int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName) \
	{																												\
		return inArrayDepth == 0 && inDataType == ObjectStream::EDataType::Instance && strcmp(inClassName, #class_name) == 0; \
	}																												\
	bool				OSIsType(class_name **, int inArrayDepth, ObjectStream::EDataType inDataType, const char *inClassName) \
	{																												\
		return inArrayDepth == 0 && inDataType == ObjectStream::EDataType::Pointer && strcmp(inClassName, #class_name) == 0; \
	}																												\
	void				OSWriteData(ObjectStreamOut &ioStream, const class_name &inInstance)						\
	{																												\
		ioStream.WriteClassData(JPH_RTTI(class_name), (void *)&inInstance);											\
	}																												\
	void				OSWriteData(ObjectStreamOut &ioStream, class_name *const &inPointer)						\
	{																												\
		if (inPointer)																								\
			ioStream.WritePointerData(GetRTTI(inPointer), (void *)inPointer);										\
		else 																										\
			ioStream.WritePointerData(nullptr, nullptr);															\
	}																												\
	void				OSWriteDataType(ObjectStreamOut &ioStream, class_name *)									\
	{																												\
		ioStream.WriteDataType(ObjectStream::EDataType::Instance);													\
		ioStream.WriteName(#class_name);																			\
	}																												\
	void				OSWriteDataType(ObjectStreamOut &ioStream, class_name **)									\
	{																												\
		ioStream.WriteDataType(ObjectStream::EDataType::Pointer);													\
		ioStream.WriteName(#class_name);																			\
	}																												\
	void				OSVisitCompounds(const class_name &inObject, const CompoundVisitor &inVisitor)				\
	{																												\
		OSVisitCompounds(&inObject, JPH_RTTI(class_name), inVisitor);												\
	}																												\
	void				OSVisitCompounds(const class_name *inObject, const CompoundVisitor &inVisitor)				\
	{																												\
		if (inObject != nullptr)																					\
			OSVisitCompounds(inObject, GetRTTI(inObject), inVisitor);												\
	}

//////////////////////////////////////////////////////////////////////////////////////////
// Use these macros on non-virtual objects to make them serializable
//////////////////////////////////////////////////////////////////////////////////////////

// JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL
#define JPH_DECLARE_SERIALIZABLE_NON_VIRTUAL(class_name)															\
public:																												\
	JPH_DECLARE_RTTI_NON_VIRTUAL(class_name)																		\
	JPH_DECLARE_SERIALIZATION_FUNCTIONS(friend, class_name)															\

// JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL
#define JPH_IMPLEMENT_SERIALIZABLE_NON_VIRTUAL(class_name)															\
	JPH_IMPLEMENT_SERIALIZATION_FUNCTIONS(class_name)																\
	JPH_IMPLEMENT_RTTI_NON_VIRTUAL(class_name)																		\

//////////////////////////////////////////////////////////////////////////////////////////
// Same as above, but when you cannot insert the declaration in the class itself
//////////////////////////////////////////////////////////////////////////////////////////

// JPH_DECLARE_SERIALIZABLE_OUTSIDE_CLASS
#define JPH_DECLARE_SERIALIZABLE_OUTSIDE_CLASS(class_name)															\
	JPH_DECLARE_RTTI_OUTSIDE_CLASS(class_name)																		\
	JPH_DECLARE_SERIALIZATION_FUNCTIONS(extern, class_name)															\

// JPH_IMPLEMENT_SERIALIZABLE_OUTSIDE_CLASS
#define JPH_IMPLEMENT_SERIALIZABLE_OUTSIDE_CLASS(class_name)														\
	JPH_IMPLEMENT_SERIALIZATION_FUNCTIONS(class_name)																\
	JPH_IMPLEMENT_RTTI_OUTSIDE_CLASS(class_name)																	\

//////////////////////////////////////////////////////////////////////////////////////////
// Same as above, but for classes that have virtual functions
//////////////////////////////////////////////////////////////////////////////////////////

// JPH_DECLARE_SERIALIZABLE_VIRTUAL - Use for concrete, non-base classes
#define JPH_DECLARE_SERIALIZABLE_VIRTUAL(class_name)																\
public:																												\
	JPH_DECLARE_RTTI_VIRTUAL(class_name)																			\
	JPH_DECLARE_SERIALIZATION_FUNCTIONS(friend, class_name)															\

// JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL
#define JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(class_name)																\
	JPH_IMPLEMENT_SERIALIZATION_FUNCTIONS(class_name)																\
	JPH_IMPLEMENT_RTTI_VIRTUAL(class_name)																			\

// JPH_DECLARE_SERIALIZABLE_ABSTRACT - Use for abstract, non-base classes
#define JPH_DECLARE_SERIALIZABLE_ABSTRACT(class_name)																\
public:																												\
	JPH_DECLARE_RTTI_ABSTRACT(class_name)																			\
	JPH_DECLARE_SERIALIZATION_FUNCTIONS(friend, class_name)															\

// JPH_IMPLEMENT_SERIALIZABLE_ABSTRACT
#define JPH_IMPLEMENT_SERIALIZABLE_ABSTRACT(class_name)																\
	JPH_IMPLEMENT_SERIALIZATION_FUNCTIONS(class_name)																\
	JPH_IMPLEMENT_RTTI_ABSTRACT(class_name)																			\

// JPH_DECLARE_SERIALIZABLE_VIRTUAL_BASE - Use for concrete base classes
#define JPH_DECLARE_SERIALIZABLE_VIRTUAL_BASE(class_name)															\
public:																												\
	JPH_DECLARE_RTTI_VIRTUAL_BASE(class_name)																		\
	JPH_DECLARE_SERIALIZATION_FUNCTIONS(friend, class_name)															\

// JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL_BASE
#define JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL_BASE(class_name)															\
	JPH_IMPLEMENT_SERIALIZATION_FUNCTIONS(class_name)																\
	JPH_IMPLEMENT_RTTI_VIRTUAL_BASE(class_name)																		\

// JPH_DECLARE_SERIALIZABLE_ABSTRACT_BASE - Use for abstract base class
#define JPH_DECLARE_SERIALIZABLE_ABSTRACT_BASE(class_name)															\
public:																												\
	JPH_DECLARE_RTTI_ABSTRACT_BASE(class_name)																		\
	JPH_DECLARE_SERIALIZATION_FUNCTIONS(friend, class_name)															\

// JPH_IMPLEMENT_SERIALIZABLE_ABSTRACT_BASE
#define JPH_IMPLEMENT_SERIALIZABLE_ABSTRACT_BASE(class_name)														\
	JPH_IMPLEMENT_SERIALIZATION_FUNCTIONS(class_name)																\
	JPH_IMPLEMENT_RTTI_ABSTRACT_BASE(class_name)																	\

//////////////////////////////////////////////////////////////////////////////////////////
// OSVisitCompounds
//
// Allows recursively visiting an object and all of its children depth first
//////////////////////////////////////////////////////////////////////////////////////////

/// Basic visitor that recurses to all attributes
void OSVisitCompounds(const void *inObject, const RTTI *inRTTI, const CompoundVisitor &inVisitor);

// Dummy implementation for OSVisitCompounds for all primitive types
#define JPH_DECLARE_PRIMITIVE(name)																					\
	inline void OSVisitCompounds(const name &inObject, const CompoundVisitor &inVisitor) { }

// This file uses the JPH_DECLARE_PRIMITIVE macro to define all types
#include <Jolt/ObjectStream/ObjectStreamTypes.h>

/// Define visitor templates
template <class T>
void OSVisitCompounds(const Ref<T> &inObject, const CompoundVisitor &inVisitor)	
{
	if (inObject != nullptr)
		OSVisitCompounds(inObject.GetPtr(), inVisitor);
}

template <class T>
void OSVisitCompounds(const RefConst<T> &inObject, const CompoundVisitor &inVisitor)	
{
	if (inObject != nullptr)
		OSVisitCompounds(inObject.GetPtr(), inVisitor);
}

template <class T>
void OSVisitCompounds(const vector<T> &inObject, const CompoundVisitor &inVisitor)	
{ 
	for (const T &v : inObject)
		OSVisitCompounds(v, inVisitor);
}

template <class T, uint N>
void OSVisitCompounds(const StaticArray<T, N> &inObject, const CompoundVisitor &inVisitor)	
{ 
	for (const T &v : inObject)
		OSVisitCompounds(v, inVisitor);
}

template <class T, uint N>
void OSVisitCompounds(const T (&inObject)[N], const CompoundVisitor &inVisitor)	
{ 
	for (const T &v : inObject)
		OSVisitCompounds(v, inVisitor);
}

/// Classes must be derived from SerializableObject if you want to be able to save pointers or
/// reference counting pointers to objects of this or derived classes. The type will automatically
/// be determined during serialization and upon deserialization it will be restored correctly.
class SerializableObject
{
	JPH_DECLARE_SERIALIZABLE_ABSTRACT_BASE(SerializableObject)

public:
	/// Constructor
	virtual						~SerializableObject() = default;

	/// Callback given when object has been loaded from an object stream
	/// This is called when all links have been resolved. Objects that this object point to have already received their OnLoaded callback.
	virtual void				OnLoaded()																			{ /* Do nothing */ }
};

JPH_NAMESPACE_END
