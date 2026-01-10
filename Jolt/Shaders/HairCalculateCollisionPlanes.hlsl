// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#include "HairCalculateCollisionPlanesBindings.h"
#include "HairCommon.h"

JPH_SHADER_FUNCTION_BEGIN(void, main, cHairPerVertexBatch, 1, 1)
	JPH_SHADER_PARAM_THREAD_ID(tid)
JPH_SHADER_FUNCTION_END
{
	// Check if this is a valid vertex
	uint vtx = tid.x + cNumStrands; // Skip the root of each strand, it's fixed
	if (vtx >= cNumVertices)
		return;

	// Load the vertex
	float3 pos = gPositions[vtx].mPosition;

	// Start with a plane that is far away (i.e. no collision)
	JPH_HairCollisionPlane collision_plane;
	collision_plane.mPlane = float4(1, 0, 0, 1.0e6f);
	collision_plane.mShapeIndex = 0;
	float largest_penetration = -1.0e6f;

	// Loop over all shapes
	uint current_idx = 0;
	uint current_plane = 0;
	for (uint current_shape_idx = 0;; ++current_shape_idx)
	{
		// Find most facing plane
		float max_distance = -1.0e6f;
		float3 max_plane_normal = float3(0, 0, 0);
		uint max_plane_face_info = 0;

		// Get number of faces in this shape
		uint nf = gShapeIndices[current_idx++];
		if (nf == 0)
			break;

		for (uint f = 0; f < nf; ++f)
		{
			// Get the plane
			JPH_Plane plane = gShapePlanes[current_plane++];
			float distance = JPH_PlaneSignedDistance(plane, pos);
			if (distance > max_distance)
			{
				max_distance = distance;
				max_plane_normal = JPH_PlaneGetNormal(plane);
				max_plane_face_info = current_idx;
			}

			// Skip over vertex start and end
			current_idx += 2;
		}

		// Project point onto that plane, in local space to the vertex
		float3 closest_point = -max_distance * max_plane_normal;

		// Check edges if we're outside the hull (when inside we know the closest face is also the closest point to the surface)
		bool is_outside = max_distance > 0.0f;
		if (is_outside)
		{
			// Loop over edges
			float closest_point_dist_sq = 1.0e12f;
			uint vi = gShapeIndices[max_plane_face_info];
			uint vi_end = gShapeIndices[max_plane_face_info + 1];
			float3 p1 = gShapeVertices[gShapeIndices[vi_end - 1]];
			for (; vi < vi_end; ++vi)
			{
				// Get edge points
				float3 p2 = gShapeVertices[gShapeIndices[vi]];

				// Check if the position is outside the edge (if not, the face will be closer)
				float3 p1_p2 = p2 - p1;
				float3 p1_pos = p1 - pos;
				float3 edge_normal = cross(p1_p2, max_plane_normal);
				if (dot(edge_normal, p1_pos) <= 0.0f)
				{
					// Get closest point on edge
					float3 closest = JPH_GetClosestPointOnLine(p1_pos, p1_p2);
					float distance_sq = dot(closest, closest);
					if (distance_sq < closest_point_dist_sq)
					{
						closest_point_dist_sq = distance_sq;
						closest_point = closest;
					}
				}

				// Cycle vertex
				p1 = p2;
			}
		}

		// Check if this is the largest penetration
		float3 normal = -closest_point;
		float normal_length = length(normal);
		float penetration = normal_length;
		if (is_outside)
			penetration = -penetration;
		else
			normal = -normal;
		if (penetration > largest_penetration)
		{
			// Calculate contact plane
			normal = normal_length > 0.0f? normal / normal_length : max_plane_normal;
			collision_plane.mPlane = JPH_PlaneFromPointAndNormal(pos + closest_point, normal);
			collision_plane.mShapeIndex = current_shape_idx;
			largest_penetration = penetration;
		}
	}

	gCollisionPlanes[vtx] = collision_plane;
}
