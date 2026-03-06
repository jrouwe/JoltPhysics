// Jolt Physics Library (https://github.com/jrouwe/JoltPhysics)
// SPDX-FileCopyrightText: 2024 Jorrit Rouwe
// SPDX-License-Identifier: MIT

#pragma once

#if defined(JPH_USE_RVV)

template<unsigned Index>
JPH_INLINE float rvv_elem(vfloat32m1_t src)
{
  const vfloat32m1_t moved = __riscv_vslidedown_vx_f32m1(src, Index, 1);
  return __riscv_vfmv_f_s_f32m1_f32(moved);
}

template<unsigned IndexX, unsigned IndexY, unsigned IndexZ, unsigned IndexW>
JPH_INLINE vfloat32m1_t RVV_shuffle_f32x4(vfloat32m1_t v0, vfloat32m1_t v1)
{
  const float x = IndexX > 3 ? rvv_elem<IndexX & 0b11>(v1) : rvv_elem<IndexX>(v0);
  const float y = IndexY > 3 ? rvv_elem<IndexY & 0b11>(v1) : rvv_elem<IndexY>(v0);
  const float z = IndexZ > 3 ? rvv_elem<IndexZ & 0b11>(v1) : rvv_elem<IndexZ>(v0);
  const float w = IndexW > 3 ? rvv_elem<IndexW & 0b11>(v1) : rvv_elem<IndexW>(v0);

  vfloat32m1_t res = __riscv_vfmv_v_f_f32m1(w, 4);
  res = __riscv_vfslide1up_vf_f32m1(res, z, 4);
  res = __riscv_vfslide1up_vf_f32m1(res, y, 4);
  res = __riscv_vfslide1up_vf_f32m1(res, x, 4);

  return res;
}

template<>
JPH_INLINE vfloat32m1_t RVV_shuffle_f32x4<0, 1, 2, 3>(vfloat32m1_t v0,
  vfloat32m1_t v1)
{
  return v0;
}

template<>
JPH_INLINE vfloat32m1_t RVV_shuffle_f32x4<4, 5, 6, 7>(vfloat32m1_t v0,
  vfloat32m1_t v1)
{
  return v1;
}



#endif // JPH_USE_RVV
