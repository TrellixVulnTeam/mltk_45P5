/***************************************************************************//**
 * @file
 * @brief Silicon Labs Feature Generator Core Functionality
 *******************************************************************************
 * # License
 * <b>Copyright 2021 Silicon Laboratories Inc. www.silabs.com</b>
 *******************************************************************************
 *
 * SPDX-License-Identifier: Zlib
 *
 * The licensor of this software is Silicon Laboratories Inc.
 *
 * This software is provided 'as-is', without any express or implied
 * warranty. In no event will the authors be held liable for any damages
 * arising from the use of this software.
 *
 * Permission is granted to anyone to use this software for any purpose,
 * including commercial applications, and to alter it and redistribute it
 * freely, subject to the following restrictions:
 *
 * 1. The origin of this software must not be misrepresented; you must not
 *    claim that you wrote the original software. If you use this software
 *    in a product, an acknowledgment in the product documentation would be
 *    appreciated but is not required.
 * 2. Altered source versions must be plainly marked as such, and must not be
 *    misrepresented as being the original software.
 * 3. This notice may not be removed or altered from any source distribution.
 *
 ******************************************************************************/
#include <stdlib.h>
#include "sl_status.h"
#include "microfrontend/lib/frontend.h"
#include "microfrontend/lib/frontend_util.h"
#include "sl_ml_audio_feature_generation.h"
#include "sl_ml_audio_feature_generation_config.h"

/*******************************************************************************
 *********************************   DEFINES   *********************************
 ******************************************************************************/
#define AUDIO_BUFFER_SIZE           (SL_ML_AUDIO_FEATURE_GENERATION_AUDIO_BUFFER_SIZE)
#define FEATURE_BUFFER_SLICE_COUNT  (1 + ((SL_ML_FRONTEND_SAMPLE_LENGTH_MS - SL_ML_FRONTEND_WINDOW_SIZE_MS) / SL_ML_FRONTEND_WINDOW_STEP_MS)) 
#define FEATURE_BUFFER_SIZE         (SL_ML_FRONTEND_FILTERBANK_N_CHANNELS * FEATURE_BUFFER_SLICE_COUNT)                    

// Feature range min and max, used for determining valid range to quantize from
#define SL_ML_AUDIO_FEATURE_GENERATION_QUANTIZE_FEATURE_RANGE_MIN      0
#define SL_ML_AUDIO_FEATURE_GENERATION_QUANTIZE_FEATURE_RANGE_MAX      666     

/*******************************************************************************
 ***************************  LOCAL VARIABLES   ********************************
 ******************************************************************************/

// Audio buffer to store long term audio data, needs to be big enough to store 100ms of data
int16_t* sl_ml_audio_feature_generation_audio_buffer;
static int16_t* audio_buffer;

// Ring buffer to store feature
static uint16_t* feature_buffer;

// Buffer indices
static volatile size_t audio_buffer_read_index;
static volatile size_t audio_buffer_write_index;
static volatile size_t feature_buffer_start;

// Counter to maintain number of new and available slices
static size_t num_unfetched_slices = 0;

// Optional audio volume scaling factor
static int audio_volume_scaler;

// Frontend State
struct FrontendState frontend_state;

extern void register_dump_streams();
extern void dump_audio(const int16_t* buffer, int length);
extern void dump_raw_spectrogram(const uint16_t* buffer, int length);
extern void dump_quantized_spectrogram(const int8_t* buffer, int length);


/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

static sl_status_t process_audio_buffer_chunk(const int16_t *audio_data, size_t num_samples, size_t *slices_updated);

/*******************************************************************************
 **************************   GLOBAL FUNCTIONS   *******************************
 ******************************************************************************/

/***************************************************************************//**
 *  Initializes the frontend
 ******************************************************************************/
sl_status_t sl_ml_audio_feature_generation_frontend_init()
{
  audio_buffer = sl_ml_audio_feature_generation_audio_buffer;

  feature_buffer = malloc(FEATURE_BUFFER_SIZE * sizeof(uint16_t));
  if(feature_buffer == NULL) {
    return SL_STATUS_ALLOCATION_FAILED;
  }

  audio_volume_scaler = SL_ML_AUDIO_FEATURE_GENERATION_AUDIO_GAIN;

  // Initialize all elements of feature buffer to 0
  for (int i = 0; i < FEATURE_BUFFER_SIZE; i++) {
    feature_buffer[i] = 0;
  }

  // Set ring-buffer indices
  audio_buffer_read_index = 0;
  audio_buffer_write_index = 0;
  feature_buffer_start = 0;

  struct FrontendConfig config;

  // Window settings
  config.window.size_ms = SL_ML_FRONTEND_WINDOW_SIZE_MS;
  config.window.step_size_ms = SL_ML_FRONTEND_WINDOW_STEP_MS;

  // Filterbank settings
  config.filterbank.num_channels = SL_ML_FRONTEND_FILTERBANK_N_CHANNELS;
  config.filterbank.lower_band_limit = SL_ML_FRONTEND_FILTERBANK_LOWER_BAND_LIMIT;
  config.filterbank.upper_band_limit = SL_ML_FRONTEND_FILTERBANK_UPPER_BAND_LIMIT;

  // This is unused
  config.filterbank.output_scale_shift = 0;

  // Noise reduction settings
  config.noise_reduction.enable_noise_reduction = SL_ML_FRONTEND_NOISE_REDUCTION_ENABLE;
  config.noise_reduction.smoothing_bits = SL_ML_FRONTEND_NOISE_REDUCTION_SMOOTHING_BITS;
  config.noise_reduction.even_smoothing = SL_ML_FRONTEND_NOISE_REDUCTION_EVEN_SMOOTHING;
  config.noise_reduction.odd_smoothing = SL_ML_FRONTEND_NOISE_REDUCTION_ODD_SMOOTHING;
  config.noise_reduction.min_signal_remaining = SL_ML_FRONTEND_NOISE_REDUCTION_MIN_SIGNAL_REMAINING;

  // PCAN gain control settings
  config.pcan_gain_control.enable_pcan = SL_ML_FRONTEND_PCAN_ENABLE;
  config.pcan_gain_control.strength = SL_ML_FRONTEND_PCAN_STRENGTH;
  config.pcan_gain_control.offset = SL_ML_FRONTEND_PCAN_OFFSET;
  config.pcan_gain_control.gain_bits = SL_ML_FRONTEND_PCAN_GAIN_BITS;

  // Log scale settings
  config.log_scale.enable_log = SL_ML_FRONTEND_LOG_SCALE_ENABLE;
  config.log_scale.scale_shift = SL_ML_FRONTEND_LOG_SCALE_SHIFT;

  if (!FrontendPopulateState(&config, &frontend_state, SL_ML_FRONTEND_SAMPLE_RATE_HZ)) {
    return SL_STATUS_FAIL;
  }

  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Writes audio samples into the audio buffer of size one chunck
 *  Fills the next chunk of the audio buffer
 *
 *  @param[in] new_samples
 *    New audio samples to copy in
 *
 *  @param[in] num_samples
 *    Number of audio samples in new_samples, is configured the first
 *    time this callback is invoked, and must be the same for all subsequent calls.
 *
 *  @return
 *    SL_STATUS_OK success
 *    SL_STATUS_INVALID_PARAMETER num_samples is not equal to chunk size
 *
 ******************************************************************************/
sl_status_t sli_ml_audio_feature_generation_audio_buffer_write_chunk(const int16_t *new_samples, size_t num_samples)
{
  // TODO: We should signal an overflow here when write index passes the read index
  if(audio_volume_scaler > 1) {
    int index = audio_buffer_write_index;
    for (uint32_t i = 0; i < num_samples; i++) {
      audio_buffer[index] = audio_buffer[index] * audio_volume_scaler;
      index = (index + 1) % AUDIO_BUFFER_SIZE;
    }
  }

  audio_buffer_write_index = (audio_buffer_write_index + num_samples) % AUDIO_BUFFER_SIZE;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Updates the feature buffer with as many feature slices as can be calculated
 *  with new audio data.
 ******************************************************************************/
sl_status_t sl_ml_audio_feature_generation_update_features()
{
  size_t num_slices_updated = 0;

  register_dump_streams();

  int new_data_length = (audio_buffer_read_index < audio_buffer_write_index) ? 
    (audio_buffer_write_index - audio_buffer_read_index) : 
    ((AUDIO_BUFFER_SIZE - audio_buffer_read_index) + audio_buffer_write_index);

  if(new_data_length > 0)
  {
    const int length_to_end = AUDIO_BUFFER_SIZE - audio_buffer_read_index;
    const int chunk_length = (new_data_length < length_to_end) ? new_data_length : length_to_end;

    dump_audio(&audio_buffer[audio_buffer_read_index], chunk_length);
    process_audio_buffer_chunk(&audio_buffer[audio_buffer_read_index], chunk_length, &num_slices_updated);

    // Update read index
    audio_buffer_read_index = (audio_buffer_read_index + chunk_length) % (AUDIO_BUFFER_SIZE);
    new_data_length -= chunk_length;

    // If the buffer wrapped and there's still more data to be read
    if(new_data_length > 0)
    {
      dump_audio(&audio_buffer[0], new_data_length);
      process_audio_buffer_chunk(&audio_buffer[0], new_data_length, &num_slices_updated);

      // Update read index
      audio_buffer_read_index = new_data_length;
    }

  }


  if (num_slices_updated == 0) {
    // No new slices could be calculated
    return SL_STATUS_EMPTY;
  }

  num_unfetched_slices += num_slices_updated;

  dump_raw_spectrogram(feature_buffer, FEATURE_BUFFER_SIZE);


  // Could return SL_STATUS_NOT_READY if not full
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Retrieves the features as type uint16 and copies them to the provided buffer.
 ******************************************************************************/
sl_status_t sl_ml_audio_feature_generation_get_features_raw(uint16_t *buffer, size_t num_elements)
{
  if (num_elements != FEATURE_BUFFER_SIZE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  for (int i = 0; i < FEATURE_BUFFER_SIZE; i++) {
    const uint32_t capture_index = (feature_buffer_start + i) % FEATURE_BUFFER_SIZE;
    buffer[i] = feature_buffer[capture_index];
  }

  num_unfetched_slices = 0;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 * @brief
 *    Retrieves the features quantized to signed integer numbers in the range
 *    -128 to 127 (int8) and copies them to the provided buffer.
 *
 *    @ref range_min and @ref range_max set the valid range of which values are
 *    quantized from.
 ******************************************************************************/
sl_status_t sli_ml_audio_feature_generation_get_features_quantized(int8_t *buffer, 
                                                                   size_t num_elements, 
                                                                   uint16_t range_min, 
                                                                   uint16_t range_max)
{
  if (num_elements != FEATURE_BUFFER_SIZE) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  if (range_min == range_max) {
    return SL_STATUS_INVALID_PARAMETER;
  }

  for (int i = 0; i < FEATURE_BUFFER_SIZE; i++) {
    const int capture_index = (feature_buffer_start + i) % FEATURE_BUFFER_SIZE;
    const int32_t value_scale = 256; 
    const int32_t value_div = range_max - range_min;
    int32_t value = (((feature_buffer[capture_index] - range_min) * value_scale) + (value_div / 2))
                    / value_div;
    value -= 128;
    if (value < -128) {
      value = -128;
    }
    if (value > 127) {
      value = 127;
    }
    buffer[i] = (int8_t)value;
  }

  dump_quantized_spectrogram(buffer, num_elements);

  num_unfetched_slices = 0;
  return SL_STATUS_OK;
}

/***************************************************************************//**
 *  Fills a TensorFlow tensor with feature data
 ******************************************************************************/
sl_status_t sl_ml_audio_feature_generation_fill_tensor(TfLiteTensor *input_tensor)
{
  sl_status_t status = SL_STATUS_OK;
  if (input_tensor->type == kTfLiteInt8) {
    status = sli_ml_audio_feature_generation_get_features_quantized(input_tensor->data.int8,
                                                                   input_tensor->bytes, 
                                                                   SL_ML_AUDIO_FEATURE_GENERATION_QUANTIZE_FEATURE_RANGE_MIN, 
                                                                   SL_ML_AUDIO_FEATURE_GENERATION_QUANTIZE_FEATURE_RANGE_MAX);
  } else {
    status = SL_STATUS_INVALID_PARAMETER;
  }

  return status;
}

/***************************************************************************//**
 *    Returns how many new or unfetched feature slices that have been updated
 *    since last call to sl_ml_audio_feature_generation_get_features_raw or
 *    sl_ml_audio_feature_generation_fill_tensor;
 ******************************************************************************/
int sl_ml_audio_feature_generation_get_new_feature_slice_count()
{
  return num_unfetched_slices;
}

void sl_ml_audio_feature_generation_reset()
{
  // Clear buffers
  for (int i = 0; i < FEATURE_BUFFER_SIZE; i++) {
    feature_buffer[i] = 0;
  }
  for (int i = 0; i < AUDIO_BUFFER_SIZE; i++) {
    audio_buffer[i] = 0;
  }

  // Reset ring-buffer indices
  audio_buffer_read_index = 0;
  audio_buffer_write_index = 0;
  feature_buffer_start = 0;

  // Reset slice counter
  num_unfetched_slices = 0;

  FrontendReset(&frontend_state);
}

/***************************************************************************//**
 *    Returns the size of the feature buffer
 ******************************************************************************/
int sl_ml_audio_feature_generation_get_feature_buffer_size()
{
  return FEATURE_BUFFER_SIZE;
}

/*******************************************************************************
 **************************   LOCAL FUNCTIONS   ********************************
 ******************************************************************************/

/***************************************************************************//**
 *  Calculates features from a single chunk from the audio buffer
 *
 *  This function will consume all samples from the audio buffer chunk
 *  and generate as many feature slices as possible from this data. Any leftover
 *  audio samples will be consumed by Microfrontend and kept until the next call
 *  to generate features from then.
 *
 *  @param[in] audio_data
 *    Next chunk from the audio_data buffer to process
 * 
 * @param[in] num_samples
 *    Number of samples to process in the audio_data buffer
 *
 *  @param[out] slices_updated
 *    Adds how many slices were updated to this parameter
 ******************************************************************************/
static sl_status_t process_audio_buffer_chunk(const int16_t *audio_data, size_t num_samples, size_t *slices_updated)
{
  while (num_samples > 0) {
    size_t num_samples_read;
    struct FrontendOutput output = FrontendProcessSamples(
      &frontend_state, audio_data, num_samples, &num_samples_read);

    audio_data += num_samples_read;
    num_samples -= num_samples_read;
    if (output.values != NULL) {
      for (size_t i = 0; i < output.size; i++) {
        feature_buffer[feature_buffer_start + i] = output.values[i];
      }

      feature_buffer_start = (feature_buffer_start + output.size) % (FEATURE_BUFFER_SIZE);
      *slices_updated += 1;
    } else {
      // No feature slice was generated, but data is stored internally in the frontend for
      // next iteration.
    }
  }

  return SL_STATUS_OK;
}

