// Copyright 2022 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <stdbool.h>
#include <math.h>  // For INFINITY

#ifdef _WIN32
  #include <windows.h>

  #ifndef PF_ARM_V82_DP_INSTRUCTIONS_AVAILABLE
    #define PF_ARM_V82_DP_INSTRUCTIONS_AVAILABLE 43
  #endif
#else
  #include <pthread.h>
#endif

#ifndef __EMSCRIPTEN__
  #include <cpuinfo.h>
#endif

#include <xnnpack/common.h>
#include <xnnpack/config.h>
#include <xnnpack/log.h>


static struct xnn_hardware_config hardware_config = {0};

#if XNN_PLATFORM_WINDOWS
  static INIT_ONCE init_guard = INIT_ONCE_STATIC_INIT;
#else
  static pthread_once_t init_guard = PTHREAD_ONCE_INIT;
#endif

static void init_hardware_config(void) {
  #if XNN_ARCH_ARM
    hardware_config.use_arm_v6 = cpuinfo_has_arm_v6();
    hardware_config.use_arm_vfpv2 = cpuinfo_has_arm_vfpv2();
    hardware_config.use_arm_vfpv3 = cpuinfo_has_arm_vfpv3();
    hardware_config.use_arm_neon = cpuinfo_has_arm_neon();
    hardware_config.use_arm_neon_fp16 = cpuinfo_has_arm_neon_fp16();
    hardware_config.use_arm_neon_fma = cpuinfo_has_arm_neon_fma();
    hardware_config.use_arm_neon_v8 = cpuinfo_has_arm_neon_v8();
  #endif

  #if XNN_ARCH_ARM64 || XNN_ARCH_ARM
    #if XNN_PLATFORM_WINDOWS
      hardware_config.use_arm_fp16_arith = !!IsProcessorFeaturePresent(PF_ARM_V82_DP_INSTRUCTIONS_AVAILABLE);
      hardware_config.use_arm_neon_fp16_arith = !!IsProcessorFeaturePresent(PF_ARM_V82_DP_INSTRUCTIONS_AVAILABLE);
      hardware_config.use_arm_neon_bf16 = false;
      hardware_config.use_arm_neon_dot = !!IsProcessorFeaturePresent(PF_ARM_V82_DP_INSTRUCTIONS_AVAILABLE);
    #else
      hardware_config.use_arm_fp16_arith = cpuinfo_has_arm_fp16_arith();
      hardware_config.use_arm_neon_fp16_arith = cpuinfo_has_arm_neon_fp16_arith();
      hardware_config.use_arm_neon_bf16 = cpuinfo_has_arm_neon_bf16();
      hardware_config.use_arm_neon_dot = cpuinfo_has_arm_neon_dot();
    #endif
  #endif

  #if XNN_ARCH_X86 || XNN_ARCH_X86_64
    hardware_config.use_x86_ssse3 = cpuinfo_has_x86_ssse3();
    hardware_config.use_x86_sse4_1 = cpuinfo_has_x86_sse4_1();
    hardware_config.use_x86_avx = cpuinfo_has_x86_avx();
    hardware_config.use_x86_f16c = cpuinfo_has_x86_f16c();
    hardware_config.use_x86_fma3 = cpuinfo_has_x86_fma3();
    hardware_config.use_x86_xop = cpuinfo_has_x86_xop();
    hardware_config.use_x86_avx2 = cpuinfo_has_x86_avx2();
    hardware_config.use_x86_avx512f = cpuinfo_has_x86_avx512f();
    hardware_config.use_x86_avx512skx = hardware_config.use_x86_avx512f &&
      cpuinfo_has_x86_avx512bw() && cpuinfo_has_x86_avx512dq() && cpuinfo_has_x86_avx512vl();
    hardware_config.use_x86_avx512vbmi = hardware_config.use_x86_avx512skx && cpuinfo_has_x86_avx512vbmi();
  #endif  // !XNN_ARCH_X86 && !XNN_ARCH_X86_64

  #if XNN_ARCH_WASM || XNN_ARCH_WASMSIMD || XNN_ARCH_WASMRELAXEDSIMD
    // Unlike most other architectures, on x86/x86-64 when floating-point instructions
    // have no NaN arguments, but produce NaN output, the output NaN has sign bit set.
    // We use it to distinguish x86/x86-64 from other architectures, by doing subtraction
    // of two infinities (must produce NaN per IEEE 754 standard).
    static const volatile float inf = INFINITY;
    hardware_config.is_x86 = signbit(inf - inf);
  #endif  // XNN_ARCH_WASM || XNN_ARCH_WASMSIMD || XNN_ARCH_WASMRELAXEDSIMD
}

#if XNN_PLATFORM_WINDOWS
  static BOOL CALLBACK init_hardware_config_windows(PINIT_ONCE init_once, PVOID parameter, PVOID* context) {
    init_hardware_config();
    return TRUE;
  }
#endif

const struct xnn_hardware_config* xnn_init_hardware_config() {
  #if !XNN_PLATFORM_WEB && !XNN_ARCH_RISCV && !(XNN_ARCH_ARM64 && XNN_PLATFORM_WINDOWS)
    if (!cpuinfo_initialize()) {
      xnn_log_error("failed to initialize cpuinfo");
      return NULL;
    }
  #endif  // !XNN_PLATFORM_WEB && !XNN_ARCH_RISCV && !(XNN_ARCH_ARM64 && XNN_PLATFORM_WINDOWS)
  #if XNN_ARCH_ARM
    #if XNN_PLATFORM_MOBILE
      if (!cpuinfo_has_arm_neon()) {
        xnn_log_debug("unsupported hardware: ARM NEON not detected");
        return NULL;
      }
    #else
      if (!cpuinfo_has_arm_v6()) {
        xnn_log_debug("unsupported hardware: ARMv6 not detected");
        return NULL;
      }

      if (!cpuinfo_has_arm_vfpv2() && !cpuinfo_has_arm_vfpv3()) {
        xnn_log_debug("unsupported hardware: VFP FPU not detected");
        return NULL;
      }
    #endif
  #endif  // XNN_ARCH_ARM
  #if XNN_ARCH_X86
    if (!cpuinfo_has_x86_sse2()) {
      xnn_log_debug("unsupported hardware: SSE2 not detected");
      return NULL;
    }
  #endif  // XNN_ARCH_X86

  #if XNN_PLATFORM_WINDOWS
    InitOnceExecuteOnce(&init_guard, &init_hardware_config_windows, NULL, NULL);
  #else
    pthread_once(&init_guard, &init_hardware_config);
  #endif
  return &hardware_config;
}
