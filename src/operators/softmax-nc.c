// Copyright (c) Facebook, Inc. and its affiliates.
// All rights reserved.
//
// Copyright 2019 Google LLC
//
// This source code is licensed under the BSD-style license found in the
// LICENSE file in the root directory of this source tree.

#include <assert.h>
#include <math.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <fp16.h>

#include <xnnpack.h>
#include <xnnpack/allocator.h>
#include <xnnpack/operator.h>
#include <xnnpack/log.h>
#include <xnnpack/microparams-init.h>


enum xnn_status xnn_create_softmax_nc_qu8(
    size_t channels,
    size_t input_stride,
    size_t output_stride,
    float input_scale,
    uint8_t output_zero_point,
    float output_scale,
    uint32_t flags,
    xnn_operator_t* softmax_op_out)
{
  xnn_operator_t softmax_op = NULL;
  enum xnn_status status = xnn_status_uninitialized;

  if ((xnn_params.init_flags & XNN_INIT_FLAG_XNNPACK) == 0) {
    xnn_log_error("failed to create %s operator: XNNPACK is not initialized",
      xnn_operator_type_to_string(xnn_operator_type_sigmoid_nc_qu8));
    goto error;
  }

  status = xnn_status_invalid_parameter;

  if (channels == 0) {
    xnn_log_error(
      "failed to create %s operator with %zu channels: number of channels must be non-zero",
      xnn_operator_type_to_string(xnn_operator_type_sigmoid_nc_qu8), channels);
    goto error;
  }

  if (input_stride < channels) {
    xnn_log_error(
      "failed to create %s operator with input element stride of %zu: "
      "stride must be at least as large as the number of channels (%zu)",
      xnn_operator_type_to_string(xnn_operator_type_sigmoid_nc_qu8), input_stride, channels);
    goto error;
  }

  if (output_stride < channels) {
    xnn_log_error(
      "failed to create %s operator with output element stride of %zu: "
      "stride must be at least as large as the number of channels (%zu)",
      xnn_operator_type_to_string(xnn_operator_type_sigmoid_nc_qu8), output_stride, channels);
    goto error;
  }

  if (input_scale <= 0.0f || !isnormal(input_scale)) {
    xnn_log_error(
      "failed to create %s operator with %.7g input scale: scale must be finite, normalized, and positive",
      xnn_operator_type_to_string(xnn_operator_type_sigmoid_nc_qu8), input_scale);
    goto error;
  }

  if (output_scale <= 0.0f || !isnormal(output_scale)) {
    xnn_log_error(
      "failed to create %s operator with %.7g output scale: scale must be finite, normalized, and positive",
      xnn_operator_type_to_string(xnn_operator_type_sigmoid_nc_qu8), output_scale);
    goto error;
  }

  status = xnn_status_unsupported_parameter;

  if (output_scale != 0x1.0p-8f) {
    xnn_log_error(
      "failed to create %s operator with %.7g output scale: only output scale of 1/256 is supported",
      xnn_operator_type_to_string(xnn_operator_type_sigmoid_nc_qu8), output_scale);
    goto error;
  }

  if (output_zero_point != 0) {
    xnn_log_error(
      "failed to create %s operator with %" PRIu8 " output zero point: only output zero point of 0 is supported",
      xnn_operator_type_to_string(xnn_operator_type_sigmoid_nc_qu8), output_zero_point);
    goto error;
  }

  status = xnn_status_out_of_memory;

  softmax_op = xnn_allocate_zero_simd_memory(sizeof(struct xnn_operator));
  if (softmax_op == NULL) {
    xnn_log_error(
      "failed to allocate %zu bytes for %s operator descriptor",
      sizeof(struct xnn_operator), xnn_operator_type_to_string(xnn_operator_type_sigmoid_nc_qu8));
    goto error;
  }

  softmax_op->lookup_table = xnn_allocate_simd_memory(256 * sizeof(uint32_t));
  if (softmax_op->lookup_table == NULL) {
    xnn_log_error(
      "failed to allocate 256 bytes for %s operator lookup table",
      xnn_operator_type_to_string(xnn_operator_type_sigmoid_nc_qu8));
    goto error;
  }

  uint32_t* lookup_table = softmax_op->lookup_table;
  const double qscale = fmin(((double) UINT32_MAX) / (double) channels, 8388607.0);
  for (int32_t i = 0; i < 256; i++) {
    const double scaled_exp_xi = qscale * exp((double) (i - 255) * (double) input_scale);
    lookup_table[(uint32_t) i] = (uint32_t) lrint(scaled_exp_xi);
  }

  softmax_op->channels = channels;
  softmax_op->input_pixel_stride = input_stride;
  softmax_op->output_pixel_stride = output_stride;

  softmax_op->type = xnn_operator_type_softmax_nc_qu8;
  softmax_op->flags = flags;

  softmax_op->state = xnn_run_state_invalid;

  *softmax_op_out = softmax_op;
  return xnn_status_success;

error:
  xnn_delete_operator(softmax_op);
  return status;
}

enum xnn_status xnn_setup_softmax_nc_qu8(
    xnn_operator_t softmax_op,
    size_t batch_size,
    const uint8_t* input,
    uint8_t* output,
    pthreadpool_t threadpool)
{
  if (softmax_op->type != xnn_operator_type_softmax_nc_qu8) {
    xnn_log_error("failed to setup operator: operator type mismatch (expected %s, got %s)",
      xnn_operator_type_to_string(xnn_operator_type_softmax_nc_qu8),
      xnn_operator_type_to_string(softmax_op->type));
    return xnn_status_invalid_parameter;
  }
  softmax_op->state = xnn_run_state_invalid;

  if ((xnn_params.init_flags & XNN_INIT_FLAG_XNNPACK) == 0) {
    xnn_log_error("failed to setup %s operator: XNNPACK is not initialized",
      xnn_operator_type_to_string(xnn_operator_type_sigmoid_nc_qu8));
    return xnn_status_uninitialized;
  }

  if (batch_size == 0) {
    softmax_op->state = xnn_run_state_skip;
    return xnn_status_success;
  }

  softmax_op->batch_size = batch_size;
  softmax_op->input = input;
  softmax_op->output = output;

  softmax_op->context.u8_softmax = (struct u8_softmax_context) {
    .n = softmax_op->channels,
    .x = input,
    .x_stride = softmax_op->input_pixel_stride * sizeof(uint8_t),
    .t = softmax_op->lookup_table,
    .y = output,
    .y_stride = softmax_op->output_pixel_stride * sizeof(uint8_t),
    .rmax_ukernel = xnn_params.u8.rmax,
    .lut_norm_ukernel = xnn_params.u8.lut32norm,
  };
  softmax_op->compute.type = xnn_parallelization_type_1d;
  softmax_op->compute.task_1d = (pthreadpool_task_1d_t) xnn_compute_u8_softmax;
  softmax_op->compute.range[0] = batch_size;
  softmax_op->state = xnn_run_state_ready;

  return xnn_status_success;
}

static enum xnn_status create_softmax_nc_floating_point(
    size_t channels,
    size_t input_stride,
    size_t output_stride,
    uint32_t flags,
    uint32_t datatype_init_flags,
    enum xnn_operator_type operator_type,
    xnn_operator_t* softmax_op_out)
{
  xnn_operator_t softmax_op = NULL;
  enum xnn_status status = xnn_status_uninitialized;

  if ((xnn_params.init_flags & XNN_INIT_FLAG_XNNPACK) == 0) {
    xnn_log_error("failed to create %s operator: XNNPACK is not initialized",
      xnn_operator_type_to_string(operator_type));
    goto error;
  }

  status = xnn_status_unsupported_hardware;

  if ((xnn_params.init_flags & datatype_init_flags) != datatype_init_flags) {
    xnn_log_error("failed to create %s operator: operations on data type are not supported",
      xnn_operator_type_to_string(operator_type));
    goto error;
  }

  status = xnn_status_invalid_parameter;

  if (channels == 0) {
    xnn_log_error(
      "failed to create %s operator with %zu channels: number of channels must be non-zero",
      xnn_operator_type_to_string(operator_type), channels);
    goto error;
  }

  if (input_stride < channels) {
    xnn_log_error(
      "failed to create %s operator with input element stride of %zu: "
      "stride must be at least as large as the number of channels (%zu)",
      xnn_operator_type_to_string(operator_type), input_stride, channels);
    goto error;
  }

  if (output_stride < channels) {
    xnn_log_error(
      "failed to create %s operator with output element stride of %zu: "
      "stride must be at least as large as the number of channels (%zu)",
      xnn_operator_type_to_string(operator_type), output_stride, channels);
    goto error;
  }

  status = xnn_status_out_of_memory;

  softmax_op = xnn_allocate_zero_simd_memory(sizeof(struct xnn_operator));
  if (softmax_op == NULL) {
    xnn_log_error(
      "failed to allocate %zu bytes for %s operator descriptor",
      sizeof(struct xnn_operator), xnn_operator_type_to_string(operator_type));
    goto error;
  }

  softmax_op->channels = channels;
  softmax_op->input_pixel_stride = input_stride;
  softmax_op->output_pixel_stride = output_stride;

  softmax_op->type = operator_type;
  softmax_op->flags = flags;

  softmax_op->state = xnn_run_state_invalid;

  *softmax_op_out = softmax_op;
  return xnn_status_success;

error:
  xnn_delete_operator(softmax_op);
  return status;
}

enum xnn_status xnn_create_softmax_nc_f16(
    size_t channels,
    size_t input_stride,
    size_t output_stride,
    uint32_t flags,
    xnn_operator_t* softmax_op_out)
{
  return create_softmax_nc_floating_point(
    channels, input_stride, output_stride,
    flags,
    XNN_INIT_FLAG_F16,
    xnn_operator_type_softmax_nc_f16,
    softmax_op_out);
}

enum xnn_status xnn_create_softmax_nc_f32(
    size_t channels,
    size_t input_stride,
    size_t output_stride,
    uint32_t flags,
    xnn_operator_t* softmax_op_out)
{
  return create_softmax_nc_floating_point(
    channels, input_stride, output_stride,
    flags,
    XNN_INIT_FLAG_F32,
    xnn_operator_type_softmax_nc_f32,
    softmax_op_out);
}

static enum xnn_status setup_softmax_nc_floating_point(
    xnn_operator_t softmax_op,
    enum xnn_operator_type expected_operator_type,
    size_t batch_size,
    const void* input,
    void* output,
    uint32_t log2_element_size,
    xnn_rmax_ukernel_fn rmax,
    const struct raddstoreexpminusmax_parameters raddstoreexpminusmax[restrict XNN_MIN_ELEMENTS(1)],
    const struct vbinary_parameters vmul[restrict XNN_MIN_ELEMENTS(1)],
    xnn_compute_reciprocal_fn compute_reciprocal,
    const void* expminus_params,
    size_t expminus_params_size,
    const void* minmax_params,
    size_t minmax_params_size)
{
  if (softmax_op->type != expected_operator_type) {
    xnn_log_error("failed to setup operator: operator type mismatch (expected %s, got %s)",
      xnn_operator_type_to_string(expected_operator_type),
      xnn_operator_type_to_string(softmax_op->type));
    return xnn_status_invalid_parameter;
  }
  softmax_op->state = xnn_run_state_invalid;

  if ((xnn_params.init_flags & XNN_INIT_FLAG_XNNPACK) == 0) {
    xnn_log_error("failed to setup %s operator: XNNPACK is not initialized",
      xnn_operator_type_to_string(expected_operator_type));
    return xnn_status_uninitialized;
  }

  if (batch_size == 0) {
    softmax_op->state = xnn_run_state_skip;
    return xnn_status_success;
  }

  softmax_op->batch_size = batch_size;
  softmax_op->input = input;
  softmax_op->output = output;

  softmax_op->context.floating_point_softmax = (struct floating_point_softmax_context) {
    .n = softmax_op->channels << log2_element_size,
    .x = input,
    .x_stride = softmax_op->input_pixel_stride << log2_element_size,
    .y = output,
    .y_stride = softmax_op->output_pixel_stride << log2_element_size,
    .rmax_ukernel = rmax,
    .raddstoreexpminusmax_ukernel = raddstoreexpminusmax->ukernel,
    .compute_reciprocal = compute_reciprocal,
    .vmulc_ukernel = vmul->minmax.opc_ukernel,
  };
  if (vmul->linear.opc_ukernel != NULL) {
    softmax_op->context.floating_point_softmax.vmulc_ukernel = vmul->linear.opc_ukernel;
  };
  memcpy(&softmax_op->context.floating_point_softmax.expminus_params, expminus_params, expminus_params_size);
  memcpy(&softmax_op->context.floating_point_softmax.minmax_params, minmax_params, minmax_params_size);
  softmax_op->compute.type = xnn_parallelization_type_1d;
  softmax_op->compute.task_1d = (pthreadpool_task_1d_t) xnn_compute_floating_point_softmax;
  softmax_op->compute.range[0] = batch_size;
  softmax_op->state = xnn_run_state_ready;

  return xnn_status_success;
}

static void compute_reciprocal_f16(
    const uint16_t input[XNN_MIN_ELEMENTS(1)],
    uint16_t output[XNN_MIN_ELEMENTS(1)])
{
  *output = fp16_ieee_from_fp32_value(1.0f / fp16_ieee_to_fp32_value(*input));
}

enum xnn_status xnn_setup_softmax_nc_f16(
    xnn_operator_t softmax_op,
    size_t batch_size,
    const void* input,
    void* output,
    pthreadpool_t threadpool)
{
  union xnn_f16_expminus_params expminus_params;
  if (xnn_params.f16.raddstoreexpminusmax.init.f16 != NULL) {
    xnn_params.f16.raddstoreexpminusmax.init.f16(&expminus_params);
  }
  union xnn_f16_minmax_params minmax_params;
  if (xnn_params.f16.vmul.init.f16_minmax != NULL) {
    xnn_params.f16.vmul.init.f16_minmax(&minmax_params, UINT16_C(0xFC00), UINT16_C(0x7C00));
  }
  return setup_softmax_nc_floating_point(
    softmax_op, xnn_operator_type_softmax_nc_f16,
    batch_size, input, output,
    1 /* log2(sizeof(uint16_t)) */,
    xnn_params.f16.rmax, &xnn_params.f16.raddstoreexpminusmax, &xnn_params.f16.vmul,
    (xnn_compute_reciprocal_fn) compute_reciprocal_f16,
    &expminus_params, sizeof(expminus_params),
    &minmax_params, sizeof(minmax_params));
}

static void compute_reciprocal_f32(
    const float input[XNN_MIN_ELEMENTS(1)],
    float output[XNN_MIN_ELEMENTS(1)])
{
  *output = 1.0f / *input;
}

enum xnn_status xnn_setup_softmax_nc_f32(
    xnn_operator_t softmax_op,
    size_t batch_size,
    const float* input,
    float* output,
    pthreadpool_t threadpool)
{
  union xnn_f32_expminus_params expminus_params;
  if (xnn_params.f32.raddstoreexpminusmax.init.f32 != NULL) {
    xnn_params.f32.raddstoreexpminusmax.init.f32(&expminus_params);
  }
  union xnn_f32_minmax_params minmax_params;
  if (xnn_params.f32.vmul.init.f32_minmax != NULL) {
    xnn_params.f32.vmul.init.f32_minmax(&minmax_params, -INFINITY, INFINITY);
  }
  return setup_softmax_nc_floating_point(
    softmax_op, xnn_operator_type_softmax_nc_f32,
    batch_size, input, output,
    2 /* log2(sizeof(float)) */,
    xnn_params.f32.rmax, &xnn_params.f32.raddstoreexpminusmax, &xnn_params.f32.vmul,
    (xnn_compute_reciprocal_fn) compute_reciprocal_f32,
    &expminus_params, sizeof(expminus_params),
    &minmax_params, sizeof(minmax_params));
}
