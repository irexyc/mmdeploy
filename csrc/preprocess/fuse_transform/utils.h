// Copyright (c) OpenMMLab. All rights reserved.

#ifndef MMDEPLOY_PREPROCESS_FUSE_TRANSFORM_UTILS_H
#define MMDEPLOY_PREPROCESS_FUSE_TRANSFORM_UTILS_H

#include "archive/json_archive.h"
#include "archive/value_archive.h"

namespace mmdeploy {

void AddTransInfo(Value &trans_info, Value &output);

bool CheckTraceInfoLengthEqual(Value &output);

void MMDEPLOY_API extract_runtime_args(Value &output, std::vector<int> &resize_hw, float &pad_val,
                                       std::vector<int> &padding_tlbr,
                                       std::vector<int> &padding_size_hw,
                                       std::vector<int> &crop_tlbr, std::vector<int> &crop_size_hw);

}  // namespace mmdeploy

#endif  // MMDEPLOY_PREPROCESS_FUSE_TRANSFORM_UTILS_H
