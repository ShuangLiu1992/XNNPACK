// Auto-generated file. Do not edit!
//   Template: src/qu8-gemm/4x8c4-aarch64-neondot-cortex-a55.S.in
//   Generator: tools/xngen
//
// Copyright 2020 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.


#include <xnnpack/assembly.h>

# void xnn_qu8_gemm_minmax_rndnu_ukernel_4x8c4__aarch64_neondot_cortex_a55(
#     size_t mr,                 x0
#     size_t nc,                 x1
#     size_t kc,                 x2 / x0
#     const int8_t* restrict a,  x3
#     size_t a_stride,           x4
#     const void* restrict w,    x5
#     int8_t* restrict c,        x6
#     size_t cm_stride,          x7
#     size_t cn_stride,          [sp] -> x12
#     const union xnn_qu8_conv_minmax_params)  [sp + 8] -> x11

# d8-d15, x19-x30 need to be preserved if used. x18 is reserved by the OS.

// Register usage
// A0  x3  v0  v4
// A1 x15  v1  v5
// A2 x13  v2  v6
// A3  x4  v3  v7
// B   x5  v28  v29 v30 v31
// C0  x6 v16 v20
// C1  x8 v17 v21
// C2  x9 v18 v22
// C3  x7 v19 v23
// zero_point v24 v25 v26 v27 v8
// unused v12 v13 v14 v15 v29 v30 v31
// temp x14 for Cortex-A55 loads

BEGIN_FUNCTION xnn_qu8_gemm_minmax_rndnu_ukernel_4x8c4__aarch64_neondot_cortex_a55

        # Clamp A and C pointers
        CMP     x0, 2                   // if mr < 2
        ADD     x2, x2, 3               // kc = (kc + 3) & ~3
        ADD     x15, x3, x4             // a1 = a0 + a_stride
        ADD     x8, x6, x7              // c1 = c0 + cm_stride
        CSEL    x15, x3, x15, LO        //   a1 = a0
        CSEL    x8, x6,  x8, LO         //   c1 = c0
        BIC     x2, x2, 3

        LDP     x12, x11, [sp]          // cn_stride, params

        ADD     x13, x15, x4            // a2 = a1 + a_stride
        ADD     x9,  x8, x7             // c2 = c1 + cm_stride
        STR     d8, [sp, -16]!          // Save d8 on stack
                                        // if mr <= 2
        CSEL    x13, x15, x13, LS       //   a2 = a1
        CSEL    x9,  x8,  x9, LS        //   c2 = c1

        LD1R    {v8.4s}, [x11], 4       // kernel_zero_point

        CMP     x0, 4                   // if mr < 4
        ADD     x4, x13, x4             // a3 = a2 + a_stride
        ADD     x7,  x9, x7             // c3 = c2 + cm_stride
        CSEL    x4, x13, x4, LO         //   a3 = a2
        CSEL    x7,  x9, x7, LO         //   c3 = c2

        .p2align 3
0:
        # Load initial bias from w into accumulators
        LDP     q16, q20, [x5], 32
        MOV     v17.16b, v16.16b
        MOV     v18.16b, v16.16b
        MOV     v19.16b, v16.16b
        MOV     v21.16b, v20.16b
        MOV     v22.16b, v20.16b
        MOV     v23.16b, v20.16b
        SUBS    x0, x2, 16              // k = kc - 16
        MOVI    v24.16b, 0
        MOVI    v25.16b, 0
        MOVI    v26.16b, 0
        MOVI    v27.16b, 0

        # Is there at least 16 bytes for prologue/epilogue?
        B.LO    4f

        # prologue - read A and B values for block 0 and 1
        LDR     d0,  [x3], 8
        LDR     q28, [x5], 16
        LDR     d1, [x15], 8
        LDR     d2, [x13], 8
        LDR     d3,  [x4], 8
        SUBS    x0, x0, 16              // is there 16 for main loop?
        LDR     d29, [x5], 8
        LDR     x14, [x5], 8
        # Is there at least 16 bytes for main loop?
        B.LO    2f

        # Main loop - 16 bytes of A in 4 groups of 2 blocks
        # 4 row of 2 vectors wide = 8 UDOT instructions for 4 channels
        # 4 LD64 for A
        # 4 LD128 for W. = 2 LD64 + INS.
        # for each 4 UDOT, 1 LD64 for A, 2 LD64 for W + INS.

        .p2align 3
1:
        # BLOCK 0
        UDOT    v16.4s,  v28.16b, v0.4b[0]
        LDR     d30,  [x5], 8
        UDOT    v17.4s,  v28.16b, v1.4b[0]
        INS     v29.d[1], x14
        UDOT    v18.4s,  v28.16b, v2.4b[0]
        LDR     x14,  [x5], 8
        UDOT    v19.4s,  v28.16b, v3.4b[0]
        LDR     d4,  [x3], 8

        # BLOCK 1
        UDOT    v20.4s,  v29.16b, v0.4b[0]
        LDR     d31,  [x5], 8
        UDOT    v21.4s,  v29.16b, v1.4b[0]
        INS     v30.d[1], x14
        UDOT    v22.4s,  v29.16b, v2.4b[0]
        LDR     x14,  [x5], 8
        UDOT    v23.4s,  v29.16b, v3.4b[0]
        LDR     d5, [x15], 8

        # BLOCK 0
        UDOT    v16.4s, v30.16b, v0.4b[1]
        LDR     d28,  [x5], 8
        UDOT    v17.4s, v30.16b, v1.4b[1]
        INS     v31.d[1], x14
        UDOT    v18.4s, v30.16b, v2.4b[1]
        LDR     x14,  [x5], 8
        UDOT    v19.4s, v30.16b, v3.4b[1]
        LDR     d6, [x13], 8

        # BLOCK 1
        UDOT    v20.4s, v31.16b, v0.4b[1]
        LDR     d29,  [x5], 8
        UDOT    v21.4s, v31.16b, v1.4b[1]
        INS     v28.d[1], x14
        UDOT    v22.4s, v31.16b, v2.4b[1]
        LDR     x14,  [x5], 8
        UDOT    v23.4s, v31.16b, v3.4b[1]
        LDR     d7,  [x4], 8

        UDOT    v24.2s, v8.8b, v0.8b
        UDOT    v25.2s, v8.8b, v1.8b
        UDOT    v26.2s, v8.8b, v2.8b
        UDOT    v27.2s, v8.8b, v3.8b

        # BLOCK 0
        UDOT    v16.4s,  v28.16b, v4.4b[0]
        LDR     d30,  [x5], 8
        UDOT    v17.4s,  v28.16b, v5.4b[0]
        INS     v29.d[1], x14
        UDOT    v18.4s,  v28.16b, v6.4b[0]
        LDR     x14,  [x5], 8
        UDOT    v19.4s,  v28.16b, v7.4b[0]
        LDR     d0,  [x3], 8

        # BLOCK 1
        UDOT    v20.4s,  v29.16b, v4.4b[0]
        LDR     d31,  [x5], 8
        UDOT    v21.4s,  v29.16b, v5.4b[0]
        INS     v30.d[1], x14
        UDOT    v22.4s,  v29.16b, v6.4b[0]
        LDR     x14,  [x5], 8
        UDOT    v23.4s,  v29.16b, v7.4b[0]
        LDR     d1, [x15], 8

        # BLOCK 0
        UDOT    v16.4s, v30.16b, v4.4b[1]
        LDR     d28,  [x5], 8
        UDOT    v17.4s, v30.16b, v5.4b[1]
        INS     v31.d[1], x14
        UDOT    v18.4s, v30.16b, v6.4b[1]
        LDR     x14,  [x5], 8
        UDOT    v19.4s, v30.16b, v7.4b[1]
        LDR     d2, [x13], 8

        # BLOCK 1
        UDOT    v20.4s, v31.16b, v4.4b[1]
        LDR     d29,  [x5], 8
        UDOT    v21.4s, v31.16b, v5.4b[1]
        INS     v28.d[1], x14
        UDOT    v22.4s, v31.16b, v6.4b[1]
        LDR     x14,  [x5], 8
        UDOT    v23.4s, v31.16b, v7.4b[1]
        LDR     d3,  [x4], 8

        UDOT    v24.2s, v8.8b, v4.8b
        UDOT    v25.2s, v8.8b, v5.8b
        SUBS    x0, x0, 16
        UDOT    v26.2s, v8.8b, v6.8b
        UDOT    v27.2s, v8.8b, v7.8b

        B.HS    1b

        # Epilogue.  Same as main loop but no preloads in final group
2:
        # BLOCK 0
        UDOT    v16.4s,  v28.16b, v0.4b[0]
        LDR     d30,  [x5], 8
        UDOT    v17.4s,  v28.16b, v1.4b[0]
        INS     v29.d[1], x14
        UDOT    v18.4s,  v28.16b, v2.4b[0]
        LDR     x14,  [x5], 8
        UDOT    v19.4s,  v28.16b, v3.4b[0]
        LDR     d4,  [x3], 8

        # BLOCK 1
        UDOT    v20.4s,  v29.16b, v0.4b[0]
        LDR     d31,  [x5], 8
        UDOT    v21.4s,  v29.16b, v1.4b[0]
        INS     v30.d[1], x14
        UDOT    v22.4s,  v29.16b, v2.4b[0]
        LDR     x14,  [x5], 8
        UDOT    v23.4s,  v29.16b, v3.4b[0]
        LDR     d5, [x15], 8

        # BLOCK 0
        UDOT    v16.4s, v30.16b, v0.4b[1]
        LDR     d28,  [x5], 8
        UDOT    v17.4s, v30.16b, v1.4b[1]
        INS     v31.d[1], x14
        UDOT    v18.4s, v30.16b, v2.4b[1]
        LDR     x14,  [x5], 8
        UDOT    v19.4s, v30.16b, v3.4b[1]
        LDR     d6, [x13], 8

        # BLOCK 1
        UDOT    v20.4s, v31.16b, v0.4b[1]
        LDR     d29,  [x5], 8
        UDOT    v21.4s, v31.16b, v1.4b[1]
        INS     v28.d[1], x14
        UDOT    v22.4s, v31.16b, v2.4b[1]
        LDR     x14,  [x5], 8
        UDOT    v23.4s, v31.16b, v3.4b[1]
        LDR     d7,  [x4], 8

        UDOT    v24.2s, v8.8b, v0.8b
        UDOT    v25.2s, v8.8b, v1.8b
        UDOT    v26.2s, v8.8b, v2.8b
        UDOT    v27.2s, v8.8b, v3.8b

        # BLOCK 0
        UDOT    v16.4s,  v28.16b, v4.4b[0]
        LDR     d30,  [x5], 8
        UDOT    v17.4s,  v28.16b, v5.4b[0]
        INS     v29.d[1], x14
        UDOT    v18.4s,  v28.16b, v6.4b[0]
        LDR     x14,  [x5], 8
        UDOT    v19.4s,  v28.16b, v7.4b[0]

        # BLOCK 1
        UDOT    v20.4s,  v29.16b, v4.4b[0]
        LDR     d31,  [x5], 8
        UDOT    v21.4s,  v29.16b, v5.4b[0]
        INS     v30.d[1], x14
        UDOT    v22.4s,  v29.16b, v6.4b[0]
        LDR     x14,  [x5], 8
        UDOT    v23.4s,  v29.16b, v7.4b[0]

        # BLOCK 0
        UDOT    v16.4s, v30.16b, v4.4b[1]
        UDOT    v17.4s, v30.16b, v5.4b[1]
        INS     v31.d[1], x14
        UDOT    v18.4s, v30.16b, v6.4b[1]
        UDOT    v19.4s, v30.16b, v7.4b[1]

        # BLOCK 1
        UDOT    v20.4s, v31.16b, v4.4b[1]
        UDOT    v21.4s, v31.16b, v5.4b[1]
        UDOT    v22.4s, v31.16b, v6.4b[1]
        UDOT    v23.4s, v31.16b, v7.4b[1]

        AND     x0, x2, 15              // kc remainder 0 to 12

        UDOT    v24.2s, v8.8b, v4.8b
        UDOT    v25.2s, v8.8b, v5.8b
        UDOT    v26.2s, v8.8b, v6.8b
        UDOT    v27.2s, v8.8b, v7.8b

        # Is there a remainder?- 4 to 12 bytes of A
        CBNZ    x0, 4f

        .p2align 3
3:
        ADDP    v0.2s, v24.2s, v25.2s
        ADDP    v1.2s, v26.2s, v27.2s
        DUP     v24.4s, v0.s[0]
        DUP     v25.4s, v0.s[1]
        DUP     v26.4s, v1.s[0]
        DUP     v27.4s, v1.s[1]

        # Subtract zero point from accumulators
        SUB     v16.4s, v16.4s, v24.4s
        SUB     v17.4s, v17.4s, v25.4s
        SUB     v18.4s, v18.4s, v26.4s
        SUB     v19.4s, v19.4s, v27.4s
        SUB     v20.4s, v20.4s, v24.4s
        SUB     v21.4s, v21.4s, v25.4s
        SUB     v22.4s, v22.4s, v26.4s
        SUB     v23.4s, v23.4s, v27.4s

        # Apply params - preshift, scale, postshift, bias and clamp
        LD1R    {v4.4s}, [x11], 4
        SSHL    v16.4s, v16.4s, v4.4s   // shift to upper bits
        SSHL    v17.4s, v17.4s, v4.4s
        SSHL    v18.4s, v18.4s, v4.4s
        SSHL    v19.4s, v19.4s, v4.4s
        LD1R    {v5.4s}, [x11], 4
        SSHL    v20.4s, v20.4s, v4.4s
        SSHL    v21.4s, v21.4s, v4.4s
        SSHL    v22.4s, v22.4s, v4.4s
        SSHL    v23.4s, v23.4s, v4.4s
        LD1R    {v6.4s}, [x11], 4
        SQDMULH v16.4s, v16.4s, v5.4s   // scale without rounding
        SQDMULH v17.4s, v17.4s, v5.4s
        SQDMULH v18.4s, v18.4s, v5.4s
        SQDMULH v19.4s, v19.4s, v5.4s
        SQDMULH v20.4s, v20.4s, v5.4s
        SQDMULH v21.4s, v21.4s, v5.4s
        SQDMULH v22.4s, v22.4s, v5.4s
        SQDMULH v23.4s, v23.4s, v5.4s
        SRSHL   v16.4s, v16.4s, v6.4s   // signed rounding shift left
        SRSHL   v17.4s, v17.4s, v6.4s
        SRSHL   v18.4s, v18.4s, v6.4s
        SRSHL   v19.4s, v19.4s, v6.4s
        SRSHL   v20.4s, v20.4s, v6.4s
        SRSHL   v21.4s, v21.4s, v6.4s
        SRSHL   v22.4s, v22.4s, v6.4s
        SRSHL   v23.4s, v23.4s, v6.4s

        SQXTN   v16.4h, v16.4s
        SQXTN   v17.4h, v17.4s
        SQXTN   v18.4h, v18.4s
        SQXTN   v19.4h, v19.4s
        LD1R    {v6.8h}, [x11], 2       // add bias

        SQXTN2  v16.8h, v20.4s
        SQXTN2  v17.8h, v21.4s
        SQXTN2  v18.8h, v22.4s
        SQXTN2  v19.8h, v23.4s

        SQADD   v16.8h, v16.8h, v6.8h
        SQADD   v17.8h, v17.8h, v6.8h
        SQADD   v18.8h, v18.8h, v6.8h
        SQADD   v19.8h, v19.8h, v6.8h
        LD1R    {v4.16b}, [x11], 1      // clamp min value

        SQXTUN  v0.8b, v16.8h
        SQXTUN  v1.8b, v18.8h
        LD1R    {v5.16b}, [x11]         // clamp max value
        SQXTUN2 v0.16b, v17.8h
        SQXTUN2 v1.16b, v19.8h
        SUB     x11, x11, 15             // rewind params pointer

        UMAX    v0.16b, v0.16b, v4.16b
        UMAX    v1.16b, v1.16b, v4.16b
        SUBS    x1, x1, 8
        UMIN    v0.16b, v0.16b, v5.16b
        UMIN    v1.16b, v1.16b, v5.16b
        B.LO    6f

        # Store full 4 x 8
        ST1     {v0.8b}, [x6], x12
        SUB     x3,  x3, x2             // a0 -= kc
        ST1     {v0.d}[1], [x8], x12
        SUB     x15, x15, x2            // a1 -= kc
        ST1     {v1.8b}, [x9], x12
        SUB     x13, x13, x2            // a2 -= kc
        ST1     {v1.d}[1], [x7], x12
        SUB     x4,  x4, x2             // a3 -= kc
        B.NE    0b

        # Restore d8 from stack
        LDR     d8, [sp], 16
        RET

        # Remainder- 4 to 12 bytes of A
        # Although C4, its safe to read 16 bytes.
        .p2align 3
4:
        TBZ     x0, 3, 5f

        LDR     d0,  [x3], 8
        LDR     q4, [x5], 16
        LDR     d1, [x15], 8
        LDR     d2, [x13], 8
        LDR     d3,  [x4], 8
        LDR     q5, [x5], 16
        UDOT    v24.2s, v8.8b, v0.8b
        UDOT    v25.2s, v8.8b, v1.8b
        UDOT    v26.2s, v8.8b, v2.8b
        UDOT    v27.2s, v8.8b, v3.8b
        UDOT    v16.4s, v4.16b, v0.4b[0]
        UDOT    v17.4s, v4.16b, v1.4b[0]
        UDOT    v18.4s, v4.16b, v2.4b[0]
        UDOT    v19.4s, v4.16b, v3.4b[0]
        LDR     q6, [x5], 16
        UDOT    v20.4s, v5.16b, v0.4b[0]
        UDOT    v21.4s, v5.16b, v1.4b[0]
        UDOT    v22.4s, v5.16b, v2.4b[0]
        UDOT    v23.4s, v5.16b, v3.4b[0]
        LDR     q4, [x5], 16
        UDOT    v16.4s, v6.16b, v0.4b[1]
        UDOT    v17.4s, v6.16b, v1.4b[1]
        UDOT    v18.4s, v6.16b, v2.4b[1]
        UDOT    v19.4s, v6.16b, v3.4b[1]
        UDOT    v20.4s, v4.16b, v0.4b[1]
        UDOT    v21.4s, v4.16b, v1.4b[1]
        UDOT    v22.4s, v4.16b, v2.4b[1]
        UDOT    v23.4s, v4.16b, v3.4b[1]
        TBZ     x0, 2, 3b
5:
        LDR     s0,  [x3], 4
        LDR     q4, [x5], 16
        LDR     s1, [x15], 4
        LDR     s2, [x13], 4
        LDR     s3,  [x4], 4
        LDR     q5, [x5], 16
        UDOT    v24.2s, v8.8b, v0.8b
        UDOT    v25.2s, v8.8b, v1.8b
        UDOT    v26.2s, v8.8b, v2.8b
        UDOT    v27.2s, v8.8b, v3.8b
        UDOT    v16.4s, v4.16b, v0.4b[0]
        UDOT    v17.4s, v4.16b, v1.4b[0]
        UDOT    v18.4s, v4.16b, v2.4b[0]
        UDOT    v19.4s, v4.16b, v3.4b[0]
        UDOT    v20.4s, v5.16b, v0.4b[0]
        UDOT    v21.4s, v5.16b, v1.4b[0]
        UDOT    v22.4s, v5.16b, v2.4b[0]
        UDOT    v23.4s, v5.16b, v3.4b[0]
        B       3b

        # Store odd width
        .p2align 3
6:
        TBZ     x1, 2, 7f
        STR     s0, [x6], 4
        ST1     {v0.s}[2], [x8], 4
        STR     s1, [x9], 4
        ST1     {v1.s}[2], [x7], 4
        EXT     v0.16b, v0.16b, v0.16b, 4
        EXT     v1.16b, v1.16b, v1.16b, 4
7:
        TBZ     x1, 1, 8f
        STR     h0, [x6], 2
        ST1     {v0.h}[4], [x8], 2
        STR     h1, [x9], 2
        ST1     {v1.h}[4], [x7], 2
        EXT     v0.16b, v0.16b, v0.16b, 2
        EXT     v1.16b, v1.16b, v1.16b, 2
8:
        TBZ     x1, 0, 9f
        STR     b0, [x6]
        ST1     {v0.b}[8], [x8]
        STR     b1, [x9]
        ST1     {v1.b}[8], [x7]
9:
        # Restore d8 from stack
        LDR     d8, [sp], 16
        RET

END_FUNCTION xnn_qu8_gemm_minmax_rndnu_ukernel_4x8c4__aarch64_neondot_cortex_a55

#ifdef __ELF__
.section ".note.GNU-stack","",%progbits
#endif
