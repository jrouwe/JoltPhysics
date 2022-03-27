// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "UnitTestFramework.h"
#include <Jolt/Core/Factory.h>
#include <Jolt/ObjectStream/ObjectStreamTextIn.h>
#include <Jolt/ObjectStream/ObjectStreamTextOut.h>
#include <Jolt/ObjectStream/SerializableObject.h>
#include <Jolt/ObjectStream/TypeDeclarations.h>

enum TestEnum
{
	A,
	B,
	C
};

class TestSerializableBase : public RefTarget<TestSerializableBase>
{
	JPH_DECLARE_SERIALIZABLE_VIRTUAL_BASE(TestSerializableBase)

public:
	virtual						~TestSerializableBase() = default;

	uint8						mUInt8 = 0;
	uint16						mUInt16 = 0;
	int							mInt = 0;
	uint32						mUInt32 = 0;
	uint64						mUInt64 = 0;
	float						mFloat = 0;
	bool						mBool = false;
	Quat						mQuat = Quat::sIdentity();
	Vec3						mVec3 = Vec3::sZero();
	Vec4						mVec4 = Vec4::sZero();
	Mat44						mMat44 = Mat44::sIdentity();
	string						mString;
};

class TestSerializable : public TestSerializableBase
{
	JPH_DECLARE_SERIALIZABLE_VIRTUAL(TestSerializable)

public:
	TestEnum					mEnum = A;
	vector<int>					mIntVector;
	StaticArray<bool, 10>		mBoolVector;
	float						mFloatVector[3] = { 0, 0, 0 };
	TestSerializable *			mPointer = nullptr;
	Ref<TestSerializable>		mReference;
	RefConst<TestSerializable>	mReferenceConst;
};

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(TestSerializableBase)
{
	JPH_ADD_ATTRIBUTE(TestSerializable, mUInt8)
	JPH_ADD_ATTRIBUTE(TestSerializable, mUInt16)
	JPH_ADD_ATTRIBUTE(TestSerializable, mInt)
	JPH_ADD_ATTRIBUTE(TestSerializable, mUInt32)
	JPH_ADD_ATTRIBUTE(TestSerializable, mUInt64)
	JPH_ADD_ATTRIBUTE(TestSerializable, mFloat)
	JPH_ADD_ATTRIBUTE(TestSerializable, mBool)
	JPH_ADD_ATTRIBUTE(TestSerializable, mQuat)
	JPH_ADD_ATTRIBUTE(TestSerializable, mVec3)
	JPH_ADD_ATTRIBUTE(TestSerializable, mVec4)
	JPH_ADD_ATTRIBUTE(TestSerializable, mMat44)
	JPH_ADD_ATTRIBUTE(TestSerializable, mString)
}

JPH_IMPLEMENT_SERIALIZABLE_VIRTUAL(TestSerializable)
{
	JPH_ADD_BASE_CLASS(TestSerializable, TestSerializableBase)

	JPH_ADD_ENUM_ATTRIBUTE(TestSerializable, mEnum)
	JPH_ADD_ATTRIBUTE(TestSerializable, mIntVector)
	JPH_ADD_ATTRIBUTE(TestSerializable, mBoolVector)
	JPH_ADD_ATTRIBUTE(TestSerializable, mFloatVector)
	JPH_ADD_ATTRIBUTE(TestSerializable, mPointer)
	JPH_ADD_ATTRIBUTE(TestSerializable, mReference)
	JPH_ADD_ATTRIBUTE(TestSerializable, mReferenceConst)
}

TEST_SUITE("ObjectStreamTest")
{
	static TestSerializable *CreateTestObject()
	{
		TestSerializable *test = new TestSerializable();
		test->mUInt8 = 0xff;
		test->mUInt16 = 0xffff;
		test->mInt = -1;
		test->mUInt32 = 0xf1f2f3f4;
		test->mUInt64 = 0xf5f6f7f8f9fafbfc;
		test->mFloat = 0.12345f;
		test->mBool = true;
		test->mVec3 = Vec3(6, 7, 8);
		test->mVec4 = Vec4(9, 10, 11, 12);
		test->mQuat = Quat::sRotation(Vec3::sAxisX(), 0.1234f);
		test->mMat44 = Mat44::sRotationTranslation(Quat::sRotation(Vec3::sAxisY(), 0.4567f), Vec3(13, 14, 15));
		test->mString = "\"test string\"";
		test->mEnum = B;
		test->mIntVector.push_back(1);
		test->mIntVector.push_back(2);
		test->mIntVector.push_back(3);
		test->mBoolVector.push_back(true);
		test->mBoolVector.push_back(false);
		test->mBoolVector.push_back(true);
		test->mFloatVector[0] = 1.0f;
		test->mFloatVector[1] = 2.0f;
		test->mFloatVector[2] = 3.0f;

		TestSerializable *test2 = new TestSerializable();
		test2->mFloat = 4.5f;
		test->mPointer = test2;
		test->mReference = test2;
		test->mReferenceConst = test2;

		return test;
	}

	static void CompareObjects(TestSerializable *inInput, TestSerializable *inOutput)
	{
		CHECK(inInput->mUInt8 == inOutput->mUInt8);
		CHECK(int(inInput->mUInt16) == int(inOutput->mUInt16));
		CHECK(inInput->mInt == inOutput->mInt);
		CHECK(inInput->mUInt32 == inOutput->mUInt32);
		CHECK(inInput->mUInt64 == inOutput->mUInt64);
		CHECK(inInput->mFloat == inOutput->mFloat);
		CHECK(inInput->mBool == inOutput->mBool);
		CHECK(inInput->mQuat == inOutput->mQuat);
		CHECK(inInput->mVec3 == inOutput->mVec3);
		CHECK(inInput->mVec4 == inOutput->mVec4);
		CHECK(inInput->mMat44 == inOutput->mMat44);
		CHECK(inInput->mString == inOutput->mString);
		CHECK(inInput->mEnum == inOutput->mEnum);

		CHECK(inInput->mIntVector.size() == inOutput->mIntVector.size());
		for (size_t i = 0; i < min(inInput->mIntVector.size(), inOutput->mIntVector.size()); ++i)
			CHECK(inInput->mIntVector[i] == inOutput->mIntVector[i]);

		CHECK(inInput->mBoolVector.size() == inOutput->mBoolVector.size());
		for (uint32 i = 0; i < min(inInput->mBoolVector.size(), inOutput->mBoolVector.size()); ++i)
			CHECK(inInput->mBoolVector[i] == inOutput->mBoolVector[i]);

		for (uint32 i = 0; i < size(inInput->mFloatVector); ++i)
			CHECK(inInput->mFloatVector[i] == inOutput->mFloatVector[i]);

		CHECK(inOutput->mPointer == inOutput->mReference);
		CHECK(inOutput->mPointer == inOutput->mReferenceConst);

		if (inInput->mPointer == nullptr)
			CHECK(inOutput->mPointer == nullptr);
		else
		{
			CHECK(inInput->mPointer != inOutput->mPointer);
			CompareObjects(inInput->mPointer, inOutput->mPointer);
			CHECK(inOutput->mReference->GetRefCount() == uint32(2));
			CHECK(inOutput->mReferenceConst->GetRefCount() == uint32(2));
		}
	}

	TEST_CASE("TestObjectStreamLoadSaveText")
	{
		Factory::sInstance.Register(JPH_RTTI(TestSerializable));

		TestSerializable *test = CreateTestObject();

		stringstream stream;
		REQUIRE(ObjectStreamOut::sWriteObject(stream, ObjectStreamOut::EStreamType::Text, *test));

		TestSerializable *test_out = nullptr;
		REQUIRE(ObjectStreamIn::sReadObject(stream, test_out));

		CompareObjects(test, test_out);

		delete test;
		delete test_out;
	}

	TEST_CASE("TestObjectStreamLoadSaveBinary")
	{
		Factory::sInstance.Register(JPH_RTTI(TestSerializable));

		TestSerializable *test = CreateTestObject();

		stringstream stream;
		REQUIRE(ObjectStreamOut::sWriteObject(stream, ObjectStreamOut::EStreamType::Binary, *test));

		TestSerializable *test_out = nullptr;
		REQUIRE(ObjectStreamIn::sReadObject(stream, test_out));
		
		CompareObjects(test, test_out);

		delete test;
		delete test_out;
	}
}