// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "ShaderCore.h"

// Prevent including this file multiple times unless we're generating bindings
#if !defined(HAIR_STRUCTS_H) || defined(JPH_SHADER_GENERATE_WRAPPER)
#ifndef JPH_SHADER_GENERATE_WRAPPER
#define HAIR_STRUCTS_H
#endif

JPH_SUPPRESS_WARNING_PUSH
JPH_SUPPRESS_WARNINGS

JPH_SHADER_CONSTANT(int, cHairPerVertexBatch, 64)
JPH_SHADER_CONSTANT(int, cHairPerGridCellBatch, 32)
JPH_SHADER_CONSTANT(int, cHairPerStrandBatch, 32)
JPH_SHADER_CONSTANT(int, cHairPerRenderVertexBatch, 128)

JPH_SHADER_CONSTANT(int, cHairNumSVertexInfluences, 3)

JPH_SHADER_STRUCT_BEGIN(JPH_HairSkinWeight)
	JPH_SHADER_STRUCT_MEMBER(JPH_uint,			JointIdx)
	JPH_SHADER_STRUCT_MEMBER(JPH_float,			Weight)
JPH_SHADER_STRUCT_END(JPH_HairSkinWeight)

JPH_SHADER_STRUCT_BEGIN(JPH_HairSkinPoint)
	JPH_SHADER_STRUCT_MEMBER(JPH_uint,			TriangleIndex)		///< Index of triangle in mScalpVertices to which this skin point is attached
	JPH_SHADER_STRUCT_MEMBER(JPH_float,			U)					///< Barycentric u coordinate of skin point
	JPH_SHADER_STRUCT_MEMBER(JPH_float,			V)					///< Barycentric v coordinate of skin point
	JPH_SHADER_STRUCT_MEMBER(JPH_uint,			ToBishop)			///< Compressed quaternion to rotate the frame defined by the triangle normal and the first edge to the Bishop frame of the first vertex of the strand
JPH_SHADER_STRUCT_END(JPH_HairSkinPoint)

JPH_SHADER_STRUCT_BEGIN(JPH_HairGlobalPoseTransform)
	JPH_SHADER_STRUCT_MEMBER(JPH_float3,		Position)
	JPH_SHADER_STRUCT_MEMBER(JPH_Quat,			Rotation)
JPH_SHADER_STRUCT_END(JPH_HairGlobalPoseTransform)

JPH_SHADER_STRUCT_BEGIN(JPH_HairSVertexInfluence)
	JPH_SHADER_STRUCT_MEMBER(JPH_uint,			VertexIndex)		///< Index in mSimVertices that indicates to which simulated vertex this vertex is attached.
	JPH_SHADER_STRUCT_MEMBER(JPH_float3,		RelativePosition)	///< Position in local space from the simulated vertex to the render vertex
	JPH_SHADER_STRUCT_MEMBER(JPH_float,			Weight)				///< Influence weight, 0 = not attached, 1 = fully attached
JPH_SHADER_STRUCT_END(JPH_HairSVertexInfluence)

JPH_SHADER_STRUCT_BEGIN(JPH_HairPosition)
	JPH_SHADER_STRUCT_MEMBER(JPH_float3,		Position)
	JPH_SHADER_STRUCT_MEMBER(JPH_Quat,			Rotation)
JPH_SHADER_STRUCT_END(JPH_HairPosition)

JPH_SHADER_STRUCT_BEGIN(JPH_HairVelocity)
	JPH_SHADER_STRUCT_MEMBER(JPH_float3,		Velocity)
	JPH_SHADER_STRUCT_MEMBER(JPH_float3,		AngularVelocity)
JPH_SHADER_STRUCT_END(JPH_HairVelocity)

JPH_SHADER_STRUCT_BEGIN(JPH_HairMaterial)
	JPH_SHADER_STRUCT_MEMBER(JPH_float4,		WorldTransformInfluence)
	JPH_SHADER_STRUCT_MEMBER(JPH_float4,		GlobalPose)
	JPH_SHADER_STRUCT_MEMBER(JPH_float4,		SkinGlobalPose)
	JPH_SHADER_STRUCT_MEMBER(JPH_float4,		GravityFactor)
	JPH_SHADER_STRUCT_MEMBER(JPH_float4,		HairRadius)
	JPH_SHADER_STRUCT_MEMBER(JPH_float4,		BendComplianceMultiplier)
	JPH_SHADER_STRUCT_MEMBER(JPH_float4,		GridVelocityFactor)
	JPH_SHADER_STRUCT_MEMBER(JPH_uint,			EnableCollision)
	JPH_SHADER_STRUCT_MEMBER(JPH_uint,			EnableLRA)
	JPH_SHADER_STRUCT_MEMBER(JPH_uint,			EnableGrid)
	JPH_SHADER_STRUCT_MEMBER(JPH_float,			Friction)
	JPH_SHADER_STRUCT_MEMBER(JPH_float,			ExpLinearDampingDeltaTime)
	JPH_SHADER_STRUCT_MEMBER(JPH_float,			ExpAngularDampingDeltaTime)
	JPH_SHADER_STRUCT_MEMBER(JPH_float,			BendComplianceInvDeltaTimeSq)
	JPH_SHADER_STRUCT_MEMBER(JPH_float,			StretchComplianceInvDeltaTimeSq)
	JPH_SHADER_STRUCT_MEMBER(JPH_float,			GridDensityForceFactor)
	JPH_SHADER_STRUCT_MEMBER(JPH_float,			InertiaMultiplier)
	JPH_SHADER_STRUCT_MEMBER(JPH_float,			MaxLinearVelocitySq)
	JPH_SHADER_STRUCT_MEMBER(JPH_float,			MaxAngularVelocitySq)
JPH_SHADER_STRUCT_END(JPH_HairMaterial)

JPH_SHADER_STRUCT_BEGIN(JPH_HairCollisionPlane)
	JPH_SHADER_STRUCT_MEMBER(JPH_Plane,			Plane)
	JPH_SHADER_STRUCT_MEMBER(JPH_uint,			ShapeIndex)
JPH_SHADER_STRUCT_END(JPH_HairCollisionPlane)

JPH_SHADER_STRUCT_BEGIN(JPH_HairCollisionShape)
	JPH_SHADER_STRUCT_MEMBER(JPH_float3,		CenterOfMass)
	JPH_SHADER_STRUCT_MEMBER(JPH_float3,		LinearVelocity)
	JPH_SHADER_STRUCT_MEMBER(JPH_float3,		AngularVelocity)
JPH_SHADER_STRUCT_END(JPH_HairCollisionShape)

// Note: The order was chosen to match the struct between C++ and HLSL.
JPH_SHADER_CONSTANTS_BEGIN(JPH_HairUpdateContext, gContext)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_uint,		NumStrands)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_uint,		NumVertices)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_uint,		NumGridPoints)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_uint,		NumRenderVertices)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_uint3,		GridSizeMin2)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_float,		TwoDivDeltaTime)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_float3,		GridSizeMin1)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_float,		DeltaTime)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_float3,		GridOffset)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_float,		HalfDeltaTime)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_float3,		GridScale)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_float,		InvDeltaTimeSq)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_float3,		SubStepGravity)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_uint,		NumSkinVertices)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_uint3,		GridStride)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_uint,		NumSkinWeightsPerVertex)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_Mat44,		DeltaTransform)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_Mat44,		ScalpToHead)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_Quat,		DeltaTransformQuat)
JPH_SHADER_CONSTANTS_END(JPH_HairUpdateContext)

// Note: The order was chosen to match the struct between C++ and HLSL.
JPH_SHADER_CONSTANTS_BEGIN(JPH_HairIterationContext, gIterationContext)
	JPH_SHADER_CONSTANTS_MEMBER(JPH_float,		AccumulatedDeltaTime)		///< = Iteration * DeltaTime
	JPH_SHADER_CONSTANTS_MEMBER(JPH_float,		IterationFraction)			///< = 1 / (NumIterations - Iteration) or the fraction to apply to get from current to target for this iteration step
JPH_SHADER_CONSTANTS_END(JPH_HairIterationContext)

JPH_SUPPRESS_WARNING_POP

#endif // !defined(HAIR_STRUCTS_H) || defined(JPH_SHADER_GENERATE_WRAPPER)
