// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

/// Emulates HLSL vector types and operations in C++.
/// Note doesn't emulate things like barriers and group shared memory.
namespace HLSLToCPP {

using std::sqrt;
using std::min;
using std::max;
using std::round;

//////////////////////////////////////////////////////////////////////////////////////////
// float2
//////////////////////////////////////////////////////////////////////////////////////////

struct float2
{
	// Constructors
	inline				float2() = default;
	constexpr			float2(float inX, float inY)						: x(inX), y(inY) { }
	explicit constexpr	float2(float inS)									: x(inS), y(inS) { }

	// Operators
	constexpr float2 &	operator += (const float2 &inRHS)					{ x += inRHS.x; y += inRHS.y; return *this; }
	constexpr float2 &	operator -= (const float2 &inRHS)					{ x -= inRHS.x; y -= inRHS.y; return *this; }
	constexpr float2 &	operator *= (float inRHS)							{ x *= inRHS; y *= inRHS; return *this; }
	constexpr float2 &	operator /= (float inRHS)							{ x /= inRHS; y /= inRHS; return *this; }
	constexpr float2 &	operator *= (const float2 &inRHS)					{ x *= inRHS.x; y *= inRHS.y; return *this; }
	constexpr float2 &	operator /= (const float2 &inRHS)					{ x /= inRHS.x; y /= inRHS.y; return *this; }

	// Equality
	constexpr bool		operator == (const float2 &inRHS) const				{ return x == inRHS.x && y == inRHS.y; }
	constexpr bool		operator != (const float2 &inRHS) const				{ return !(*this == inRHS); }

	// Component access
	const float &		operator [] (uint inIndex) const					{ return (&x)[inIndex]; }
	float &				operator [] (uint inIndex)							{ return (&x)[inIndex]; }

	// Swizzling (note return value is const to prevent assignment to swizzled results)
	const float2		swizzle_xy() const									{ return float2(x, y); }
	const float2		swizzle_yx() const									{ return float2(y, x); }

	float				x, y;
};

// Operators
constexpr float2		operator - (const float2 &inA)						{ return float2(-inA.x, -inA.y); }
constexpr float2		operator + (const float2 &inA, const float2 &inB)	{ return float2(inA.x + inB.x, inA.y + inB.y); }
constexpr float2		operator - (const float2 &inA, const float2 &inB)	{ return float2(inA.x - inB.x, inA.y - inB.y); }
constexpr float2		operator * (const float2 &inA, const float2 &inB)	{ return float2(inA.x * inB.x, inA.y * inB.y); }
constexpr float2		operator / (const float2 &inA, const float2 &inB)	{ return float2(inA.x / inB.x, inA.y / inB.y); }
constexpr float2		operator * (const float2 &inA, float inS)			{ return float2(inA.x * inS, inA.y * inS); }
constexpr float2		operator * (float inS, const float2 &inA)			{ return inA * inS; }
constexpr float2		operator / (const float2 &inA, float inS)			{ return float2(inA.x / inS, inA.y / inS); }

// Dot product
constexpr float			dot(const float2 &inA, const float2 &inB)			{ return inA.x * inB.x + inA.y * inB.y; }

// Min value
constexpr float2		min(const float2 &inA, const float2 &inB)			{ return float2(min(inA.x, inB.x), min(inA.y, inB.y)); }

// Max value
constexpr float2		max(const float2 &inA, const float2 &inB)			{ return float2(max(inA.x, inB.x), max(inA.y, inB.y)); }

// Length
inline float			length(const float2 &inV)							{ return sqrt(dot(inV, inV)); }

// Normalization
inline float2			normalize(const float2 &inV)						{ return inV / length(inV); }

// Rounding to int
inline float2			round(const float2 &inV)							{ return float2(round(inV.x), round(inV.y)); }

//////////////////////////////////////////////////////////////////////////////////////////
// float3
//////////////////////////////////////////////////////////////////////////////////////////

struct uint3;

struct float3
{
	// Constructors
	inline				float3() = default;
	constexpr			float3(const float2 &inV, float inZ)				: x(inV.x), y(inV.y), z(inZ) { }
	constexpr			float3(float inX, float inY, float inZ)				: x(inX), y(inY), z(inZ) { }
	explicit constexpr	float3(float inS)									: x(inS), y(inS), z(inS) { }
	explicit constexpr	float3(const uint3 &inV);

	// Operators
	constexpr float3 &	operator += (const float3 &inRHS)					{ x += inRHS.x; y += inRHS.y; z += inRHS.z; return *this; }
	constexpr float3 &	operator -= (const float3 &inRHS)					{ x -= inRHS.x; y -= inRHS.y; z -= inRHS.z; return *this; }
	constexpr float3 &	operator *= (float inRHS)							{ x *= inRHS; y *= inRHS; z *= inRHS; return *this; }
	constexpr float3 &	operator /= (float inRHS)							{ x /= inRHS; y /= inRHS; z /= inRHS; return *this; }
	constexpr float3 &	operator *= (const float3 &inRHS)					{ x *= inRHS.x; y *= inRHS.y; z *= inRHS.z; return *this; }
	constexpr float3 &	operator /= (const float3 &inRHS)					{ x /= inRHS.x; y /= inRHS.y; z /= inRHS.z; return *this; }

	// Equality
	constexpr bool		operator == (const float3 &inRHS) const				{ return x == inRHS.x && y == inRHS.y && z == inRHS.z; }
	constexpr bool		operator != (const float3 &inRHS) const				{ return !(*this == inRHS); }

	// Component access
	const float &		operator [] (uint inIndex) const					{ return (&x)[inIndex]; }
	float &				operator [] (uint inIndex)							{ return (&x)[inIndex]; }

	// Swizzling (note return value is const to prevent assignment to swizzled results)
	const float2		swizzle_xy() const									{ return float2(x, y); }
	const float2		swizzle_yx() const									{ return float2(y, x); }
	const float3		swizzle_xyz() const									{ return float3(x, y, z); }
	const float3		swizzle_xzy() const									{ return float3(x, z, y); }
	const float3		swizzle_yxz() const									{ return float3(y, x, z); }
	const float3		swizzle_yzx() const									{ return float3(y, z, x); }
	const float3		swizzle_zxy() const									{ return float3(z, x, y); }
	const float3		swizzle_zyx() const									{ return float3(z, y, x); }

	float				x, y, z;
};

// Operators
constexpr float3		operator - (const float3 &inA)						{ return float3(-inA.x, -inA.y, -inA.z); }
constexpr float3		operator + (const float3 &inA, const float3 &inB)	{ return float3(inA.x + inB.x, inA.y + inB.y, inA.z + inB.z); }
constexpr float3		operator - (const float3 &inA, const float3 &inB)	{ return float3(inA.x - inB.x, inA.y - inB.y, inA.z - inB.z); }
constexpr float3		operator * (const float3 &inA, const float3 &inB)	{ return float3(inA.x * inB.x, inA.y * inB.y, inA.z * inB.z); }
constexpr float3		operator / (const float3 &inA, const float3 &inB)	{ return float3(inA.x / inB.x, inA.y / inB.y, inA.z / inB.z); }
constexpr float3		operator * (const float3 &inA, float inS)			{ return float3(inA.x * inS, inA.y * inS, inA.z * inS); }
constexpr float3		operator * (float inS, const float3 &inA)			{ return inA * inS; }
constexpr float3		operator / (const float3 &inA, float inS)			{ return float3(inA.x / inS, inA.y / inS, inA.z / inS); }

// Dot product
constexpr float			dot(const float3 &inA, const float3 &inB)			{ return inA.x * inB.x + inA.y * inB.y + inA.z * inB.z; }

// Min value
constexpr float3		min(const float3 &inA, const float3 &inB)			{ return float3(min(inA.x, inB.x), min(inA.y, inB.y), min(inA.z, inB.z)); }

// Max value
constexpr float3		max(const float3 &inA, const float3 &inB)			{ return float3(max(inA.x, inB.x), max(inA.y, inB.y), max(inA.z, inB.z)); }

// Length
inline float			length(const float3 &inV)							{ return sqrt(dot(inV, inV)); }

// Normalization
inline float3			normalize(const float3 &inV)						{ return inV / length(inV); }

// Rounding to int
inline float3			round(const float3 &inV)							{ return float3(round(inV.x), round(inV.y), round(inV.z)); }

// Cross product
constexpr float3		cross(const float3 &inA, const float3 &inB)			{ return float3(inA.y * inB.z - inA.z * inB.y, inA.z * inB.x - inA.x * inB.z, inA.x * inB.y - inA.y * inB.x); }

//////////////////////////////////////////////////////////////////////////////////////////
// float4
//////////////////////////////////////////////////////////////////////////////////////////

struct int4;

struct float4
{
	// Constructors
	inline				float4() = default;
	constexpr			float4(const float3 &inV, float inW)				: x(inV.x), y(inV.y), z(inV.z), w(inW) { }
	constexpr			float4(float inX, float inY, float inZ, float inW)	: x(inX), y(inY), z(inZ), w(inW) { }
	explicit constexpr	float4(float inS)									: x(inS), y(inS), z(inS), w(inS) { }
	explicit constexpr	float4(const int4 &inV);

	// Operators
	constexpr float4 &	operator += (const float4 &inRHS)					{ x += inRHS.x; y += inRHS.y; z += inRHS.z; w += inRHS.w; return *this; }
	constexpr float4 &	operator -= (const float4 &inRHS)					{ x -= inRHS.x; y -= inRHS.y; z -= inRHS.z; w -= inRHS.w; return *this; }
	constexpr float4 &	operator *= (float inRHS)							{ x *= inRHS; y *= inRHS; z *= inRHS; w *= inRHS; return *this; }
	constexpr float4 &	operator /= (float inRHS)							{ x /= inRHS; y /= inRHS; z /= inRHS; w /= inRHS; return *this; }
	constexpr float4 &	operator *= (const float4 &inRHS)					{ x *= inRHS.x; y *= inRHS.y; z *= inRHS.z; w *= inRHS.w; return *this; }
	constexpr float4 &	operator /= (const float4 &inRHS)					{ x /= inRHS.x; y /= inRHS.y; z /= inRHS.z; w /= inRHS.w; return *this; }

	// Equality
	constexpr bool		operator == (const float4 &inRHS) const				{ return x == inRHS.x && y == inRHS.y && z == inRHS.z && w == inRHS.w; }
	constexpr bool		operator != (const float4 &inRHS) const				{ return !(*this == inRHS); }

	// Component access
	const float &		operator [] (uint inIndex) const					{ return (&x)[inIndex]; }
	float &				operator [] (uint inIndex)							{ return (&x)[inIndex]; }

	// Swizzling (note return value is const to prevent assignment to swizzled results)
	const float2		swizzle_xy() const									{ return float2(x, y); }
	const float2		swizzle_yx() const									{ return float2(y, x); }
	const float3		swizzle_xyz() const									{ return float3(x, y, z); }
	const float3		swizzle_xzy() const									{ return float3(x, z, y); }
	const float3		swizzle_yxz() const									{ return float3(y, x, z); }
	const float3		swizzle_yzx() const									{ return float3(y, z, x); }
	const float3		swizzle_zxy() const									{ return float3(z, x, y); }
	const float3		swizzle_zyx() const									{ return float3(z, y, x); }
	const float4		swizzle_xywz() const								{ return float4(x, y, w, z); }
	const float4		swizzle_xwyz() const								{ return float4(x, w, y, z); }
	const float4		swizzle_wxyz() const								{ return float4(w, x, y, z); }

	float				x, y, z, w;
};

// Operators
constexpr float4		operator - (const float4 &inA)						{ return float4(-inA.x, -inA.y, -inA.z, -inA.w); }
constexpr float4		operator + (const float4 &inA, const float4 &inB)	{ return float4(inA.x + inB.x, inA.y + inB.y, inA.z + inB.z, inA.w + inB.w); }
constexpr float4		operator - (const float4 &inA, const float4 &inB)	{ return float4(inA.x - inB.x, inA.y - inB.y, inA.z - inB.z, inA.w - inB.w); }
constexpr float4		operator * (const float4 &inA, const float4 &inB)	{ return float4(inA.x * inB.x, inA.y * inB.y, inA.z * inB.z, inA.w * inB.w); }
constexpr float4		operator / (const float4 &inA, const float4 &inB)	{ return float4(inA.x / inB.x, inA.y / inB.y, inA.z / inB.z, inA.w / inB.w); }
constexpr float4		operator * (const float4 &inA, float inS)			{ return float4(inA.x * inS, inA.y * inS, inA.z * inS, inA.w * inS); }
constexpr float4		operator * (float inS, const float4 &inA)			{ return inA * inS; }
constexpr float4		operator / (const float4 &inA, float inS)			{ return float4(inA.x / inS, inA.y / inS, inA.z / inS, inA.w / inS); }

// Dot product
constexpr float			dot(const float4 &inA, const float4 &inB)			{ return inA.x * inB.x + inA.y * inB.y + inA.z * inB.z + inA.w * inB.w; }

// Min value
constexpr float4		min(const float4 &inA, const float4 &inB)			{ return float4(min(inA.x, inB.x), min(inA.y, inB.y), min(inA.z, inB.z), min(inA.w, inB.w)); }

// Max value
constexpr float4		max(const float4 &inA, const float4 &inB)			{ return float4(max(inA.x, inB.x), max(inA.y, inB.y), max(inA.z, inB.z), max(inA.w, inB.w)); }

// Length
inline float			length(const float4 &inV)							{ return sqrt(dot(inV, inV)); }

// Normalization
inline float4			normalize(const float4 &inV)						{ return inV / length(inV); }

// Rounding to int
inline float4			round(const float4 &inV)							{ return float4(round(inV.x), round(inV.y), round(inV.z), round(inV.w)); }

//////////////////////////////////////////////////////////////////////////////////////////
// uint3
//////////////////////////////////////////////////////////////////////////////////////////

struct uint3
{
	inline				uint3() = default;
	constexpr			uint3(uint32 inX, uint32 inY, uint32 inZ)			: x(inX), y(inY), z(inZ) { }
	explicit constexpr	uint3(const float3 &inV)							: x(uint32(inV.x)), y(uint32(inV.y)), z(uint32(inV.z)) { }

	// Operators
	constexpr uint3 &	operator += (const uint3 &inRHS)					{ x += inRHS.x; y += inRHS.y; z += inRHS.z; return *this; }
	constexpr uint3 &	operator -= (const uint3 &inRHS)					{ x -= inRHS.x; y -= inRHS.y; z -= inRHS.z; return *this; }
	constexpr uint3 &	operator *= (uint32 inRHS)							{ x *= inRHS; y *= inRHS; z *= inRHS; return *this; }
	constexpr uint3 &	operator /= (uint32 inRHS)							{ x /= inRHS; y /= inRHS; z /= inRHS; return *this; }
	constexpr uint3 &	operator *= (const uint3 &inRHS)					{ x *= inRHS.x; y *= inRHS.y; z *= inRHS.z; return *this; }
	constexpr uint3 &	operator /= (const uint3 &inRHS)					{ x /= inRHS.x; y /= inRHS.y; z /= inRHS.z; return *this; }

	// Equality
	constexpr bool		operator == (const uint3 &inRHS) const				{ return x == inRHS.x && y == inRHS.y && z == inRHS.z; }
	constexpr bool		operator != (const uint3 &inRHS) const				{ return !(*this == inRHS); }

	// Component access
	const uint32 &		operator [] (uint inIndex) const					{ return (&x)[inIndex]; }
	uint32 &			operator [] (uint inIndex)							{ return (&x)[inIndex]; }

	// Swizzling (note return value is const to prevent assignment to swizzled results)
	const uint3			swizzle_xyz() const									{ return uint3(x, y, z); }
	const uint3			swizzle_xzy() const									{ return uint3(x, z, y); }
	const uint3			swizzle_yxz() const									{ return uint3(y, x, z); }
	const uint3			swizzle_yzx() const									{ return uint3(y, z, x); }
	const uint3			swizzle_zxy() const									{ return uint3(z, x, y); }
	const uint3			swizzle_zyx() const									{ return uint3(z, y, x); }

	uint32				x, y, z;
};

// Operators
constexpr uint3			operator + (const uint3 &inA, const uint3 &inB)		{ return uint3(inA.x + inB.x, inA.y + inB.y, inA.z + inB.z); }
constexpr uint3			operator - (const uint3 &inA, const uint3 &inB)		{ return uint3(inA.x - inB.x, inA.y - inB.y, inA.z - inB.z); }
constexpr uint3			operator * (const uint3 &inA, const uint3 &inB)		{ return uint3(inA.x * inB.x, inA.y * inB.y, inA.z * inB.z); }
constexpr uint3			operator / (const uint3 &inA, const uint3 &inB)		{ return uint3(inA.x / inB.x, inA.y / inB.y, inA.z / inB.z); }
constexpr uint3			operator * (const uint3 &inA, uint32 inS)			{ return uint3(inA.x * inS, inA.y * inS, inA.z * inS); }
constexpr uint3			operator * (uint32 inS, const uint3 &inA)			{ return inA * inS; }
constexpr uint3			operator / (const uint3 &inA, uint32 inS)			{ return uint3(inA.x / inS, inA.y / inS, inA.z / inS); }

// Dot product
constexpr uint32		dot(const uint3 &inA, const uint3 &inB)				{ return inA.x * inB.x + inA.y * inB.y + inA.z * inB.z; }

// Min value
constexpr uint3			min(const uint3 &inA, const uint3 &inB)				{ return uint3(min(inA.x, inB.x), min(inA.y, inB.y), min(inA.z, inB.z)); }

// Max value
constexpr uint3			max(const uint3 &inA, const uint3 &inB)				{ return uint3(max(inA.x, inB.x), max(inA.y, inB.y), max(inA.z, inB.z)); }

//////////////////////////////////////////////////////////////////////////////////////////
// uint4
//////////////////////////////////////////////////////////////////////////////////////////

struct uint4
{
	// Constructors
	inline				uint4() = default;
	constexpr			uint4(const uint3 &inV, uint32 inW)					: x(inV.x), y(inV.y), z(inV.z), w(inW) { }
	constexpr			uint4(uint32 inX, uint32 inY, uint32 inZ, uint32 inW) : x(inX), y(inY), z(inZ), w(inW) { }
	explicit constexpr	uint4(uint32 inS)									: x(inS), y(inS), z(inS), w(inS) { }

	// Operators
	constexpr uint4 &	operator += (const uint4 &inRHS)					{ x += inRHS.x; y += inRHS.y; z += inRHS.z; w += inRHS.w; return *this; }
	constexpr uint4 &	operator -= (const uint4 &inRHS)					{ x -= inRHS.x; y -= inRHS.y; z -= inRHS.z; w -= inRHS.w; return *this; }
	constexpr uint4 &	operator *= (uint32 inRHS)							{ x *= inRHS; y *= inRHS; z *= inRHS; w *= inRHS; return *this; }
	constexpr uint4 &	operator /= (uint32 inRHS)							{ x /= inRHS; y /= inRHS; z /= inRHS; w /= inRHS; return *this; }
	constexpr uint4 &	operator *= (const uint4 &inRHS)					{ x *= inRHS.x; y *= inRHS.y; z *= inRHS.z; w *= inRHS.w; return *this; }
	constexpr uint4 &	operator /= (const uint4 &inRHS)					{ x /= inRHS.x; y /= inRHS.y; z /= inRHS.z; w /= inRHS.w; return *this; }

	// Equality
	constexpr bool		operator == (const uint4 &inRHS) const				{ return x == inRHS.x && y == inRHS.y && z == inRHS.z && w == inRHS.w; }
	constexpr bool		operator != (const uint4 &inRHS) const				{ return !(*this == inRHS); }

	// Component access
	const uint32 &		operator [] (uint inIndex) const					{ return (&x)[inIndex]; }
	uint32 &			operator [] (uint inIndex)							{ return (&x)[inIndex]; }

	// Swizzling (note return value is const to prevent assignment to swizzled results)
	const uint3			swizzle_xyz() const									{ return uint3(x, y, z); }
	const uint3			swizzle_xzy() const									{ return uint3(x, z, y); }
	const uint3			swizzle_yxz() const									{ return uint3(y, x, z); }
	const uint3			swizzle_yzx() const									{ return uint3(y, z, x); }
	const uint3			swizzle_zxy() const									{ return uint3(z, x, y); }
	const uint3			swizzle_zyx() const									{ return uint3(z, y, x); }
	const uint4			swizzle_xywz() const								{ return uint4(x, y, w, z); }
	const uint4			swizzle_xwyz() const								{ return uint4(x, w, y, z); }
	const uint4			swizzle_wxyz() const								{ return uint4(w, x, y, z); }

	uint32				x, y, z, w;
};

// Operators
constexpr uint4			operator + (const uint4 &inA, const uint4 &inB)		{ return uint4(inA.x + inB.x, inA.y + inB.y, inA.z + inB.z, inA.w + inB.w); }
constexpr uint4			operator - (const uint4 &inA, const uint4 &inB)		{ return uint4(inA.x - inB.x, inA.y - inB.y, inA.z - inB.z, inA.w - inB.w); }
constexpr uint4			operator * (const uint4 &inA, const uint4 &inB)		{ return uint4(inA.x * inB.x, inA.y * inB.y, inA.z * inB.z, inA.w * inB.w); }
constexpr uint4			operator / (const uint4 &inA, const uint4 &inB)		{ return uint4(inA.x / inB.x, inA.y / inB.y, inA.z / inB.z, inA.w / inB.w); }
constexpr uint4			operator * (const uint4 &inA, uint32 inS)			{ return uint4(inA.x * inS, inA.y * inS, inA.z * inS, inA.w * inS); }
constexpr uint4			operator * (uint32 inS, const uint4 &inA)			{ return inA * inS; }
constexpr uint4			operator / (const uint4 &inA, uint32 inS)			{ return uint4(inA.x / inS, inA.y / inS, inA.z / inS, inA.w / inS); }

// Dot product
constexpr uint32		dot(const uint4 &inA, const uint4 &inB)				{ return inA.x * inB.x + inA.y * inB.y + inA.z * inB.z + inA.w * inB.w; }

// Min value
constexpr uint4			min(const uint4 &inA, const uint4 &inB)				{ return uint4(min(inA.x, inB.x), min(inA.y, inB.y), min(inA.z, inB.z), min(inA.w, inB.w)); }

// Max value
constexpr uint4			max(const uint4 &inA, const uint4 &inB)				{ return uint4(max(inA.x, inB.x), max(inA.y, inB.y), max(inA.z, inB.z), max(inA.w, inB.w)); }

//////////////////////////////////////////////////////////////////////////////////////////
// int3
//////////////////////////////////////////////////////////////////////////////////////////

struct int3
{
	inline				int3() = default;
	constexpr			int3(int inX, int inY, int inZ)						: x(inX), y(inY), z(inZ) { }
	explicit constexpr	int3(const float3 &inV)								: x(int(inV.x)), y(int(inV.y)), z(int(inV.z)) { }

	// Operators
	constexpr int3 &	operator += (const int3 &inRHS)						{ x += inRHS.x; y += inRHS.y; z += inRHS.z; return *this; }
	constexpr int3 &	operator -= (const int3 &inRHS)						{ x -= inRHS.x; y -= inRHS.y; z -= inRHS.z; return *this; }
	constexpr int3 &	operator *= (int inRHS)								{ x *= inRHS; y *= inRHS; z *= inRHS; return *this; }
	constexpr int3 &	operator /= (int inRHS)								{ x /= inRHS; y /= inRHS; z /= inRHS; return *this; }
	constexpr int3 &	operator *= (const int3 &inRHS)						{ x *= inRHS.x; y *= inRHS.y; z *= inRHS.z; return *this; }
	constexpr int3 &	operator /= (const int3 &inRHS)						{ x /= inRHS.x; y /= inRHS.y; z /= inRHS.z; return *this; }

	// Equality
	constexpr bool		operator == (const int3 &inRHS) const				{ return x == inRHS.x && y == inRHS.y && z == inRHS.z; }
	constexpr bool		operator != (const int3 &inRHS) const				{ return !(*this == inRHS); }

	// Component access
	const int &			operator [] (uint inIndex) const					{ return (&x)[inIndex]; }
	int &				operator [] (uint inIndex)							{ return (&x)[inIndex]; }

	// Swizzling (note return value is const to prevent assignment to swizzled results)
	const int3			swizzle_xyz() const									{ return int3(x, y, z); }
	const int3			swizzle_xzy() const									{ return int3(x, z, y); }
	const int3			swizzle_yxz() const									{ return int3(y, x, z); }
	const int3			swizzle_yzx() const									{ return int3(y, z, x); }
	const int3			swizzle_zxy() const									{ return int3(z, x, y); }
	const int3			swizzle_zyx() const									{ return int3(z, y, x); }

	int					x, y, z;
};

// Operators
constexpr int3			operator - (const int3 &inA)						{ return int3(-inA.x, -inA.y, -inA.z); }
constexpr int3			operator + (const int3 &inA, const int3 &inB)		{ return int3(inA.x + inB.x, inA.y + inB.y, inA.z + inB.z); }
constexpr int3			operator - (const int3 &inA, const int3 &inB)		{ return int3(inA.x - inB.x, inA.y - inB.y, inA.z - inB.z); }
constexpr int3			operator * (const int3 &inA, const int3 &inB)		{ return int3(inA.x * inB.x, inA.y * inB.y, inA.z * inB.z); }
constexpr int3			operator / (const int3 &inA, const int3 &inB)		{ return int3(inA.x / inB.x, inA.y / inB.y, inA.z / inB.z); }
constexpr int3			operator * (const int3 &inA, int inS)				{ return int3(inA.x * inS, inA.y * inS, inA.z * inS); }
constexpr int3			operator * (int inS, const int3 &inA)				{ return inA * inS; }
constexpr int3			operator / (const int3 &inA, int inS)				{ return int3(inA.x / inS, inA.y / inS, inA.z / inS); }

// Dot product
constexpr int			dot(const int3 &inA, const int3 &inB)				{ return inA.x * inB.x + inA.y * inB.y + inA.z * inB.z; }

// Min value
constexpr int3			min(const int3 &inA, const int3 &inB)				{ return int3(min(inA.x, inB.x), min(inA.y, inB.y), min(inA.z, inB.z)); }

// Max value
constexpr int3			max(const int3 &inA, const int3 &inB)				{ return int3(max(inA.x, inB.x), max(inA.y, inB.y), max(inA.z, inB.z)); }

//////////////////////////////////////////////////////////////////////////////////////////
// int4
//////////////////////////////////////////////////////////////////////////////////////////

struct int4
{
	// Constructors
	inline				int4() = default;
	constexpr			int4(const int3 &inV, int inW)						: x(inV.x), y(inV.y), z(inV.z), w(inW) { }
	constexpr			int4(int inX, int inY, int inZ, int inW)			: x(inX), y(inY), z(inZ), w(inW) { }
	explicit constexpr	int4(int inS)										: x(inS), y(inS), z(inS), w(inS) { }
	explicit constexpr	int4(const float4 &inV)								: x(int(inV.x)), y(int(inV.y)), z(int(inV.z)), w(int(inV.w)) { }

	// Operators
	constexpr int4 &	operator += (const int4 &inRHS)						{ x += inRHS.x; y += inRHS.y; z += inRHS.z; w += inRHS.w; return *this; }
	constexpr int4 &	operator -= (const int4 &inRHS)						{ x -= inRHS.x; y -= inRHS.y; z -= inRHS.z; w -= inRHS.w; return *this; }
	constexpr int4 &	operator *= (int inRHS)								{ x *= inRHS; y *= inRHS; z *= inRHS; w *= inRHS; return *this; }
	constexpr int4 &	operator /= (int inRHS)								{ x /= inRHS; y /= inRHS; z /= inRHS; w /= inRHS; return *this; }
	constexpr int4 &	operator *= (const int4 &inRHS)						{ x *= inRHS.x; y *= inRHS.y; z *= inRHS.z; w *= inRHS.w; return *this; }
	constexpr int4 &	operator /= (const int4 &inRHS)						{ x /= inRHS.x; y /= inRHS.y; z /= inRHS.z; w /= inRHS.w; return *this; }

	// Equality
	constexpr bool		operator == (const int4 &inRHS) const				{ return x == inRHS.x && y == inRHS.y && z == inRHS.z && w == inRHS.w; }
	constexpr bool		operator != (const int4 &inRHS) const				{ return !(*this == inRHS); }

	// Component access
	const int &			operator [] (uint inIndex) const					{ return (&x)[inIndex]; }
	int &				operator [] (uint inIndex)							{ return (&x)[inIndex]; }

	// Swizzling (note return value is const to prevent assignment to swizzled results)
	const int3			swizzle_xyz() const									{ return int3(x, y, z); }
	const int3			swizzle_xzy() const									{ return int3(x, z, y); }
	const int3			swizzle_yxz() const									{ return int3(y, x, z); }
	const int3			swizzle_yzx() const									{ return int3(y, z, x); }
	const int3			swizzle_zxy() const									{ return int3(z, x, y); }
	const int3			swizzle_zyx() const									{ return int3(z, y, x); }
	const int4			swizzle_xywz() const								{ return int4(x, y, w, z); }
	const int4			swizzle_xwyz() const								{ return int4(x, w, y, z); }
	const int4			swizzle_wxyz() const								{ return int4(w, x, y, z); }

	int					x, y, z, w;
};

// Operators
constexpr int4			operator - (const int4 &inA)						{ return int4(-inA.x, -inA.y, -inA.z, -inA.w); }
constexpr int4			operator + (const int4 &inA, const int4 &inB)		{ return int4(inA.x + inB.x, inA.y + inB.y, inA.z + inB.z, inA.w + inB.w); }
constexpr int4			operator - (const int4 &inA, const int4 &inB)		{ return int4(inA.x - inB.x, inA.y - inB.y, inA.z - inB.z, inA.w - inB.w); }
constexpr int4			operator * (const int4 &inA, const int4 &inB)		{ return int4(inA.x * inB.x, inA.y * inB.y, inA.z * inB.z, inA.w * inB.w); }
constexpr int4			operator / (const int4 &inA, const int4 &inB)		{ return int4(inA.x / inB.x, inA.y / inB.y, inA.z / inB.z, inA.w / inB.w); }
constexpr int4			operator * (const int4 &inA, int inS)				{ return int4(inA.x * inS, inA.y * inS, inA.z * inS, inA.w * inS); }
constexpr int4			operator * (int inS, const int4 &inA)				{ return inA * inS; }
constexpr int4			operator / (const int4 &inA, int inS)				{ return int4(inA.x / inS, inA.y / inS, inA.z / inS, inA.w / inS); }

// Dot product
constexpr int			dot(const int4 &inA, const int4 &inB)				{ return inA.x * inB.x + inA.y * inB.y + inA.z * inB.z + inA.w * inB.w; }

// Min value
constexpr int4			min(const int4 &inA, const int4 &inB)				{ return int4(min(inA.x, inB.x), min(inA.y, inB.y), min(inA.z, inB.z), min(inA.w, inB.w)); }

// Max value
constexpr int4			max(const int4 &inA, const int4 &inB)				{ return int4(max(inA.x, inB.x), max(inA.y, inB.y), max(inA.z, inB.z), max(inA.w, inB.w)); }

//////////////////////////////////////////////////////////////////////////////////////////
// Mat44
//////////////////////////////////////////////////////////////////////////////////////////

struct Mat44
{
	// Constructors
	inline				Mat44() = default;
	constexpr 			Mat44(const float4 &inC0, const float4 &inC1, const float4 &inC2, const float4 &inC3) : c { inC0, inC1, inC2, inC3 } { }

	// Columns
	float4 &			operator [] (uint inIndex)							{ return c[inIndex]; }
	const float4 &		operator [] (uint inIndex) const					{ return c[inIndex]; }

private:
	float4				c[4];
};

//////////////////////////////////////////////////////////////////////////////////////////
// Other types
//////////////////////////////////////////////////////////////////////////////////////////

using Quat = float4;
using Plane = float4;

// Clamp value
template <class T>
constexpr T				clamp(const T &inValue, const T &inMinValue, const T &inMaxValue)
{
	return min(max(inValue, inMinValue), inMaxValue);
}

// Atomic add
template <class T>
T						JPH_AtomicAdd(T &ioT, const T &inValue)
{
	std::atomic<T> *value = reinterpret_cast<std::atomic<T> *>(&ioT);
	return value->fetch_add(inValue) + inValue;
}

// Bitcast float4 to int4
inline int4				asint(const float4 &inV)							{ return int4(BitCast<int>(inV.x), BitCast<int>(inV.y), BitCast<int>(inV.z), BitCast<int>(inV.w)); }

// Functions that couldn't be declared earlier
constexpr				float3::float3(const uint3 &inV)					: x(float(inV.x)), y(float(inV.y)), z(float(inV.z)) { }
constexpr				float4::float4(const int4 &inV)						: x(float(inV.x)), y(float(inV.y)), z(float(inV.z)), w(float(inV.w)) { }

// Swizzle operators
#define xy				swizzle_xy()
#define yx				swizzle_yx()
#define xyz				swizzle_xyz()
#define xzy				swizzle_xzy()
#define yxz				swizzle_yxz()
#define yzx				swizzle_yzx()
#define zxy				swizzle_zxy()
#define zyx				swizzle_zyx()
#define xywz			swizzle_xywz()
#define xwyz			swizzle_xwyz()
#define wxyz			swizzle_wxyz()

} // HLSLToCPP

JPH_NAMESPACE_END
