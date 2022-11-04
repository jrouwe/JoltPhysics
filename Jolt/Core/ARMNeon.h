// SPDX-FileCopyrightText: 2022 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#ifdef JPH_CPU_ARM64

#ifdef JPH_COMPILER_MSVC
	JPH_NAMESPACE_BEGIN

	template <unsigned I1, unsigned I2, unsigned I3, unsigned I4>
	float32x4_t NeonShuffleFloat32x4(float32x4_t inV1, float32x4_t inV2)
	{
		float32x4_t ret;
		ret = vmovq_n_f32(vgetq_lane_f32(I1 >= 4? inV2 : inV1, I1 & 0b11));
		ret = vsetq_lane_f32(vgetq_lane_f32(I2 >= 4? inV2 : inV1, I2 & 0b11), ret, 1);
		ret = vsetq_lane_f32(vgetq_lane_f32(I3 >= 4? inV2 : inV1, I3 & 0b11), ret, 2);
		ret = vsetq_lane_f32(vgetq_lane_f32(I4 >= 4? inV2 : inV1, I4 & 0b11), ret, 3);
		return ret;
	}

	#define JPH_NEON_SHUFFLE_F32x4(vec1, vec2, index1, index2, index3, index4) NeonShuffleFloat32x4<index1, index2, index3, index4>(vec1, vec2)

	// Constructing NEON values
	#define JPH_NEON_INT32x4(v1, v2, v3, v4) { int64_t(v1) + (int64_t(v2) << 32), int64_t(v3) + (int64_t(v4) << 32) }
	#define JPH_NEON_UINT32x4(v1, v2, v3, v4) { uint64_t(v1) + (uint64_t(v2) << 32), uint64_t(v3) + (uint64_t(v4) << 32) }
	#define JPH_NEON_INT8x16(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16) { int64_t(v1) + (int64_t(v2) << 8) + (int64_t(v3) << 16) + (int64_t(v4) << 24) + (int64_t(v5) << 32) + (int64_t(v6) << 40) + (int64_t(v7) << 48) + (int64_t(v8) << 56), int64_t(v9) + (int64_t(v10) << 8) + (int64_t(v11) << 16) + (int64_t(v12) << 24) + (int64_t(v13) << 32) + (int64_t(v14) << 40) + (int64_t(v15) << 48) + (int64_t(v16) << 56) }

	JPH_NAMESPACE_END
#else
	#define JPH_NEON_SHUFFLE_F32x4(vec1, vec2, index1, index2, index3, index4) __builtin_shufflevector(vec1, vec2, index1, index2, index3, index4)

	// Constructing NEON values
	#define JPH_NEON_INT32x4(v1, v2, v3, v4) { v1, v2, v3, v4 }
	#define JPH_NEON_UINT32x4(v1, v2, v3, v4) { v1, v2, v3, v4 }
	#define JPH_NEON_INT8x16(v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16) { v1, v2, v3, v4, v5, v6, v7, v8, v9, v10, v11, v12, v13, v14, v15, v16 }
#endif

#endif // JPH_CPU_ARM64
