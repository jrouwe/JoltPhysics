// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

void ApplyGlobalPose(JPH_IN_OUT(JPH_HairPosition) ioPos, float3 inRestPosition, JPH_Quat inRestOrientation, JPH_IN(JPH_HairGlobalPoseTransform) inGlobalPoseTransform, JPH_IN(JPH_HairMaterial) inMaterial, float inStrandFraction)
{
	// LERP between stored global pose and global pose skinned to the scalp
	float skin_factor = GradientSamplerSample(inMaterial.mSkinGlobalPose, inStrandFraction);
	float3 in_position = inRestPosition;
	in_position += skin_factor * (inGlobalPoseTransform.mPosition + JPH_QuatMulVec3(inGlobalPoseTransform.mRotation, in_position) - in_position);
	JPH_Quat in_rotation = inRestOrientation;
	in_rotation += skin_factor * (JPH_QuatMulQuat(inGlobalPoseTransform.mRotation, in_rotation) - in_rotation);

	// LERP between simulated position and skinned position
	float pose_factor = GradientSamplerSample(inMaterial.mGlobalPose, inStrandFraction);
	ioPos.mPosition += pose_factor * (in_position - ioPos.mPosition);
	ioPos.mRotation += pose_factor * (in_rotation - ioPos.mRotation);
	ioPos.mRotation = normalize(ioPos.mRotation);
}
