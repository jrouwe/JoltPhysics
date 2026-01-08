# Usage: Run in Blender with the correct scene setup to produce a face.bin file:
# - A collection named "Visual"
#   - An Armature with Bones and an Animation
#   - One of the Bones should be called Neck and will be used to parent the hair to
#   - A mesh skinned to those Bones
# - A collection named "Collision"
#   - One or more meshes parent constrained to one of the Bones

import bpy
import mathutils
import struct

export_path = "C:\\Users\\jrouw\\Documents\\Code\\JoltPhysics\\Assets\\"
head_joint = "Neck"
scale = 0.00254
basis = mathutils.Matrix([[0, scale, 0, 0], [0, 0, scale, 0], [scale, 0, 0, 0], [0, 0, 0, 1]])
num_weights = 3

def apply_basis(m):
	return basis @ m @ basis.inverted()

def export_scene_meshes_to_bin(filepath):
	with open(filepath, "wb") as f:
		bpy.context.scene.frame_set(1)

		# Find the skinned mesh in the "Visual" collection
		visual_coll = bpy.data.collections.get("Visual")
		assert visual_coll, "No 'Visual' collection found"
		visual_meshes = [obj for obj in visual_coll.objects if obj.type == 'MESH']
		assert len(visual_meshes) == 1, "There must be exactly one mesh in the 'Visual' collection"
		obj = visual_meshes[0]
		mesh = obj.data

		# Head joint index
		armature = next((m for m in obj.modifiers if m.type == 'ARMATURE'), None)
		assert armature
		bones = armature.object.data.bones
		head_joint_idx = [b.name for b in bones].index(head_joint)
		f.write(struct.pack("<I", head_joint_idx))

		# Vertices
		vertices = [basis @ obj.matrix_world @ v.co for v in mesh.vertices]
		f.write(struct.pack("<I", len(vertices)))
		for v in vertices:
			f.write(struct.pack("<3f", v[0], v[1], v[2]))

		# Indices (triangles)
		indices = [tuple(p.vertices) for p in mesh.polygons]
		f.write(struct.pack("<I", len(indices)))
		for t in indices:
			f.write(struct.pack("<3I", t[0], t[1], t[2]))

		# Inverse Bind Matrices
		inv_bind_matrices = [apply_basis(b.matrix_local).inverted() for b in bones]
		f.write(struct.pack("<I", len(inv_bind_matrices)))
		for m in inv_bind_matrices:
			# Write 16 floats (column major)
			for i in range(4):
				for j in range(4):
					f.write(struct.pack("<f", m[j][i]))

		# Skin Weights
		weights = []
		for v in mesh.vertices:
			vw = []
			total_weight = 0
			for g in sorted(v.groups, key=lambda g: g.weight, reverse=True)[:num_weights]:
				bone_index = [b.name for b in bones].index(obj.vertex_groups[g.group].name)
				vw.append((bone_index, g.weight))
				total_weight += g.weight
			# Pad to 1
			if len(vw) < 1:
				vw.append((0, 1.0))
			elif total_weight <= 0:
				vw = [[v[0], 1.0] for v in vw]
			else:
				vw = [[v[0], v[1] / total_weight] for v in vw]
			weights.append(vw)
		f.write(struct.pack("<I", num_weights))
		for vw in weights:
			# Always write 'num_weights' weights per vertex (pad with zeros if needed)
			for i in range(num_weights):
				if i < len(vw):
					bone_index, weight = vw[i]
				else:
					bone_index, weight = 0, 0.0
				f.write(struct.pack("<If", bone_index, weight))

		# Animation (per frame, per joint)
		armature_obj = armature.object
		if armature_obj.animation_data and armature_obj.animation_data.action:
			action = armature_obj.animation_data.action
			frame_start = int(action.frame_range[0])
			frame_end = int(action.frame_range[1])
			num_frames = frame_end - frame_start + 1
			f.write(struct.pack("<I", num_frames))
			for frame in range(frame_start, frame_end + 1):
				bpy.context.scene.frame_set(frame)
				for bone in bones:
					pose_bone = armature.object.pose.bones[bone.name]
					mat = apply_basis(pose_bone.matrix)

					# Translation from matrix
					t = mat.to_translation()

					# Rotation quaternion from matrix
					q = mat.to_quaternion().normalized()

					# Ensure unique quaternion sign: make W positive
					if q.w < 0.0:
						q = -q

					# Export translation (x,y,z) + quaternion real part (x,y,z)
					f.write(struct.pack("<3f", t.x, t.y, t.z))
					f.write(struct.pack("<3f", q.x, q.y, q.z))
		else:
			print("No animation data found on the armature object.")
		bpy.context.scene.frame_set(1)

		# Export collision hulls
		collision_coll = bpy.data.collections.get("Collision")
		assert collision_coll, "No 'Collision' collection found"
		collision_meshes = [obj for obj in collision_coll.objects if obj.type == 'MESH']

		f.write(struct.pack("<I", len(collision_meshes)))
		for col_obj in collision_meshes:
			# Find parent bone name and index
			parent = col_obj.parent
			assert parent and parent.type == 'ARMATURE', f"Collision mesh '{col_obj.name}' must be parented to a bone"
			bone_name = col_obj.parent_bone
			try:
				joint_index = [b.name for b in bones].index(bone_name)
			except ValueError:
				joint_index = 0xffffffff
			f.write(struct.pack("<I", joint_index))

			# Write vertices
			verts = [basis @ col_obj.matrix_world @ v.co for v in col_obj.data.vertices]
			f.write(struct.pack("<I", len(verts)))
			for v in verts:
				f.write(struct.pack("<3f", v[0], v[1], v[2]))

export_scene_meshes_to_bin(export_path + "face.bin")
