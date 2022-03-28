// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#include <Jolt/ObjectStream/SerializableObject.h>
#include <Jolt/Core/Color.h>
#include <Jolt/Geometry/AABox.h>
#include <Jolt/Geometry/Triangle.h>
#include <Jolt/Geometry/IndexedTriangle.h>

JPH_NAMESPACE_BEGIN

JPH_DECLARE_RTTI_OUTSIDE_CLASS(uint8);
JPH_DECLARE_RTTI_OUTSIDE_CLASS(uint16);
JPH_DECLARE_RTTI_OUTSIDE_CLASS(int);
JPH_DECLARE_RTTI_OUTSIDE_CLASS(uint32);
JPH_DECLARE_RTTI_OUTSIDE_CLASS(uint64);
JPH_DECLARE_RTTI_OUTSIDE_CLASS(float);
JPH_DECLARE_RTTI_OUTSIDE_CLASS(bool);
JPH_DECLARE_RTTI_OUTSIDE_CLASS(string);
JPH_DECLARE_RTTI_OUTSIDE_CLASS(Float3);
JPH_DECLARE_RTTI_OUTSIDE_CLASS(Vec3);
JPH_DECLARE_RTTI_OUTSIDE_CLASS(Vec4);
JPH_DECLARE_RTTI_OUTSIDE_CLASS(Quat);
JPH_DECLARE_RTTI_OUTSIDE_CLASS(Mat44);
JPH_DECLARE_SERIALIZABLE_OUTSIDE_CLASS(Color);
JPH_DECLARE_SERIALIZABLE_OUTSIDE_CLASS(AABox);
JPH_DECLARE_SERIALIZABLE_OUTSIDE_CLASS(Triangle);
JPH_DECLARE_SERIALIZABLE_OUTSIDE_CLASS(IndexedTriangle);

JPH_NAMESPACE_END

// These need to be added after all types have been registered or else clang under linux will not find GetRTTIOfType for the type
#include <Jolt/ObjectStream/SerializableAttributeTyped.h>
#include <Jolt/ObjectStream/SerializableAttributeEnum.h>
