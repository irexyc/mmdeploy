// Copyright (c) OpenMMLab. All rights reserved.

/**
 * @file video_recognizer.h
 * @brief Interface to MMACTION video recognition task
 */

#ifndef MMDEPLOY_VIDEO_RECOGNIZER_H
#define MMDEPLOY_VIDEO_RECOGNIZER_H

#include "common.h"
#include "executor.h"
#include "model.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct mmdeploy_video_recognition_t {
  int label_id;
  float score;
} mmdeploy_video_recognition_t;

typedef struct mmdeploy_video_sample_info_t {
  int clip_len;
  int num_clips;
} mmdeploy_video_sample_info_t;

typedef struct mmdeploy_video_recognizer* mmdeploy_video_recognizer_t;

MMDEPLOY_API int mmdeploy_video_recognizer_create(mmdeploy_model_t model, const char* device_name,
                                                  int device_id,
                                                  mmdeploy_video_recognizer_t* recognizer);

MMDEPLOY_API int mmdeploy_video_recognizer_create_by_path(const char* model_path,
                                                          const char* device_name, int device_id,
                                                          mmdeploy_video_recognizer_t* recognizer);

MMDEPLOY_API int mmdeploy_video_recognizer_apply(mmdeploy_video_recognizer_t recognizer,
                                                 const mmdeploy_mat_t* images,
                                                 const mmdeploy_video_sample_info_t* video_info,
                                                 int video_count,
                                                 mmdeploy_video_recognition_t** results,
                                                 int** result_count);

MMDEPLOY_API void mmdeploy_video_recognizer_release_result(mmdeploy_video_recognition_t* results,
                                                           int* result_count, int video_count);

MMDEPLOY_API void mmdeploy_video_recognizer_destroy(mmdeploy_video_recognizer_t recognizer);

MMDEPLOY_API int mmdeploy_video_recognizer_create_v2(mmdeploy_model_t model,
                                                     mmdeploy_context_t context,
                                                     mmdeploy_video_recognizer_t* recognizer);

MMDEPLOY_API int mmdeploy_video_recognizer_create_input(
    const mmdeploy_mat_t* images, const mmdeploy_video_sample_info_t* video_info, int video_count,
    mmdeploy_value_t* value);

MMDEPLOY_API int mmdeploy_video_recognizer_apply_v2(mmdeploy_video_recognizer_t recognizer,
                                                    mmdeploy_value_t input,
                                                    mmdeploy_value_t* output);

MMDEPLOY_API int mmdeploy_video_recognizer_get_result(mmdeploy_value_t output,
                                                      mmdeploy_video_recognition_t** results,
                                                      int** result_count);

#ifdef __cplusplus
}
#endif

#endif  // MMDEPLOY_VIDEO_RECOGNIZER_H
