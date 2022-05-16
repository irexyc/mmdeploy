// Copyright (c) OpenMMLab. All rights reserved.

#ifndef MMDEPLOY_PREPROCESS_FUSE_TRANSFORM_FUSE_TRANSFORM_H
#define MMDEPLOY_PREPROCESS_FUSE_TRANSFORM_FUSE_TRANSFORM_H

#include <array>

#include "core/device.h"
#include "core/module.h"
#include "core/registry.h"
#include "core/tensor.h"

namespace mmdeploy {

class MMDEPLOY_API FuseTransformKernel : public Module {
 public:
  FuseTransformKernel() = default;
  explicit FuseTransformKernel(const Value& args);
  ~FuseTransformKernel() override = default;
  Result<Value> Process(const Value& input);

 protected:
  Device device_;
  Stream stream_;
};

MMDEPLOY_DECLARE_REGISTRY(FuseTransformKernel);

class MMDEPLOY_API FuseTransform : public Module {
 public:
  ~FuseTransform() override = default;

  FuseTransform() = default;
  explicit FuseTransform(const Value& args);
  FuseTransform(const FuseTransform&) = delete;
  FuseTransform& operator=(const FuseTransform&) = delete;

 protected:
  std::vector<std::string> GetImageFields(const Value& input);
  std::string specified_platform_;
  std::vector<std::string> candidate_platforms_;
};

MMDEPLOY_DECLARE_REGISTRY(FuseTransform);

}  // namespace mmdeploy

#endif  // MMDEPLOY_PREPROCESS_FUSE_TRANSFORM_FUSE_TRANSFORM_H