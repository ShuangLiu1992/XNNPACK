// Copyright (c) Facebook, Inc. and its affiliates.
// All rights reserved.
//
// Copyright 2019 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include <fp16.h>

#include <xnnpack/math.h>
#include <xnnpack/operator.h>
#include <xnnpack/pack.h>
#include <xnnpack/unaligned.h>


void xnn_pack_f32_gemm_goi_w(
  size_t g,
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const float* k,
  const float* b,
  float* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  do {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          packed_weights[nr_block_offset] = b[nr_block_start + nr_block_offset];
        }
      }
      packed_weights += nr;

      for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
            const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
            if (kc_idx < kc) {
              packed_weights[kr_block_offset] = k[(nr_block_start + nr_block_offset) * kc + kc_idx];
            }
          }
          packed_weights += kr;
        }
        packed_weights += (nr - nr_block_size) * kr;
      }
      packed_weights = (float*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += nc * kc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  } while (--g != 0);
}

void xnn_pack_f16_gemm_goi_w(
  size_t g,
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const uint16_t* k,
  const uint16_t* b,
  uint16_t* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  do {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          packed_weights[nr_block_offset] = b[nr_block_start + nr_block_offset];
        }
      }
      packed_weights += nr;

      for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
            const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
            if (kc_idx < kc) {
              packed_weights[kr_block_offset] = k[(nr_block_start + nr_block_offset) * kc + kc_idx];
            }
          }
          packed_weights += kr;
        }
        packed_weights += (nr - nr_block_size) * kr;
      }
      packed_weights = (uint16_t*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += nc * kc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  } while (--g != 0);
}

void xnn_pack_f32_to_f16_gemm_goi_w(
  size_t g,
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const float* k,
  const float* b,
  uint16_t* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  do {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          packed_weights[nr_block_offset] = fp16_ieee_from_fp32_value(b[nr_block_start + nr_block_offset]);
        }
      }
      packed_weights += nr;

      for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
            const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
            if (kc_idx < kc) {
              packed_weights[kr_block_offset] = fp16_ieee_from_fp32_value(k[(nr_block_start + nr_block_offset) * kc + kc_idx]);
            }
          }
          packed_weights += kr;
        }
        packed_weights += (nr - nr_block_size) * kr;
      }
      packed_weights = (uint16_t*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += nc * kc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  } while (--g != 0);
}

void xnn_pack_qu8_gemm_goi_w(
  size_t g,
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const uint8_t* k,
  const int32_t* b,
  void* packed_weights,
  size_t extra_bytes,
  const struct xnn_qu8_packing_params* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  const int32_t izp = (int32_t) params->input_zero_point;
  const int32_t bzp = (int32_t) kc * izp * (int32_t) params->kernel_zero_point;
  do {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      int32_t* packed_b = (int32_t*) packed_weights;
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          unaligned_store_s32(packed_weights, bzp + b[nr_block_start + nr_block_offset]);
          packed_weights = (int32_t*) packed_weights + 1;
        }
      } else {
        size_t n = nr_block_size;
        do {
          unaligned_store_s32(packed_weights, bzp);
          packed_weights = (int32_t*) packed_weights + 1;
        } while (--n != 0);
      }
      packed_weights = (int32_t*) packed_weights + (nr - nr_block_size);

      for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          int32_t ksum = 0;
          for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
            const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
            if (kc_idx < kc) {
              const uint8_t kv = k[(nr_block_start + nr_block_offset) * kc + kc_idx];
              ksum += (int32_t) kv;
              ((uint8_t*) packed_weights)[kr_block_offset] = kv;
            }
          }
          unaligned_indexed_store_s32(packed_b, nr_block_offset, unaligned_indexed_load_s32(packed_b, nr_block_offset) - ksum * izp);
          packed_weights = (uint8_t*) packed_weights + kr;
        }
        packed_weights = (uint8_t*) packed_weights + (nr - nr_block_size) * kr;
      }
      packed_weights = (void*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += nc * kc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  } while (--g != 0);
}

void xnn_pack_qs8_gemm_goi_w(
  size_t g,
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const int8_t* k,
  const int32_t* b,
  void* packed_weights,
  size_t extra_bytes,
  const struct xnn_qs8_packing_params* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  const uint32_t izp = (uint32_t) params->input_zero_point;
  do {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      int32_t* packed_b = (int32_t*) packed_weights;
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          unaligned_store_s32(packed_weights, b[nr_block_start + nr_block_offset]);
          packed_weights = (int32_t*) packed_weights + 1;
        }
      } else {
        size_t n = nr_block_size;
        do {
          unaligned_store_s32(packed_weights, 0);
          packed_weights = (int32_t*) packed_weights + 1;
        } while (--n != 0);
      }
      packed_weights = (int32_t*) packed_weights + (nr - nr_block_size);

      for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          uint32_t ksum = 0;
          for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
            const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
            if (kc_idx < kc) {
              const int8_t kv = k[(nr_block_start + nr_block_offset) * kc + kc_idx];
              ksum += (uint32_t) kv;
              ((int8_t*) packed_weights)[kr_block_offset] = kv;
            }
          }
          unaligned_indexed_store_u32(packed_b, nr_block_offset, unaligned_indexed_load_u32(packed_b, nr_block_offset) - ksum * izp);
          packed_weights = (int8_t*) packed_weights + kr;
        }
        packed_weights = (int8_t*) packed_weights + (nr - nr_block_size) * kr;
      }
      packed_weights = (void*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += nc * kc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  } while (--g != 0);
}

void xnn_pack_qs8_gemm_xw_goi_w(
  size_t g,
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const int8_t* k,
  const int32_t* b,
  void* packed_weights,
  size_t extra_bytes,
  const struct xnn_qs8_packing_params* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  const uint32_t izp = (uint32_t) params->input_zero_point;
  do {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      int32_t* packed_b = (int32_t*) packed_weights;
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          unaligned_store_s32(packed_weights, b[nr_block_start + nr_block_offset]);
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
        }
      } else {
        size_t n = nr_block_size;
        do {
          unaligned_store_s32(packed_weights, 0);
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
        } while (--n != 0);
      }
      packed_weights = (void*) ((uintptr_t) packed_weights + (nr - nr_block_size) * sizeof(int32_t));

      for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          uint32_t ksum = 0;
          for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
            const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
            if (kc_idx < kc) {
              const int8_t kv = k[(nr_block_start + nr_block_offset) * kc + kc_idx];
              ksum += (uint32_t) kv;
              ((int16_t*) packed_weights)[kr_block_offset] = (int16_t) kv;
            }
          }
          unaligned_indexed_store_u32(packed_b, nr_block_offset, unaligned_indexed_load_u32(packed_b, nr_block_offset) - ksum * izp);
          packed_weights = (int16_t*) packed_weights + kr;
        }
        packed_weights = (int16_t*) packed_weights + (nr - nr_block_size) * kr;
      }
      packed_weights = (void*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += nc * kc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  } while (--g != 0);
}

void xnn_pack_f32_gemm_io_w(
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const float* k,
  const float* b,
  float* packed_weights,
  const void* params)
{
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
    const size_t nr_block_size = min(nc - nr_block_start, nr);
    if XNN_LIKELY(b != NULL) {
      for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
        packed_weights[nr_block_offset] = b[nr_block_start + nr_block_offset];
      }
    }
    packed_weights += nr;

    for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
      for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
        for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
          const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
          if (kc_idx < kc) {
            packed_weights[kr_block_offset] = k[kc_idx * nc + nr_block_start + nr_block_offset];
          }
        }
        packed_weights += kr;
      }
      packed_weights += (nr - nr_block_size) * kr;
    }
  }
}

void xnn_pack_f16_gemm_io_w(
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const uint16_t* k,
  const uint16_t* b,
  uint16_t* packed_weights,
  const void* params)
{
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
    const size_t nr_block_size = min(nc - nr_block_start, nr);
    if XNN_LIKELY(b != NULL) {
      for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
        packed_weights[nr_block_offset] = b[nr_block_start + nr_block_offset];
      }
    }
    packed_weights += nr;

    for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
      for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
        for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
          const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
          if (kc_idx < kc) {
            packed_weights[kr_block_offset] = k[kc_idx * nc + nr_block_start + nr_block_offset];
          }
        }
        packed_weights += kr;
      }
      packed_weights += (nr - nr_block_size) * kr;
    }
  }
}

void xnn_pack_f32_to_f16_gemm_io_w(
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const float* k,
  const float* b,
  uint16_t* packed_weights,
  const void* params)
{
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
    const size_t nr_block_size = min(nc - nr_block_start, nr);
    if XNN_LIKELY(b != NULL) {
      for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
        packed_weights[nr_block_offset] = fp16_ieee_from_fp32_value(b[nr_block_start + nr_block_offset]);
      }
    }
    packed_weights += nr;

    for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
      for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
        for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
          const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
          if (kc_idx < kc) {
            packed_weights[kr_block_offset] = fp16_ieee_from_fp32_value(k[kc_idx * nc + nr_block_start + nr_block_offset]);
          }
        }
        packed_weights += kr;
      }
      packed_weights += (nr - nr_block_size) * kr;
    }
  }
}

void xnn_pack_qu8_gemm_io_w(
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const uint8_t* k,
  const int32_t* b,
  void* packed_weights,
  const struct xnn_qu8_packing_params* params)
{
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  const int32_t izp = (int32_t) params->input_zero_point;
  const int32_t bzp = (int32_t) kc * izp * (int32_t) params->kernel_zero_point;
  for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
    const size_t nr_block_size = min(nc - nr_block_start, nr);
    int32_t* packed_b = (int32_t*) packed_weights;
    if XNN_LIKELY(b != NULL) {
      for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
        unaligned_store_s32(packed_weights, bzp + b[nr_block_start + nr_block_offset]);
        packed_weights = (int32_t*) packed_weights + 1;
      }
    } else {
      size_t n = nr_block_size;
      do {
        unaligned_store_s32(packed_weights, bzp);
        packed_weights = (int32_t*) packed_weights + 1;
      } while (--n != 0);
    }
    packed_weights = (int32_t*) packed_weights + (nr - nr_block_size);

    for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
      for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
        int32_t ksum = 0;
        for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
          const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
          if (kc_idx < kc) {
            const uint8_t kv = k[kc_idx * nc + (nr_block_start + nr_block_offset)];
            ksum += (int32_t) kv;
            ((uint8_t*) packed_weights)[kr_block_offset] = kv;
          }
        }
        unaligned_indexed_store_s32(packed_b, nr_block_offset, unaligned_indexed_load_s32(packed_b, nr_block_offset) - ksum * izp);
        packed_weights = (uint8_t*) packed_weights + kr;
      }
      packed_weights = (uint8_t*) packed_weights + (nr - nr_block_size) * kr;
    }
  }
}

void xnn_pack_qs8_gemm_io_w(
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const int8_t* k,
  const int32_t* b,
  void* packed_weights,
  const struct xnn_qs8_packing_params* params)
{
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  const uint32_t izp = (uint32_t) params->input_zero_point;
  for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
    const size_t nr_block_size = min(nc - nr_block_start, nr);
    int32_t* packed_b = (int32_t*) packed_weights;
    if XNN_LIKELY(b != NULL) {
      for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
        unaligned_store_s32(packed_weights, b[nr_block_start + nr_block_offset]);
        packed_weights = (int32_t*) packed_weights + 1;
      }
    } else {
      size_t n = nr_block_size;
      do {
        unaligned_store_s32(packed_weights, 0);
        packed_weights = (int32_t*) packed_weights + 1;
      } while (--n != 0);
    }
    packed_weights = (uint32_t*) packed_weights + (nr - nr_block_size);

    for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
      for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
        uint32_t ksum = 0;
        for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
          const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
          if (kc_idx < kc) {
            const int8_t kv = k[kc_idx * nc + (nr_block_start + nr_block_offset)];
            ksum += (uint32_t) kv;
            ((int8_t*) packed_weights)[kr_block_offset] = kv;
          }
        }
        unaligned_indexed_store_u32(packed_b, nr_block_offset, unaligned_indexed_load_u32(packed_b, nr_block_offset) - ksum * izp);
        packed_weights = (int8_t*) packed_weights + kr;
      }
      packed_weights = (int8_t*) packed_weights + (nr - nr_block_size) * kr;
    }
  }
}

void xnn_pack_f32_conv_goki_w(
  size_t g,
  size_t nc,
  size_t ks,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const float* k,
  const float* b,
  float* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  do {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          packed_weights[nr_block_offset] = b[nr_block_start + nr_block_offset];
        }
      }
      packed_weights += nr;

      for (size_t ki = 0; ki < ks; ki++) {
        for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
          for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
            for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
              const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
              if (kc_idx < kc) {
                packed_weights[kr_block_offset] = k[((nr_block_start + nr_block_offset) * ks + ki) * kc + kc_idx];
              }
            }
            packed_weights += kr;
          }
          packed_weights += (nr - nr_block_size) * kr;
        }
      }
      packed_weights = (float*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += ks * kc * nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  } while (--g != 0);
}

void xnn_pack_f16_conv_goki_w(
  size_t g,
  size_t nc,
  size_t ks,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const uint16_t* k,
  const uint16_t* b,
  uint16_t* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  do {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          packed_weights[nr_block_offset] = b[nr_block_start + nr_block_offset];
        }
      }
      packed_weights += nr;

      for (size_t ki = 0; ki < ks; ki++) {
        for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
          for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
            for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
              const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
              if (kc_idx < kc) {
                packed_weights[kr_block_offset] = k[((nr_block_start + nr_block_offset) * ks + ki) * kc + kc_idx];
              }
            }
            packed_weights += kr;
          }
          packed_weights += (nr - nr_block_size) * kr;
        }
      }
      packed_weights = (uint16_t*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += ks * kc * nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  } while (--g != 0);
}

void xnn_pack_f32_to_f16_conv_goki_w(
  size_t g,
  size_t nc,
  size_t ks,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const float* k,
  const float* b,
  uint16_t* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  do {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          packed_weights[nr_block_offset] = fp16_ieee_from_fp32_value(b[nr_block_start + nr_block_offset]);
        }
      }
      packed_weights += nr;

      for (size_t ki = 0; ki < ks; ki++) {
        for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
          for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
            for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
              const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
              if (kc_idx < kc) {
                packed_weights[kr_block_offset] = fp16_ieee_from_fp32_value(k[((nr_block_start + nr_block_offset) * ks + ki) * kc + kc_idx]);
              }
            }
            packed_weights += kr;
          }
          packed_weights += (nr - nr_block_size) * kr;
        }
      }
      packed_weights = (uint16_t*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += ks * kc * nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  } while (--g != 0);
}

void xnn_pack_qu8_conv_goki_w(
  size_t g,
  size_t nc,
  size_t ks,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const uint8_t* k,
  const int32_t* b,
  void* packed_weights,
  size_t extra_bytes,
  const struct xnn_qu8_packing_params* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  const int32_t izp = (int32_t) params->input_zero_point;
  const int32_t bzp = (int32_t) ks * (int32_t) kc * izp * (int32_t) params->kernel_zero_point;
  do {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      int32_t* packed_b = (int32_t*) packed_weights;
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          unaligned_store_s32(packed_weights, bzp + b[nr_block_start + nr_block_offset]);
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
        }
      } else {
        size_t n = nr_block_size;
        do {
          unaligned_store_s32(packed_weights, bzp);
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
        } while (--n != 0);
      }
      packed_weights = (void*) ((uintptr_t) packed_weights + (nr - nr_block_size) * sizeof(int32_t));

      for (size_t ki = 0; ki < ks; ki++) {
        for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
          for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
            int32_t ksum = 0;
            for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
              const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
              if (kc_idx < kc) {
                const uint8_t kv = k[((nr_block_start + nr_block_offset) * ks + ki) * kc + kc_idx];
                ksum += (int32_t) kv;
                ((uint8_t*) packed_weights)[kr_block_offset] = kv;
              }
            }
            unaligned_indexed_store_s32(packed_b, nr_block_offset, unaligned_indexed_load_s32(packed_b, nr_block_offset) - ksum * izp);
            packed_weights = (uint8_t*) packed_weights + kr;
          }
          packed_weights = (uint8_t*) packed_weights + (nr - nr_block_size) * kr;
        }
      }
      packed_weights = (void*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += ks * kc * nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  } while (--g != 0);
}

void xnn_pack_qs8_conv_goki_w(
  size_t g,
  size_t nc,
  size_t ks,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const int8_t* k,
  const int32_t* b,
  void* packed_weights,
  size_t extra_bytes,
  const struct xnn_qs8_packing_params* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  const uint32_t izp = (int32_t) params->input_zero_point;
  do {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      int32_t* packed_b = (int32_t*) packed_weights;
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          unaligned_store_s32(packed_weights, b[nr_block_start + nr_block_offset]);
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
        }
      } else {
        size_t n = nr_block_size;
        do {
          unaligned_store_s32(packed_weights, 0);
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
        } while (--n != 0);
      }
      packed_weights = (void*) ((uintptr_t) packed_weights + (nr - nr_block_size) * sizeof(int32_t));

      for (size_t ki = 0; ki < ks; ki++) {
        for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
          for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
            uint32_t ksum = 0;
            for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
              const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
              if (kc_idx < kc) {
                const int8_t kv = k[((nr_block_start + nr_block_offset) * ks + ki) * kc + kc_idx];
                ksum += (uint32_t) kv;
                ((int8_t*) packed_weights)[kr_block_offset] = kv;
              }
            }
            unaligned_indexed_store_u32(packed_b, nr_block_offset, unaligned_indexed_load_u32(packed_b, nr_block_offset) - ksum * izp);
            packed_weights = (int8_t*) packed_weights + kr;
          }
          packed_weights = (int8_t*) packed_weights + (nr - nr_block_size) * kr;
        }
      }
      packed_weights = (void*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += ks * kc * nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  } while (--g != 0);
}

void xnn_pack_f32_conv_kgo_w(
  size_t g,
  size_t nc,
  size_t ks,
  size_t nr,
  size_t kr,
  size_t sr,
  const float* k,
  const float* b,
  float* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t i = 0; i < g; i++) {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          packed_weights[nr_block_offset] = b[nr_block_start + nr_block_offset];
        }
      }
      packed_weights += nr;

      for (size_t ki = 0; ki < ks; ki++) {
        for (size_t sr_block_offset = 0; sr_block_offset < sr; sr_block_offset++) {
          for (size_t nr_block_offset = (-sr_block_offset) & (sr - 1); nr_block_offset < nr_block_size; nr_block_offset += sr) {
            packed_weights[nr_block_offset * kr] = k[ki * g * nc + (nr_block_start + nr_block_offset)];
          }
          packed_weights += nr * kr;
        }
      }
      packed_weights = (float*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  }
}

void xnn_pack_f16_conv_kgo_w(
  size_t g,
  size_t nc,
  size_t ks,
  size_t nr,
  size_t kr,
  size_t sr,
  const uint16_t* k,
  const uint16_t* b,
  uint16_t* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t i = 0; i < g; i++) {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          packed_weights[nr_block_offset] = b[nr_block_start + nr_block_offset];
        }
      }
      packed_weights += nr;

      for (size_t ki = 0; ki < ks; ki++) {
        for (size_t sr_block_offset = 0; sr_block_offset < sr; sr_block_offset++) {
          for (size_t nr_block_offset = (-sr_block_offset) & (sr - 1); nr_block_offset < nr_block_size; nr_block_offset += sr) {
            packed_weights[nr_block_offset * kr] = k[ki * g * nc + (nr_block_start + nr_block_offset)];
          }
          packed_weights += nr * kr;
        }
      }
      packed_weights = (uint16_t*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  }
}

void xnn_pack_f32_to_f16_conv_kgo_w(
  size_t g,
  size_t nc,
  size_t ks,
  size_t nr,
  size_t kr,
  size_t sr,
  const float* k,
  const float* b,
  uint16_t* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t i = 0; i < g; i++) {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          packed_weights[nr_block_offset] = fp16_ieee_from_fp32_value(b[nr_block_start + nr_block_offset]);
        }
      }
      packed_weights += nr;

      for (size_t ki = 0; ki < ks; ki++) {
        for (size_t sr_block_offset = 0; sr_block_offset < sr; sr_block_offset++) {
          for (size_t nr_block_offset = (-sr_block_offset) & (sr - 1); nr_block_offset < nr_block_size; nr_block_offset += sr) {
            packed_weights[nr_block_offset * kr] = fp16_ieee_from_fp32_value(k[ki * g * nc + (nr_block_start + nr_block_offset)]);
          }
          packed_weights += nr * kr;
        }
      }
      packed_weights = (uint16_t*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  }
}

void xnn_pack_qu8_conv_kgo_w(
  size_t g,
  size_t nc,
  size_t ks,
  size_t nr,
  size_t kr,
  size_t sr,
  const uint8_t* k,
  const int32_t* b,
  void* packed_weights,
  size_t extra_bytes,
  const struct xnn_qu8_packing_params* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const int32_t izp = (int32_t) params->input_zero_point;
  const int32_t bzp = (int32_t) ks * izp * (int32_t) params->kernel_zero_point;
  for (size_t i = 0; i < g; i++) {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      int32_t* packed_b = (int32_t*) packed_weights;
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          unaligned_store_s32(packed_weights, bzp + b[nr_block_start + nr_block_offset]);
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
        }
      } else {
        size_t n = nr_block_size;
        do {
          unaligned_store_s32(packed_weights, bzp);
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
        } while (--n != 0);
      }
      packed_weights = (void*) ((uintptr_t) packed_weights + (nr - nr_block_size) * sizeof(int32_t));

      for (size_t ki = 0; ki < ks; ki++) {
        for (size_t sr_block_offset = 0; sr_block_offset < sr; sr_block_offset++) {
          for (size_t nr_block_offset = (-sr_block_offset) & (sr - 1); nr_block_offset < nr_block_size; nr_block_offset += sr) {
            const uint8_t kv = k[ki * g * nc + (nr_block_start + nr_block_offset)];
            ((uint8_t*) packed_weights)[nr_block_offset * kr] = kv;
            unaligned_indexed_store_s32(packed_b, nr_block_offset, unaligned_indexed_load_s32(packed_b, nr_block_offset) - (int32_t) kv * izp);
          }
          packed_weights = (uint8_t*) packed_weights + nr * kr;
        }
      }
      packed_weights = (void*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  }
}

void xnn_pack_qs8_conv_kgo_w(
  size_t g,
  size_t nc,
  size_t ks,
  size_t nr,
  size_t kr,
  size_t sr,
  const int8_t* k,
  const int32_t* b,
  void* packed_weights,
  size_t extra_bytes,
  const struct xnn_qs8_packing_params* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const uint32_t izp = (uint32_t) params->input_zero_point;
  for (size_t i = 0; i < g; i++) {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);
      int32_t* packed_b = (int32_t*) packed_weights;
      if XNN_LIKELY(b != NULL) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          unaligned_store_s32(packed_weights, b[nr_block_start + nr_block_offset]);
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
        }
      } else {
        size_t n = nr_block_size;
        do {
          unaligned_store_s32(packed_weights, 0);
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
        } while (--n != 0);
      }
      packed_weights = (void*) ((uintptr_t) packed_weights + (nr - nr_block_size) * sizeof(int32_t));

      for (size_t ki = 0; ki < ks; ki++) {
        for (size_t sr_block_offset = 0; sr_block_offset < sr; sr_block_offset++) {
          for (size_t nr_block_offset = (-sr_block_offset) & (sr - 1); nr_block_offset < nr_block_size; nr_block_offset += sr) {
            const int8_t kv = k[ki * g * nc + (nr_block_start + nr_block_offset)];
            ((int8_t*) packed_weights)[nr_block_offset * kr] = kv;
            unaligned_indexed_store_u32(packed_b, nr_block_offset, unaligned_indexed_load_u32(packed_b, nr_block_offset) - (uint32_t) kv * izp);
          }
          packed_weights = (int8_t*) packed_weights + nr * kr;
        }
      }
      packed_weights = (void*) ((uintptr_t) packed_weights + extra_bytes);
    }
    k += nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  }
}

void xnn_pack_f32_deconv_goki_w(
  size_t g,
  size_t nc,
  size_t kh,
  size_t kw,
  size_t kc,
  size_t sh,
  size_t sw,
  size_t nr,
  size_t kr,
  size_t sr,
  const float* k,
  const float* b,
  float* packed_weights,
  struct subconvolution_params* subconv_params,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  for (size_t i = 0; i < g; i++) {
    for (size_t oy = 0; oy < sh; oy++) {
      for (size_t ox = 0; ox < sw; ox++) {
        if (i == 0) {
          (*subconv_params++).weights = packed_weights;
        }
        for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
          const size_t nr_block_size = min(nc - nr_block_start, nr);
          if XNN_LIKELY(b != NULL) {
            for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
              packed_weights[nr_block_offset] = b[nr_block_start + nr_block_offset];
            }
          }
          packed_weights += nr;
          for (size_t ky = oy; ky < kh; ky += sh) {
            for (size_t kx = ox; kx < kw; kx += sw) {
              for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
                for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
                  for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
                    const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
                    if (kc_idx < kc) {
                      packed_weights[kr_block_offset] = k[(((nr_block_start + nr_block_offset) * kh + ky) * kw + kx) * kc + kc_idx];
                    }
                  }
                  packed_weights += kr;
                }
                packed_weights += (nr - nr_block_size) * kr;
              }
            }
          }
        }
      }
    }
    k += kh * kw * kc * nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  }
}

void xnn_pack_f16_deconv_goki_w(
  size_t g,
  size_t nc,
  size_t kh,
  size_t kw,
  size_t kc,
  size_t sh,
  size_t sw,
  size_t nr,
  size_t kr,
  size_t sr,
  const uint16_t* k,
  const uint16_t* b,
  uint16_t* packed_weights,
  struct subconvolution_params* subconv_params,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  for (size_t i = 0; i < g; i++) {
    for (size_t oy = 0; oy < sh; oy++) {
      for (size_t ox = 0; ox < sw; ox++) {
        if (i == 0) {
          (*subconv_params++).weights = packed_weights;
        }
        for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
          const size_t nr_block_size = min(nc - nr_block_start, nr);
          if XNN_LIKELY(b != NULL) {
            for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
              packed_weights[nr_block_offset] = b[nr_block_start + nr_block_offset];
            }
          }
          packed_weights += nr;
          for (size_t ky = oy; ky < kh; ky += sh) {
            for (size_t kx = ox; kx < kw; kx += sw) {
              for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
                for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
                  for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
                    const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
                    if (kc_idx < kc) {
                      packed_weights[kr_block_offset] = k[(((nr_block_start + nr_block_offset) * kh + ky) * kw + kx) * kc + kc_idx];
                    }
                  }
                  packed_weights += kr;
                }
                packed_weights += (nr - nr_block_size) * kr;
              }
            }
          }
        }
      }
    }
    k += kh * kw * kc * nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  }
}

void xnn_pack_f32_to_f16_deconv_goki_w(
  size_t g,
  size_t nc,
  size_t kh,
  size_t kw,
  size_t kc,
  size_t sh,
  size_t sw,
  size_t nr,
  size_t kr,
  size_t sr,
  const float* k,
  const float* b,
  uint16_t* packed_weights,
  struct subconvolution_params* subconv_params,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  for (size_t i = 0; i < g; i++) {
    for (size_t oy = 0; oy < sh; oy++) {
      for (size_t ox = 0; ox < sw; ox++) {
        if (i == 0) {
          (*subconv_params++).weights = packed_weights;
        }
        for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
          const size_t nr_block_size = min(nc - nr_block_start, nr);
          if XNN_LIKELY(b != NULL) {
            for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
              packed_weights[nr_block_offset] = fp16_ieee_from_fp32_value(b[nr_block_start + nr_block_offset]);
            }
          }
          packed_weights += nr;
          for (size_t ky = oy; ky < kh; ky += sh) {
            for (size_t kx = ox; kx < kw; kx += sw) {
              for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
                for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
                  for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
                    const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
                    if (kc_idx < kc) {
                      packed_weights[kr_block_offset] = fp16_ieee_from_fp32_value(k[(((nr_block_start + nr_block_offset) * kh + ky) * kw + kx) * kc + kc_idx]);
                    }
                  }
                  packed_weights += kr;
                }
                packed_weights += (nr - nr_block_size) * kr;
              }
            }
          }
        }
      }
    }
    k += kh * kw * kc * nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  }
}

void xnn_pack_qs8_deconv_goki_w(
  size_t g,
  size_t nc,
  size_t kh,
  size_t kw,
  size_t kc,
  size_t sh,
  size_t sw,
  size_t nr,
  size_t kr,
  size_t sr,
  const int8_t* k,
  const int32_t* b,
  void* packed_weights,
  struct subconvolution_params* subconv_params,
  const struct xnn_qs8_packing_params* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  const uint32_t izp = (uint32_t) params->input_zero_point;
  for (size_t i = 0; i < g; i++) {
    for (size_t oy = 0; oy < sh; oy++) {
      for (size_t ox = 0; ox < sw; ox++) {
        if (i == 0) {
          (*subconv_params++).weights = packed_weights;
        }
        for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
          const size_t nr_block_size = min(nc - nr_block_start, nr);
          int32_t* packed_b = (int32_t*) packed_weights;
          if XNN_LIKELY(b != 0) {
            for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
              unaligned_store_s32(packed_weights, b[nr_block_start + nr_block_offset]);
              packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
            }
          } else {
            size_t n = nr_block_size;
            do {
              unaligned_store_s32(packed_weights, 0);
              packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
            } while (--n != 0);
          }
          packed_weights = (void*) ((uintptr_t) packed_weights + (nr - nr_block_size) * sizeof(int32_t));
          for (size_t ky = oy; ky < kh; ky += sh) {
            for (size_t kx = ox; kx < kw; kx += sw) {
              for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
                for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
                  uint32_t ksum = 0;
                  for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
                    const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
                    if (kc_idx < kc) {
                      const int8_t kv = k[(((nr_block_start + nr_block_offset) * kh + ky) * kw + kx) * kc + kc_idx];
                      ksum += (uint32_t) kv;
                      ((int8_t*) packed_weights)[kr_block_offset] = kv;
                    }
                  }
                  unaligned_indexed_store_u32(packed_b, nr_block_offset, unaligned_indexed_load_u32(packed_b, nr_block_offset) - ksum * izp);
                  packed_weights = (int8_t*) packed_weights + kr;
                }
                packed_weights = (int8_t*) packed_weights + (nr - nr_block_size) * kr;
              }
            }
          }
        }
      }
    }
    k += kh * kw * kc * nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  }
}

void xnn_pack_qu8_deconv_goki_w(
  size_t g,
  size_t nc,
  size_t kh,
  size_t kw,
  size_t kc,
  size_t sh,
  size_t sw,
  size_t nr,
  size_t kr,
  size_t sr,
  const uint8_t* k,
  const int32_t* b,
  void* packed_weights,
  struct subconvolution_params* subconv_params,
  const struct xnn_qu8_packing_params* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  const int32_t izp = (int32_t) params->input_zero_point;
  const int32_t kzp = (int32_t) params->kernel_zero_point;
  for (size_t i = 0; i < g; i++) {
    for (size_t oy = 0; oy < sh; oy++) {
      for (size_t ox = 0; ox < sw; ox++) {
        if (i == 0) {
          (*subconv_params++).weights = packed_weights;
        }
        const int32_t bzp = (int32_t) divide_round_up(kh - oy, sh) * (int32_t) divide_round_up(kw - ox, sw) * (int32_t) kc * izp * kzp;
        for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
          const size_t nr_block_size = min(nc - nr_block_start, nr);
          int32_t* packed_b = (int32_t*) packed_weights;
          if XNN_LIKELY(b != 0) {
            for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
              unaligned_store_s32(packed_weights, bzp + b[nr_block_start + nr_block_offset]);
              packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
            }
          } else {
            size_t n = nr_block_size;
            do {
              unaligned_store_s32(packed_weights, bzp);
              packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
            } while (--n != 0);
          }
          packed_weights = (void*) ((uintptr_t) packed_weights + (nr - nr_block_size) * sizeof(int32_t));
          for (size_t ky = oy; ky < kh; ky += sh) {
            for (size_t kx = ox; kx < kw; kx += sw) {
              for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
                for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
                  int32_t ksum = 0;
                  for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
                    const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
                    if (kc_idx < kc) {
                      const uint8_t kv = k[(((nr_block_start + nr_block_offset) * kh + ky) * kw + kx) * kc + kc_idx];
                      ksum += (int32_t) kv;
                      ((uint8_t*) packed_weights)[kr_block_offset] = kv;
                    }
                  }
                  unaligned_indexed_store_s32(packed_b, nr_block_offset, unaligned_indexed_load_s32(packed_b, nr_block_offset) - ksum * izp);
                  packed_weights = (uint8_t*) packed_weights + kr;
                }
                packed_weights = (uint8_t*) packed_weights + (nr - nr_block_size) * kr;
              }
            }
          }
        }
      }
    }
    k += kh * kw * kc * nc;
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nc;
    }
  }
}

void xnn_pack_f32_dwconv_ghw_w(
  size_t primary_tile,
  size_t h,
  size_t w,
  size_t c,
  size_t cr,
  const float* k,
  const float* b,
  float* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(primary_tile >= h * w);
  xnn_pack_f32_multipass_dwconv_ghw_w(
      primary_tile,
      /*middle_pass_tile=*/0,
      /*last_pass_tile=*/0,
      h,
      w,
      c,
      cr,
      k,
      b,
      packed_weights,
      extra_bytes,
      params);
}

void xnn_pack_f32_multipass_dwconv_ghw_w(
  size_t first_pass_tile,
  size_t middle_pass_tile,
  size_t last_pass_tile,
  size_t h,
  size_t w,
  size_t c,
  size_t cr,
  const float* k,
  const float* b,
  float* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);
  size_t kernel_size = h * w;
  if (middle_pass_tile == 0) {
    // Uni-pass DWCONV.
    assert(last_pass_tile == 0);
  } else {
    // Multi-pass DWCONV.
    assert(kernel_size > first_pass_tile);
  }

  // Stores the x and y index that should be processed next.
  size_t processed_x = 0;
  size_t processed_y = 0;
  size_t x = 0;
  size_t y = 0;

  // First pass. Pack in blocks of cr, cr biases, then first_pass_tile * cr
  // weights.
  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    const size_t cr_block_size = min(c - cr_block_start, cr);
    if XNN_LIKELY(b != NULL) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        *packed_weights++ = b[cr_block_start + cr_block_offset];
      }
    } else {
      size_t n = cr_block_size;
      do {
        *packed_weights++ = 0.0f;
      } while (--n != 0);
    }
    packed_weights += cr - cr_block_size;

    x = processed_x;
    y = processed_y;
    // kernel_size can be less than the first_pass_tile, in this case, pack up
    // to the smaller of the two.
    for (size_t i = 0; i < min(first_pass_tile, kernel_size); i++) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        const float kv = k[((cr_block_start + cr_block_offset) * h + y) * w + x];
        *packed_weights++ = kv;
      }
      packed_weights += cr - cr_block_size;
      if (++y == h) {
        y = 0;
        x++;
      }
    }
    // And make sure to skip weights if kernel_size < first_pass_tile.
    packed_weights += doz(first_pass_tile, kernel_size) * cr_block_size;
    packed_weights = (float*) ((uintptr_t) packed_weights + extra_bytes);
  }

  if (kernel_size <= first_pass_tile) {
    return;
  }

  kernel_size -= first_pass_tile;

  processed_x = x;
  processed_y = y;

  // Middle pass. (kernel_size / middle_pass_tile) blocks, within each block is
  // middle_pass_tile * cr weights.
  for (; kernel_size > last_pass_tile; kernel_size -= middle_pass_tile) {
    assert(kernel_size >= middle_pass_tile);
    for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
      x = processed_x;
      y = processed_y;
      const size_t cr_block_size = min(c - cr_block_start, cr);
      for (size_t j = 0; j < middle_pass_tile; j++) {
        for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
          const float kv = k[((cr_block_start + cr_block_offset) * h + y) * w + x];
          *packed_weights++ = kv;
        }
        packed_weights += cr - cr_block_size;
        if (++y == h) {
          y = 0;
          x++;
        }
      }
    }
    processed_x = x;
    processed_y = y;
  }

  // Last pass.
  assert(kernel_size <= last_pass_tile);
  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    x = processed_x;
    y = processed_y;
    const size_t cr_block_size = min(c - cr_block_start, cr);
    for (size_t i = 0; i < kernel_size; i++) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        const float kv = k[((cr_block_start + cr_block_offset) * h + y) * w + x];
        *packed_weights++ = kv;
      }
      packed_weights += cr - cr_block_size;
      if (++y == h) {
        y = 0;
        x++;
      }
    }
    // Pad so that we can always read last_pass_tile weights in the last pass.
    packed_weights += (last_pass_tile - kernel_size) * cr;
  }
}

void xnn_pack_f16_dwconv_ghw_w(
  size_t primary_tile,
  size_t h,
  size_t w,
  size_t c,
  size_t cr,
  const uint16_t* k,
  const uint16_t* b,
  uint16_t* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    const size_t cr_block_size = min(c - cr_block_start, cr);
    if XNN_LIKELY(b != NULL) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        *packed_weights++ = b[cr_block_start + cr_block_offset];
      }
    } else {
      size_t n = cr_block_size;
      do {
        *packed_weights++ = 0;
      } while (--n != 0);
    }
    packed_weights += cr - cr_block_size;
    for (size_t x = 0; x < w; x++) {
      for (size_t y = 0; y < h; y++) {
        for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
          const uint16_t kv = k[((cr_block_start + cr_block_offset) * h + y) * w + x];
          *packed_weights++ = kv;
        }
        packed_weights += cr - cr_block_size;
      }
    }
    packed_weights += (primary_tile - (h * w)) * cr_block_size;
    packed_weights = (uint16_t*) ((uintptr_t) packed_weights + extra_bytes);
  }
}

void xnn_pack_f32_to_f16_dwconv_ghw_w(
  size_t primary_tile,
  size_t h,
  size_t w,
  size_t c,
  size_t cr,
  const float* k,
  const float* b,
  uint16_t* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    const size_t cr_block_size = min(c - cr_block_start, cr);
    if XNN_LIKELY(b != NULL) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        *packed_weights++ = fp16_ieee_from_fp32_value(b[cr_block_start + cr_block_offset]);
      }
    } else {
      size_t n = cr_block_size;
      do {
        *packed_weights++ = 0;
      } while (--n != 0);
    }
    packed_weights += cr - cr_block_size;
    for (size_t x = 0; x < w; x++) {
      for (size_t y = 0; y < h; y++) {
        for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
          const uint16_t kv = fp16_ieee_from_fp32_value(k[((cr_block_start + cr_block_offset) * h + y) * w + x]);
          *packed_weights++ = kv;
        }
        packed_weights += cr - cr_block_size;
      }
    }
    packed_weights += (primary_tile - (h * w)) * cr_block_size;
    packed_weights = (uint16_t*) ((uintptr_t) packed_weights + extra_bytes);
  }
}

void xnn_pack_qu8_dwconv_ghw_w(
  size_t primary_tile,
  size_t h,
  size_t w,
  size_t c,
  size_t cr,
  const uint8_t* k,
  const int32_t* b,
  void* packed_weights,
  size_t extra_bytes,
  const struct xnn_qu8_packing_params* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  const int32_t izp = (int32_t) params->input_zero_point;
  const int32_t boff = (int32_t) h * (int32_t) w * izp * (int32_t) params->kernel_zero_point;
  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    const size_t cr_block_size = min(c - cr_block_start, cr);
    int32_t* packed_b = (int32_t*) packed_weights;
    if XNN_LIKELY(b != NULL) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        unaligned_store_s32(packed_weights, boff + b[cr_block_start + cr_block_offset]);
        packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
      }
    } else {
      size_t n = cr_block_size;
      do {
        unaligned_store_s32(packed_weights, boff);
        packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
      } while (--n != 0);
    }
    packed_weights = (void*) ((uintptr_t) packed_weights + (cr - cr_block_size) * sizeof(int32_t));
    for (size_t x = 0; x < w; x++) {
      for (size_t y = 0; y < h; y++) {
        for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
          const uint8_t kv = k[((cr_block_start + cr_block_offset) * h + y) * w + x];
          unaligned_indexed_store_s32(packed_b, cr_block_offset, unaligned_indexed_load_s32(packed_b, cr_block_offset) - (int32_t) kv * izp);
          *((uint8_t*) packed_weights) = kv;
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(uint8_t));
        }
        packed_weights = (void*) ((uintptr_t) packed_weights + (cr - cr_block_size) * sizeof(uint8_t));
      }
    }
    packed_weights = (void*) ((uintptr_t) packed_weights + (primary_tile - (h * w)) * cr_block_size * sizeof(uint8_t));
    packed_weights = (void*) ((uintptr_t) packed_weights + extra_bytes);
  }
}

void xnn_pack_qs8_dwconv_ghw_w(
  size_t primary_tile,
  size_t h,
  size_t w,
  size_t c,
  size_t cr,
  const int8_t* k,
  const int32_t* b,
  void* packed_weights,
  size_t extra_bytes,
  const struct xnn_qs8_packing_params* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  const uint32_t izp = (uint32_t) params->input_zero_point;
  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    const size_t cr_block_size = min(c - cr_block_start, cr);
    int32_t* packed_b = (int32_t*) packed_weights;
    if XNN_LIKELY(b != NULL) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        unaligned_store_s32(packed_weights, b[cr_block_start + cr_block_offset]);
        packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
      }
    } else {
      size_t n = cr_block_size;
      do {
        unaligned_store_s32(packed_weights, 0);
        packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
      } while (--n != 0);
    }
    packed_weights = (void*) ((uintptr_t) packed_weights + (cr - cr_block_size) * sizeof(int32_t));
    for (size_t x = 0; x < w; x++) {
      for (size_t y = 0; y < h; y++) {
        for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
          const int8_t kv = k[((cr_block_start + cr_block_offset) * h + y) * w + x];
          unaligned_indexed_store_u32(packed_b, cr_block_offset, unaligned_indexed_load_u32(packed_b, cr_block_offset) - (uint32_t) kv * izp);
          *((int8_t*) packed_weights) = kv;
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int8_t));
        }
        packed_weights = (void*) ((uintptr_t) packed_weights + (cr - cr_block_size) * sizeof(int8_t));
      }
    }
    packed_weights = (void*) ((uintptr_t) packed_weights + (primary_tile - (h * w)) * cr_block_size * sizeof(int8_t));
    packed_weights = (void*) ((uintptr_t) packed_weights + extra_bytes);
  }
}

void xnn_pack_f32_dwconv_hwg_w(
  size_t primary_tile,
  size_t h,
  size_t w,
  size_t c,
  size_t cr,
  const float* k,
  const float* b,
  float* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(primary_tile >= h * w);
  xnn_pack_f32_multipass_dwconv_hwg_w(
      primary_tile,
      /*middle_pass_tile=*/0,
      /*last_pass_tile=*/0,
      h,
      w,
      c,
      cr,
      k,
      b,
      packed_weights,
      extra_bytes,
      params);
}

void xnn_pack_f32_multipass_dwconv_hwg_w(
  size_t first_pass_tile,
  size_t middle_pass_tile,
  size_t last_pass_tile,
  size_t h,
  size_t w,
  size_t c,
  size_t cr,
  const float* k,
  const float* b,
  float* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);
  size_t kernel_size = h * w;
  if (middle_pass_tile == 0) {
    // Uni-pass DWCONV.
    assert(last_pass_tile == 0);
  } else {
    // Multi-pass DWCONV.
    assert(kernel_size > first_pass_tile);
  }


  // Stores the x and y index that should be processed next.
  size_t processed_x = 0;
  size_t processed_y = 0;
  size_t x = 0;
  size_t y = 0;

  // First pass. Pack in blocks of cr, cr biases, then first_pass_tile * cr
  // weights.
  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    const size_t cr_block_size = min(c - cr_block_start, cr);
    if XNN_LIKELY(b != NULL) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        *packed_weights++ = b[cr_block_start + cr_block_offset];
      }
    } else {
      size_t n = cr_block_size;
      do {
        *packed_weights++ = 0.0f;
      } while (--n != 0);
    }
    packed_weights += cr - cr_block_size;

    x = processed_x;
    y = processed_y;
    // kernel_size can be less than the first_pass_tile, in this case, pack up
    // to the smaller of the two.
    for (size_t i = 0; i < min(first_pass_tile, kernel_size); i++) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        const float kv = k[(y * w + x) * c + (cr_block_start + cr_block_offset)];
        *packed_weights++ = kv;
      }
      packed_weights += cr - cr_block_size;
      if (++y == h) {
        y = 0;
        x++;
      }
    }
    // And make sure to skip weights if kernel_size < first_pass_tile.
    packed_weights += doz(first_pass_tile, kernel_size) * cr_block_size;
    packed_weights = (float*) ((uintptr_t) packed_weights + extra_bytes);
  }

  if (kernel_size <= first_pass_tile) {
    return;
  }

  kernel_size -= first_pass_tile;

  processed_x = x;
  processed_y = y;

  // Middle pass. (kernel_size / middle_pass_tile) blocks, within each block is
  // middle_pass_tile * cr weights.
  for (; kernel_size > last_pass_tile; kernel_size -= middle_pass_tile) {
    assert(kernel_size >= middle_pass_tile);
    for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
      x = processed_x;
      y = processed_y;
      const size_t cr_block_size = min(c - cr_block_start, cr);
      for (size_t j = 0; j < middle_pass_tile; j++) {
        for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
          const float kv = k[(y * w + x) * c + (cr_block_start + cr_block_offset)];
          *packed_weights++ = kv;
        }
        packed_weights += cr - cr_block_size;
        if (++y == h) {
          y = 0;
          x++;
        }
      }
    }
    processed_x = x;
    processed_y = y;
  }

  // Last pass.
  assert(kernel_size <= last_pass_tile);
  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    x = processed_x;
    y = processed_y;
    const size_t cr_block_size = min(c - cr_block_start, cr);
    for (size_t i = 0; i < kernel_size; i++) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        const float kv = k[(y * w + x) * c + (cr_block_start + cr_block_offset)];
        *packed_weights++ = kv;
      }
      packed_weights += cr - cr_block_size;
      if (++y == h) {
        y = 0;
        x++;
      }
    }
    // Pad so that we can always read last_pass_tile weights in the last pass.
    packed_weights += (last_pass_tile - kernel_size) * cr;
  }
}

void xnn_pack_f16_dwconv_hwg_w(
  size_t primary_tile,
  size_t h,
  size_t w,
  size_t c,
  size_t cr,
  const uint16_t* k,
  const uint16_t* b,
  uint16_t* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    const size_t cr_block_size = min(c - cr_block_start, cr);
    if XNN_LIKELY(b != NULL) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        *packed_weights++ = b[cr_block_start + cr_block_offset];
      }
    } else {
      size_t n = cr_block_size;
      do {
        *packed_weights++ = 0;
      } while (--n != 0);
    }
    packed_weights += cr - cr_block_size;
    for (size_t x = 0; x < w; x++) {
      for (size_t y = 0; y < h; y++) {
        for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
          const uint16_t kv = k[(y * w + x) * c + (cr_block_start + cr_block_offset)];
          *packed_weights++ = kv;
        }
        packed_weights += cr - cr_block_size;
      }
    }
    packed_weights += (primary_tile - (h * w)) * cr_block_size;
    packed_weights = (uint16_t*) ((uintptr_t) packed_weights + extra_bytes);
  }
}

void xnn_pack_f32_to_f16_dwconv_hwg_w(
  size_t primary_tile,
  size_t h,
  size_t w,
  size_t c,
  size_t cr,
  const float* k,
  const float* b,
  uint16_t* packed_weights,
  size_t extra_bytes,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    const size_t cr_block_size = min(c - cr_block_start, cr);
    if XNN_LIKELY(b != NULL) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        *packed_weights++ = fp16_ieee_from_fp32_value(b[cr_block_start + cr_block_offset]);
      }
    } else {
      size_t n = cr_block_size;
      do {
        *packed_weights++ = 0;
      } while (--n != 0);
    }
    packed_weights += cr - cr_block_size;
    for (size_t x = 0; x < w; x++) {
      for (size_t y = 0; y < h; y++) {
        for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
          const uint16_t kv = fp16_ieee_from_fp32_value(k[(y * w + x) * c + (cr_block_start + cr_block_offset)]);
          *packed_weights++ = kv;
        }
        packed_weights += cr - cr_block_size;
      }
    }
    packed_weights += (primary_tile - (h * w)) * cr_block_size;
    packed_weights = (uint16_t*) ((uintptr_t) packed_weights + extra_bytes);
  }
}

void xnn_pack_qu8_dwconv_hwg_w(
  size_t primary_tile,
  size_t h,
  size_t w,
  size_t c,
  size_t cr,
  const uint8_t* k,
  const int32_t* b,
  void* packed_weights,
  size_t extra_bytes,
  const struct xnn_qu8_packing_params* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  const int32_t izp = (int32_t) params->input_zero_point;
  const int32_t boff = (int32_t) h * (int32_t) w * izp * (int32_t) params->kernel_zero_point;
  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    const size_t cr_block_size = min(c - cr_block_start, cr);
    int32_t* packed_b = (int32_t*) packed_weights;
    if XNN_LIKELY(b != NULL) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        unaligned_store_s32(packed_weights, boff + b[cr_block_start + cr_block_offset]);
        packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
      }
    } else {
      size_t n = cr_block_size;
      do {
        unaligned_store_s32(packed_weights, boff);
        packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
      } while (--n != 0);
    }
    packed_weights = (void*) ((uintptr_t) packed_weights + (cr - cr_block_size) * sizeof(int32_t));
    for (size_t x = 0; x < w; x++) {
      for (size_t y = 0; y < h; y++) {
        for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
          const uint8_t kv = k[(y * w + x) * c + (cr_block_start + cr_block_offset)];
          unaligned_indexed_store_s32(packed_b, cr_block_offset, unaligned_indexed_load_s32(packed_b, cr_block_offset) - (int32_t) kv * izp);
          *((uint8_t*) packed_weights) = kv;
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(uint8_t));
        }
        packed_weights = (void*) ((uintptr_t) packed_weights + (cr - cr_block_size) * sizeof(uint8_t));
      }
    }
    packed_weights = (void*) ((uintptr_t) packed_weights + (primary_tile - (h * w)) * cr_block_size * sizeof(uint8_t));
    packed_weights = (void*) ((uintptr_t) packed_weights + extra_bytes);
  }
}

void xnn_pack_qs8_dwconv_hwg_w(
  size_t primary_tile,
  size_t h,
  size_t w,
  size_t c,
  size_t cr,
  const int8_t* k,
  const int32_t* b,
  void* packed_weights,
  size_t extra_bytes,
  const struct xnn_qs8_packing_params* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  const uint32_t izp = (int32_t) params->input_zero_point;
  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    const size_t cr_block_size = min(c - cr_block_start, cr);
    int32_t* packed_b = (int32_t*) packed_weights;
    if XNN_LIKELY(b != NULL) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        unaligned_store_s32(packed_weights, b[cr_block_start + cr_block_offset]);
        packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
      }
    } else {
      size_t n = cr_block_size;
      do {
        unaligned_store_s32(packed_weights, 0);
        packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int32_t));
      } while (--n != 0);
    }
    packed_weights = (void*) ((uintptr_t) packed_weights + (cr - cr_block_size) * sizeof(int32_t));
    for (size_t x = 0; x < w; x++) {
      for (size_t y = 0; y < h; y++) {
        for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
          const int8_t kv = k[(y * w + x) * c + (cr_block_start + cr_block_offset)];
          unaligned_indexed_store_u32(packed_b, cr_block_offset, unaligned_indexed_load_u32(packed_b, cr_block_offset) - (uint32_t) kv * izp);
          *((int8_t*) packed_weights) = kv;
          packed_weights = (void*) ((uintptr_t) packed_weights + sizeof(int8_t));
        }
        packed_weights = (void*) ((uintptr_t) packed_weights + (cr - cr_block_size) * sizeof(int8_t));
      }
    }
    packed_weights = (void*) ((uintptr_t) packed_weights + (primary_tile - (h * w)) * cr_block_size * sizeof(int8_t));
    packed_weights = (void*) ((uintptr_t) packed_weights + extra_bytes);
  }
}

void xnn_pack_f32_gemminc_goi_w(
  size_t g,
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const float* k,
  float* packed_weights,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  do {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);

      for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
            const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
            if (kc_idx < kc) {
              packed_weights[kr_block_offset] = k[(nr_block_start + nr_block_offset) * kc + kc_idx];
            }
          }
          packed_weights += kr;
        }
        packed_weights += (nr - nr_block_size) * kr;
      }
    }
    k += nc * kc;
  } while (--g != 0);
}

void xnn_pack_f16_gemminc_goi_w(
  size_t g,
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kr,
  size_t sr,
  const uint16_t* k,
  uint16_t* packed_weights,
  const void* params)
{
  assert(g != 0);
  assert(nr >= sr);
  assert(k != NULL);
  assert(packed_weights != NULL);

  const size_t skr = sr * kr;
  do {
    for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
      const size_t nr_block_size = min(nc - nr_block_start, nr);

      for (size_t kr_block_start = 0; kr_block_start < round_up_po2(kc, skr); kr_block_start += kr) {
        for (size_t nr_block_offset = 0; nr_block_offset < nr_block_size; nr_block_offset++) {
          for (size_t kr_block_offset = 0; kr_block_offset < kr; kr_block_offset++) {
            const size_t kc_idx = round_down_po2(kr_block_start, skr) + ((kr_block_start + kr_block_offset + nr_block_offset * kr) & (skr - 1));
            if (kc_idx < kc) {
              packed_weights[kr_block_offset] = k[(nr_block_start + nr_block_offset) * kc + kc_idx];
            }
          }
          packed_weights += kr;
        }
        packed_weights += (nr - nr_block_size) * kr;
      }
    }
    k += nc * kc;
  } while (--g != 0);
}

void xnn_pack_f32_dconv_oki_w(
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kh,
  size_t kw,
  const float* k,
  const float* b,
  float* packed_weights,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
    const size_t nr_block_size = min(nc - nr_block_start, nr);
    if XNN_LIKELY(b != NULL) {
      for (size_t nr_block_offset = 0; nr_block_offset < nr; nr_block_offset++) {
        *packed_weights++ = b[min(nr_block_offset, nr_block_size - 1)];
      }
    } else {
      size_t n = nr;
      do {
        *packed_weights++ = 0.0f;
      } while (--n != 0);
    }

    for (size_t kx = 0; kx < kw; kx++) {
      for (size_t c = 0; c < kc; c++) {
        for (size_t ky = 0; ky < kh; ky++) {
          for (size_t nr_block_offset = 0; nr_block_offset < nr; nr_block_offset++) {
            *packed_weights++ = k[(((nr_block_start + min(nr_block_offset, nr_block_size - 1)) * kh + ky) * kw + kx) * kc + c];
          }
        }
      }
    }
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nr;
    }
  }
}

void xnn_pack_f32_to_f16_dconv_oki_w(
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kh,
  size_t kw,
  const float* k,
  const float* b,
  uint16_t* packed_weights,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
    const size_t nr_block_size = min(nc - nr_block_start, nr);
    if XNN_LIKELY(b != NULL) {
      for (size_t nr_block_offset = 0; nr_block_offset < nr; nr_block_offset++) {
        *packed_weights++ = fp16_ieee_from_fp32_value(b[min(nr_block_offset, nr_block_size - 1)]);
      }
    } else {
      size_t n = nr;
      do {
        *packed_weights++ = 0;
      } while (--n != 0);
    }

    for (size_t kx = 0; kx < kw; kx++) {
      for (size_t c = 0; c < kc; c++) {
        for (size_t ky = 0; ky < kh; ky++) {
          for (size_t nr_block_offset = 0; nr_block_offset < nr; nr_block_offset++) {
            *packed_weights++ = fp16_ieee_from_fp32_value(k[(((nr_block_start + min(nr_block_offset, nr_block_size - 1)) * kh + ky) * kw + kx) * kc + c]);
          }
        }
      }
    }
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nr;
    }
  }
}

void xnn_pack_f16_dconv_oki_w(
  size_t nc,
  size_t kc,
  size_t nr,
  size_t kh,
  size_t kw,
  const uint16_t* k,
  const uint16_t* b,
  uint16_t* packed_weights,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t nr_block_start = 0; nr_block_start < nc; nr_block_start += nr) {
    const size_t nr_block_size = min(nc - nr_block_start, nr);
    if XNN_LIKELY(b != NULL) {
      for (size_t nr_block_offset = 0; nr_block_offset < nr; nr_block_offset++) {
        *packed_weights++ = b[min(nr_block_offset, nr_block_size - 1)];
      }
    } else {
      size_t n = nr;
      do {
        *packed_weights++ = 0;
      } while (--n != 0);
    }

    for (size_t kx = 0; kx < kw; kx++) {
      for (size_t c = 0; c < kc; c++) {
        for (size_t ky = 0; ky < kh; ky++) {
          for (size_t nr_block_offset = 0; nr_block_offset < nr; nr_block_offset++) {
            *packed_weights++ = k[(((nr_block_start + min(nr_block_offset, nr_block_size - 1)) * kh + ky) * kw + kx) * kc + c];
          }
        }
      }
    }
    if XNN_UNPREDICTABLE(b != NULL) {
      b += nr;
    }
  }
}

void xnn_pack_f32_chw_dwconv_ghw_w(
  size_t kernel_size,
  size_t groups,
  const float* k,
  const float* b,
  float* packed_weights,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t g = 0; g < groups; g++) {
    if XNN_LIKELY(b != NULL) {
      *packed_weights = *b++;
    } else {
      *packed_weights = 0.0f;
    }
    packed_weights += 1;
    for (size_t i = 0; i < kernel_size; i++) {
      *packed_weights++ = k[g * kernel_size + i];
    }
  }
}

void xnn_pack_f32_to_f16_chw_dwconv_ghw_w(
  size_t kernel_size,
  size_t groups,
  const float* k,
  const float* b,
  uint16_t* packed_weights,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t g = 0; g < groups; g++) {
    if XNN_LIKELY(b != NULL) {
      *packed_weights = fp16_ieee_from_fp32_value(*b++);
    } else {
      *packed_weights = 0;
    }
    packed_weights += 1;
    for (size_t i = 0; i < kernel_size; i++) {
      *packed_weights++ = fp16_ieee_from_fp32_value(k[g * kernel_size + i]);
    }
  }
}

void xnn_pack_f16_chw_dwconv_ghw_w(
  size_t kernel_size,
  size_t groups,
  const uint16_t* k,
  const uint16_t* b,
  uint16_t* packed_weights,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t g = 0; g < groups; g++) {
    if XNN_LIKELY(b != NULL) {
      *packed_weights = *b++;
    } else {
      *packed_weights = 0;
    }
    packed_weights += 1;
    for (size_t i = 0; i < kernel_size; i++) {
      *packed_weights++ = k[g * kernel_size + i];
    }
  }
}

void xnn_pack_f32_chw_dwconv_hwg_w(
  size_t kernel_size,
  size_t groups,
  const float* k,
  const float* b,
  float* packed_weights,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t g = 0; g < groups; g++) {
    if XNN_LIKELY(b != NULL) {
      *packed_weights = *b++;
    } else {
      *packed_weights = 0.0f;
    }
    packed_weights += 1;
    for (size_t i = 0; i < kernel_size; i++) {
      *packed_weights++ = k[i * groups + g];
    }
  }
}

void xnn_pack_f16_chw_dwconv_hwg_w(
  size_t kernel_size,
  size_t groups,
  const uint16_t* k,
  const uint16_t* b,
  uint16_t* packed_weights,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t g = 0; g < groups; g++) {
    if XNN_LIKELY(b != NULL) {
      *packed_weights = *b++;
    } else {
      *packed_weights = 0;
    }
    packed_weights += 1;
    for (size_t i = 0; i < kernel_size; i++) {
      *packed_weights++ = k[i * groups + g];
    }
  }
}

void xnn_pack_f32_to_f16_chw_dwconv_hwg_w(
  size_t kernel_size,
  size_t groups,
  const float* k,
  const float* b,
  uint16_t* packed_weights,
  const void* params)
{
  assert(k != NULL);
  assert(packed_weights != NULL);

  for (size_t g = 0; g < groups; g++) {
    if XNN_LIKELY(b != NULL) {
      *packed_weights = fp16_ieee_from_fp32_value(*b++);
    } else {
      *packed_weights = 0;
    }
    packed_weights += 1;
    for (size_t i = 0; i < kernel_size; i++) {
      *packed_weights++ = fp16_ieee_from_fp32_value(k[i * groups + g]);
    }
  }
}


void xnn_pack_f32_vmulcaddc_w(
  size_t c,
  size_t cr,
  const float* s,
  const float* b,
  float* packed_weights,
  const void* params)
{
  assert(s != NULL);
  assert(packed_weights != NULL);

  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    const size_t cr_block_size = min(c - cr_block_start, cr);
    for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
      *packed_weights++ = s[cr_block_start + cr_block_offset];
    }
    packed_weights += cr - cr_block_size;
    if XNN_LIKELY(b != NULL) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        *packed_weights++ = b[cr_block_start + cr_block_offset];
      }
    } else {
      size_t n = cr_block_size;
      do {
        *packed_weights++ = 0.0f;
      } while (--n != 0);
    }
    packed_weights += cr - cr_block_size;
  }
}

void xnn_pack_f16_vmulcaddc_w(
  size_t c,
  size_t cr,
  const uint16_t* s,
  const uint16_t* b,
  uint16_t* packed_weights,
  const void* params)
{
  assert(s != NULL);
  assert(packed_weights != NULL);

  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    const size_t cr_block_size = min(c - cr_block_start, cr);
    for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
      *packed_weights++ = s[cr_block_start + cr_block_offset];
    }
    packed_weights += cr - cr_block_size;
    if XNN_LIKELY(b != NULL) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        *packed_weights++ = b[cr_block_start + cr_block_offset];
      }
    } else {
      size_t n = cr_block_size;
      do {
        *packed_weights++ = 0;
      } while (--n != 0);
    }
    packed_weights += cr - cr_block_size;
  }
}

void xnn_pack_f32_to_f16_vmulcaddc_w(
  size_t c,
  size_t cr,
  const float* s,
  const float* b,
  uint16_t* packed_weights,
  const void* params)
{
  assert(s != NULL);
  assert(packed_weights != NULL);

  for (size_t cr_block_start = 0; cr_block_start < c; cr_block_start += cr) {
    const size_t cr_block_size = min(c - cr_block_start, cr);
    for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
      *packed_weights++ = fp16_ieee_from_fp32_value(s[cr_block_start + cr_block_offset]);
    }
    packed_weights += cr - cr_block_size;
    if XNN_LIKELY(b != NULL) {
      for (size_t cr_block_offset = 0; cr_block_offset < cr_block_size; cr_block_offset++) {
        *packed_weights++ = fp16_ieee_from_fp32_value(b[cr_block_start + cr_block_offset]);
      }
    } else {
      size_t n = cr_block_size;
      do {
        *packed_weights++ = 0;
      } while (--n != 0);
    }
    packed_weights += cr - cr_block_size;
  }
}

void xnn_pack_f32_prelu_w(
  size_t c,
  const float* s,
  float* packed_weights)
{
  assert(s != NULL);
  assert(packed_weights != NULL);

  memcpy(packed_weights, s, c * sizeof(float));
}

void xnn_pack_f16_prelu_w(
  size_t c,
  const uint16_t* s,
  uint16_t* packed_weights)
{
  assert(s != NULL);
  assert(packed_weights != NULL);

  memcpy(packed_weights, s, c * sizeof(uint16_t));
}

void xnn_pack_f32_to_f16_prelu_w(
  size_t c,
  const float* s,
  uint16_t* packed_weights)
{
  assert(s != NULL);
  assert(packed_weights != NULL);

  do {
    *packed_weights++ = fp16_ieee_from_fp32_value(*s++);
  } while (--c != 0);
}
