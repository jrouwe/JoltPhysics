// SPDX-FileCopyrightText: 2021 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

JPH_NAMESPACE_BEGIN

// Turn off fused multiply add instruction because it makes the equations of the form a * b - c * d inaccurate below
JPH_PRECISE_MATH_ON

/// Helper utils to find the closest point to a line segment, triangle or tetrahedron
namespace ClosestPoint
{
	/// Compute barycentric coordinates of closest point to origin for infinite line defined by (inA, inB)
	/// Point can then be computed as inA * outU + inB * outV
	inline void GetBaryCentricCoordinates(Vec3Arg inA, Vec3Arg inB, float &outU, float &outV)
	{
		Vec3 ab = inB - inA;
		float denominator = ab.LengthSq();
		if (denominator < Square(FLT_EPSILON))
		{
			// Degenerate line segment, fallback to points
			if (inA.LengthSq() < inB.LengthSq())
			{
				// A closest
				outU = 1.0f;
				outV = 0.0f;
			}
			else
			{
				// B closest
				outU = 0.0f;
				outV = 1.0f;
			}
		}
		else
		{
			outV = -inA.Dot(ab) / denominator;
			outU = 1.0f - outV;
		}
	}
	
	/// Compute barycentric coordinates of closest point to origin for plane defined by (inA, inB, inC)
	/// Point can then be computed as inA * outU + inB * outV + inC * outW
	inline void GetBaryCentricCoordinates(Vec3Arg inA, Vec3Arg inB, Vec3Arg inC, float &outU, float &outV, float &outW)
	{
		// Taken from: Real-Time Collision Detection - Christer Ericson (Section: Barycentric Coordinates)
		// With p = 0
		// Adjusted to always include the shortest edge of the triangle in the calculation to improve numerical accuracy

		// First calculate the three edges
		Vec3 v0 = inB - inA;
		Vec3 v1 = inC - inA;
		Vec3 v2 = inC - inB;

		// Make sure that the shortest edge is included in the calculation to keep the products a * b - c * d as small as possible to preserve accuracy
		float d00 = v0.Dot(v0); 
		float d11 = v1.Dot(v1); 
		float d22 = v2.Dot(v2);
		if (d00 <= d22)
		{
			// Use v0 and v1 to calculate barycentric coordinates
			float d01 = v0.Dot(v1); 
		
			float denominator = d00 * d11 - d01 * d01; 
			if (abs(denominator) < FLT_EPSILON)
			{
				// Degenerate triangle, return coordinates along longest edge
				if (d00 > d11)
				{
					GetBaryCentricCoordinates(inA, inB, outU, outV);
					outW = 0.0f;
				}
				else
				{
					GetBaryCentricCoordinates(inA, inC, outU, outW);
					outV = 0.0f;
				}
			}
			else
			{
				float a0 = inA.Dot(v0);
				float a1 = inA.Dot(v1); 
				outV = (d01 * a1 - d11 * a0) / denominator; 
				outW = (d01 * a0 - d00 * a1) / denominator; 
				outU = 1.0f - outV - outW;
			}
		}
		else
		{
			// Use v1 and v2 to calculate barycentric coordinates
			float d12 = v1.Dot(v2); 
		
			float denominator = d11 * d22 - d12 * d12; 
			if (abs(denominator) < FLT_EPSILON)
			{
				// Degenerate triangle, return coordinates along longest edge
				if (d11 > d22)
				{
					GetBaryCentricCoordinates(inA, inC, outU, outW);
					outV = 0.0f;
				}
				else
				{
					GetBaryCentricCoordinates(inB, inC, outV, outW);
					outU = 0.0f;
				}
			}
			else
			{
				float c1 = inC.Dot(v1);
				float c2 = inC.Dot(v2); 
				outU = (d22 * c1 - d12 * c2) / denominator; 
				outV = (d11 * c2 - d12 * c1) / denominator; 
				outW = 1.0f - outU - outV;
			}
		}
	}

	/// Get the closest point to the origin of line (inA, inB)
	/// outSet describes which features are closest: 1 = a, 2 = b, 3 = line segment ab
	inline Vec3	GetClosestPointOnLine(Vec3Arg inA, Vec3Arg inB, uint32 &outSet) 
	{
		float u, v;
		GetBaryCentricCoordinates(inA, inB, u, v);
		if (v <= 0.0f)
		{
			// inA is closest point
			outSet = 0b0001;
			return inA;
		}
		else if (u <= 0.0f)
		{
			// inB is closest point
			outSet = 0b0010;
			return inB;
		}
		else
		{
			// Closest point lies on line inA inB
			outSet = 0b0011;
			return u * inA + v * inB;
		}
	}

	/// Get the closest point to the origin of triangle (inA, inB, inC)
	/// outSet describes which features are closest: 1 = a, 2 = b, 4 = c, 5 = line segment ac, 7 = triangle interior etc.
	inline Vec3	GetClosestPointOnTriangle(Vec3Arg inA, Vec3Arg inB, Vec3Arg inC, uint32 &outSet)
	{
		// Taken from: Real-Time Collision Detection - Christer Ericson (Section: Closest Point on Triangle to Point)
		// With p = 0

		// Calculate edges
		Vec3 ab = inB - inA; 
		Vec3 ac = inC - inA; 
		Vec3 bc = inC - inB;

		// The most accurate normal is calculated by using the two shortest edges
		// See: https://box2d.org/posts/2014/01/troublesome-triangle/
		// The difference in normals is most pronounced when one edge is much smaller than the others (in which case the other 2 must have roughly the same length).
		// Therefore we can suffice by just picking the shortest from 2 edges and use that with the 3rd edge to calculate the normal.
		// We first check which of the edges is shorter
		UVec4 bc_shorter_than_ac = Vec4::sLess(bc.DotV4(bc), ac.DotV4(ac));

		// We calculate both normals and then select the one that had the shortest edge for our normal (this avoids branching)
		Vec3 normal_bc = ab.Cross(bc);
		Vec3 normal_ac = ab.Cross(ac);
		Vec3 n = Vec3::sSelect(normal_ac, normal_bc, bc_shorter_than_ac);
		float n_len_sq = n.LengthSq();

		// Check degenerate
		if (n_len_sq < Square(FLT_EPSILON))
		{
			// Degenerate, fallback to edges

			// Edge AB
			uint32 closest_set;
			Vec3 closest_point = GetClosestPointOnLine(inA, inB, closest_set);
			float best_dist_sq = closest_point.LengthSq();

			// Edge AC
			uint32 set;
			Vec3 q = GetClosestPointOnLine(inA, inC, set);
			float dist_sq = q.LengthSq();
			if (dist_sq < best_dist_sq)
			{
				closest_point = q;
				best_dist_sq = dist_sq;
				closest_set = (set & 0b0001) + ((set & 0b0010) << 1);
			}

			// Edge BC
			q = GetClosestPointOnLine(inB, inC, set);
			dist_sq = q.LengthSq();
			if (dist_sq < best_dist_sq)
			{
				closest_point = q;
				closest_set = set << 1;
			}

			outSet = closest_set;
			return closest_point;
		}

		// Check if P in vertex region outside A 
		Vec3 ap = -inA; 
		float d1 = ab.Dot(ap); 
		float d2 = ac.Dot(ap); 
		if (d1 <= 0.0f && d2 <= 0.0f)
		{
			outSet = 0b0001;
			return inA; // barycentric coordinates (1,0,0)
		}

		// Check if P in vertex region outside B 
		Vec3 bp = -inB; 
		float d3 = ab.Dot(bp); 
		float d4 = ac.Dot(bp); 
		if (d3 >= 0.0f && d4 <= d3) 
		{
			outSet = 0b0010;
			return inB; // barycentric coordinates (0,1,0)
		}

		// Check if P in edge region of AB, if so return projection of P onto AB 
		float vc = d1 * d4 - d3 * d2; 
		if (vc <= 0.0f && d1 >= 0.0f && d3 <= 0.0f) 
		{ 
			float v = d1 / (d1 - d3); 
			outSet = 0b0011;
			return inA + v * ab; // barycentric coordinates (1-v,v,0) 
		}

		// Check if P in vertex region outside C 
		Vec3 cp = -inC; 
		float d5 = ab.Dot(cp); 
		float d6 = ac.Dot(cp); 
		if (d6 >= 0.0f && d5 <= d6) 
		{
			outSet = 0b0100;
			return inC; // barycentric coordinates (0,0,1)
		}

		// Check if P in edge region of AC, if so return projection of P onto AC 
		float vb = d5 * d2 - d1 * d6; 
		if (vb <= 0.0f && d2 >= 0.0f && d6 <= 0.0f) 
		{ 
			float w = d2 / (d2 - d6); 
			outSet = 0b0101;
			return inA + w * ac; // barycentric coordinates (1-w,0,w) 
		}

		// Check if P in edge region of BC, if so return projection of P onto BC 
		float va = d3 * d6 - d5 * d4;
		float d4_d3 = d4 - d3;
		float d5_d6 = d5 - d6;
		if (va <= 0.0f && d4_d3 >= 0.0f && d5_d6 >= 0.0f) 
		{ 
			float w = d4_d3 / (d4_d3 + d5_d6); 
			outSet = 0b0110;
			return inB + w * bc; // barycentric coordinates (0,1-w,w) 
		}

		// P inside face region.
		// Here we deviate from Christer Ericson's article to improve accuracy.
		// Determine distance between triangle and origin: distance = (centroid - origin) . normal / |normal|
		// Closest point to origin is then: distance . normal / |normal|
		// Note that this way of calculating the closest point is much more accurate than first calculating barycentric coordinates 
		// and then calculating the closest point based on those coordinates.
		outSet = 0b0111;
		return n * (inA + inB + inC).Dot(n) / (3.0f * n_len_sq);
	}

	/// Check if the origin is outside the plane of triangle (inA, inB, inC). inD specifies the front side of the plane.
	inline bool OriginOutsideOfPlane(Vec3Arg inA, Vec3Arg inB, Vec3Arg inC, Vec3Arg inD)
	{
		// Taken from: Real-Time Collision Detection - Christer Ericson (Section: Closest Point on Tetrahedron to Point)
		// With p = 0

		// Test if point p and d lie on opposite sides of plane through abc 
		Vec3 n = (inB - inA).Cross(inC - inA);
		float signp = inA.Dot(n); // [AP AB AC]
		float signd = (inD - inA).Dot(n); // [AD AB AC] 
													   
		// Points on opposite sides if expression signs are the same
		// Note that we left out the minus sign in signp so we need to check > 0 instead of < 0 as in Christer's book
		// We compare against a small negative value to allow for a little bit of slop in the calculations
		return signp * signd > -FLT_EPSILON; 
	}

	/// Returns for each of the planes of the tetrahedron if the origin is inside it
	/// Roughly equivalent to: 
	///	[OriginOutsideOfPlane(inA, inB, inC, inD), 
	///	 OriginOutsideOfPlane(inA, inC, inD, inB), 
	///	 OriginOutsideOfPlane(inA, inD, inB, inC), 
	///	 OriginOutsideOfPlane(inB, inD, inC, inA)]
	inline UVec4 OriginOutsideOfTetrahedronPlanes(Vec3Arg inA, Vec3Arg inB, Vec3Arg inC, Vec3Arg inD)
	{
		Vec3 ab = inB - inA;
		Vec3 ac = inC - inA;
		Vec3 ad = inD - inA;
		Vec3 bd = inD - inB;
		Vec3 bc = inC - inB;

		Vec3 ab_cross_ac = ab.Cross(ac);
		Vec3 ac_cross_ad = ac.Cross(ad);
		Vec3 ad_cross_ab = ad.Cross(ab);
		Vec3 bd_cross_bc = bd.Cross(bc);
		
		// For each plane get the side on which the origin is
		float signp0 = inA.Dot(ab_cross_ac); // ABC
		float signp1 = inA.Dot(ac_cross_ad); // ACD
		float signp2 = inA.Dot(ad_cross_ab); // ADB
		float signp3 = inB.Dot(bd_cross_bc); // BDC
		Vec4 signp(signp0, signp1, signp2, signp3);

		// For each plane get the side that is outside (determined by the 4th point)
		float signd0 = ad.Dot(ab_cross_ac);  // D
		float signd1 = ab.Dot(ac_cross_ad);  // B
		float signd2 = ac.Dot(ad_cross_ab);  // C
		float signd3 = -ab.Dot(bd_cross_bc); // A
		Vec4 signd(signd0, signd1, signd2, signd3);

		// The winding of all triangles has been chosen so that signd should have the
		// same sign for all components. If this is not the case the tetrahedron
		// is degenerate and we return that the origin is in front of all sides
		int sign_bits = signd.GetSignBits();
		switch (sign_bits)
		{
		case 0:
			// All positive
			return Vec4::sGreaterOrEqual(signp, Vec4::sReplicate(-FLT_EPSILON));

		case 0xf:
			// All negative
			return Vec4::sLessOrEqual(signp, Vec4::sReplicate(FLT_EPSILON));

		default:
			// Mixed signs, degenerate tetrahedron
			return UVec4::sReplicate(0xffffffff);
		}
	}

	/// Get the closest point between tetrahedron (inA, inB, inC, inD) to the origin
	/// outSet specifies which feature was closest, 1 = a, 2 = b, 4 = c, 8 = d. Edges have 2 bits set, triangles 3 and if the point is in the interior 4 bits are set.
	inline Vec3	GetClosestPointOnTetrahedron(Vec3Arg inA, Vec3Arg inB, Vec3Arg inC, Vec3Arg inD, uint32 &outSet)
	{
		// Taken from: Real-Time Collision Detection - Christer Ericson (Section: Closest Point on Tetrahedron to Point)
		// With p = 0

		// Start out assuming point inside all halfspaces, so closest to itself 
		uint32 closest_set = 0b1111;
		Vec3 closest_point = Vec3::sZero(); 
		float best_dist_sq = FLT_MAX; 
		
		// Determine for each of the faces of the tetrahedron if the origin is in front of the plane
		UVec4 origin_out_of_planes = OriginOutsideOfTetrahedronPlanes(inA, inB, inC, inD);

		// If point outside face abc then compute closest point on abc 
		if (origin_out_of_planes.GetX()) // OriginOutsideOfPlane(inA, inB, inC, inD)
		{ 
			Vec3 q = GetClosestPointOnTriangle(inA, inB, inC, closest_set); 
			float dist_sq = q.LengthSq(); 
			
			// Update best closest point if (squared) distance is less than current best 
			if (dist_sq < best_dist_sq) 
			{
				best_dist_sq = dist_sq;
				closest_point = q; 
			}
		} 
		
		// Repeat test for face acd 
		if (origin_out_of_planes.GetY()) // OriginOutsideOfPlane(inA, inC, inD, inB)
		{ 
			uint32 set;
			Vec3 q = GetClosestPointOnTriangle(inA, inC, inD, set); 
			float dist_sq = q.LengthSq(); 
			if (dist_sq < best_dist_sq) 
			{
				best_dist_sq = dist_sq;
				closest_point = q;
				closest_set = (set & 0b0001) + ((set & 0b0110) << 1);
			}
		}

		// Repeat test for face adb 
		if (origin_out_of_planes.GetZ()) // OriginOutsideOfPlane(inA, inD, inB, inC)
		{
			uint32 set;
			Vec3 q = GetClosestPointOnTriangle(inA, inD, inB, set); 
			float dist_sq = q.LengthSq(); 
			if (dist_sq < best_dist_sq) 
			{
				best_dist_sq = dist_sq;
				closest_point = q;
				closest_set = (set & 0b0001) + ((set & 0b0010) << 2) + ((set & 0b0100) >> 1); 
			}
		} 
		
		// Repeat test for face bdc 
		if (origin_out_of_planes.GetW()) // OriginOutsideOfPlane(inB, inD, inC, inA)
		{ 
			uint32 set;
			Vec3 q = GetClosestPointOnTriangle(inB, inD, inC, set); 
			float dist_sq = q.LengthSq(); 
			if (dist_sq < best_dist_sq) 
			{
				closest_point = q;
				closest_set = ((set & 0b0001) << 1) + ((set & 0b0010) << 2) + (set & 0b0100); 
			}
		} 
	
		outSet = closest_set;
		return closest_point;
	}
};

JPH_PRECISE_MATH_OFF

JPH_NAMESPACE_END
