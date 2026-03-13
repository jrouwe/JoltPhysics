// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2026 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#if defined(JPH_USE_RVV)

template <unsigned Index>
JPH_INLINE float RVVElement(vfloat32m1_t inV)
{
	const vfloat32m1_t moved = __riscv_vslidedown_vx_f32m1(inV, Index, 1);
	return __riscv_vfmv_f_s_f32m1_f32(moved);
}

template <unsigned IndexX, unsigned IndexY, unsigned IndexZ, unsigned IndexW>
JPH_INLINE vfloat32m1_t RVVShuffleFloat32x4(vfloat32m1_t inV0, vfloat32m1_t inV1)
{
	vfloat32m2_t combined = __riscv_vset_v_f32m1_f32m2(__riscv_vundefined_f32m2(), 0, inV0);
	combined = __riscv_vset_v_f32m1_f32m2(combined, 1, inV1);

	const uint32_t indices_raw[4] = { IndexX, IndexY, IndexZ, IndexW };
	const vuint32m1_t v_indices_m1 = __riscv_vle32_v_u32m1(indices_raw, 4);
	const vuint32m2_t v_indices_m2 = __riscv_vlmul_ext_v_u32m1_u32m2(v_indices_m1);

	const vfloat32m2_t gathered_m2 = __riscv_vrgather_vv_f32m2(combined, v_indices_m2, 4);
	return __riscv_vlmul_trunc_v_f32m2_f32m1(gathered_m2);
}

template <>
JPH_INLINE vfloat32m1_t RVVShuffleFloat32x4<0, 1, 2, 3>(vfloat32m1_t inV0, vfloat32m1_t inV1)
{
	return inV0;
}

template <>
JPH_INLINE vfloat32m1_t RVVShuffleFloat32x4<4, 5, 6, 7>(vfloat32m1_t inV0, vfloat32m1_t inV1)
{
	return inV1;
}

#endif // JPH_USE_RVV
