// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

float DeltaDensity(uint inIndex)
{
	return gVelocityAndDensity[inIndex].w - gNeutralDensity[inIndex];
}

void ApplyGrid(JPH_IN(JPH_HairPosition) inPos, JPH_IN_OUT(JPH_HairVelocity) ioVel, JPH_IN(JPH_HairMaterial) inMaterial, float inStrandFraction)
{
	if (!inMaterial.mEnableGrid)
		return;

	// Convert position to grid index and fraction
	uint3 index;
	float3 ma;
	GridPositionToIndexAndFraction(inPos.mPosition, index, ma);
	float3 a = float3(1, 1, 1) - ma;

	// Get average velocity at the vertex position (trilinear sample)
	float3 velocity;
	uint3 stride = cGridStride;
	uint adr_000 = GridIndexToBufferIndex(index);
	uint adr_100 = adr_000 + 1;
	uint adr_010 = adr_000 + stride.y;
	uint adr_110 = adr_010 + 1;
	velocity =  gVelocityAndDensity[adr_000].xyz            * ( a.x *  a.y *  a.z);
	velocity += gVelocityAndDensity[adr_100].xyz            * (ma.x *  a.y *  a.z);
	velocity += gVelocityAndDensity[adr_010].xyz            * ( a.x * ma.y *  a.z);
	velocity += gVelocityAndDensity[adr_110].xyz            * (ma.x * ma.y *  a.z);
	velocity += gVelocityAndDensity[adr_000 + stride.z].xyz * ( a.x *  a.y * ma.z);
	velocity += gVelocityAndDensity[adr_100 + stride.z].xyz * (ma.x *  a.y * ma.z);
	velocity += gVelocityAndDensity[adr_010 + stride.z].xyz * ( a.x * ma.y * ma.z);
	velocity += gVelocityAndDensity[adr_110 + stride.z].xyz * (ma.x * ma.y * ma.z);

	// Drive towards the average velocity of the cell
	ioVel.mVelocity += GradientSamplerSample(inMaterial.mGridVelocityFactor, inStrandFraction) * (velocity - ioVel.mVelocity);

	// Calculate force to go towards neutral density
	// Based on eq 3 of Volumetric Methods for Simulation and Rendering of Hair - Lena Petrovic, Mark Henne and John Anderson
	float dd000 = DeltaDensity(adr_000);
	float dd100 = DeltaDensity(adr_100);
	float dd010 = DeltaDensity(adr_010);
	float dd110 = DeltaDensity(adr_110);
	float dd001 = DeltaDensity(adr_000 + stride.z);
	float dd101 = DeltaDensity(adr_100 + stride.z);
	float dd011 = DeltaDensity(adr_010 + stride.z);
	float dd111 = DeltaDensity(adr_110 + stride.z);

	float3 force = float3(
		   a.y *  a.z * (dd000 - dd100)
		+ ma.y *  a.z * (dd010 - dd110)
		+  a.y * ma.z * (dd001 - dd101)
		+ ma.y * ma.z * (dd011 - dd111),

		   a.x *  a.z * (dd000 - dd010)
		+ ma.x *  a.z * (dd100 - dd110)
		+  a.x * ma.z * (dd001 - dd011)
		+ ma.x * ma.z * (dd101 - dd111),

		   a.x *  a.y * (dd000 - dd001)
		+ ma.x *  a.y * (dd100 - dd101)
		+  a.x * ma.y * (dd010 - dd011)
		+ ma.x * ma.y * (dd110 - dd111));

	ioVel.mVelocity += inMaterial.mGridDensityForceFactor * force * cDeltaTime; // / mass, but mass is 1
}

void Integrate(JPH_IN_OUT(JPH_HairPosition) ioPos, JPH_IN(JPH_HairVelocity) inVel, JPH_IN(JPH_HairMaterial) inMaterial, float inStrandFraction)
{
	JPH_HairVelocity vel = inVel;

	// Gravity
	vel.mVelocity += cSubStepGravity * GradientSamplerSample(inMaterial.mGravityFactor, inStrandFraction);

	// Damping
	vel.mVelocity *= inMaterial.mExpLinearDampingDeltaTime;
	vel.mAngularVelocity *= inMaterial.mExpAngularDampingDeltaTime;

	// Integrate position
	ioPos.mPosition += vel.mVelocity * cDeltaTime;

	// Integrate rotation
	JPH_Quat rotation = ioPos.mRotation;
	JPH_Quat delta_rotation = cHalfDeltaTime * JPH_QuatImaginaryMulQuat(vel.mAngularVelocity, rotation);
	ioPos.mRotation = normalize(rotation + delta_rotation);
}
