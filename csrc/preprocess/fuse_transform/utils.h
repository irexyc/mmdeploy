// Copyright (c) OpenMMLab. All rights reserved.

#ifndef MMDEPLOY_PREPROCESS_FUSE_TRANSFORM_UTILS_H
#define MMDEPLOY_PREPROCESS_FUSE_TRANSFORM_UTILS_H

#include "archive/json_archive.h"
#include "archive/value_archive.h"

namespace mmdeploy {

void AddTransInfo(Value& trans_info, Value& output);

bool CheckTraceInfoLengthEqual(Value& output);

}  // namespace mmdeploy

#endif  // MMDEPLOY_PREPROCESS_FUSE_TRANSFORM_UTILS_H
