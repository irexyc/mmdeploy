// Copyright (c) OpenMMLab. All rights reserved.

#ifndef MMDEPLOY_SRC_MODULE_TRANSFORM_MODULE_H_
#define MMDEPLOY_SRC_MODULE_TRANSFORM_MODULE_H_

#include "core/value.h"
#include "core/module.h"

namespace mmdeploy {

class Transform;
class FuseTransform;

class MMDEPLOY_API TransformModule {
 public:
  ~TransformModule();
  explicit TransformModule(const Value& args);
  TransformModule(TransformModule&&) = default;
  Result<Value> operator()(const Value& input);

 private:
  std::unique_ptr<Module> transform_;
};

}  // namespace mmdeploy

#endif  // MMDEPLOY_SRC_MODULE_TRANSFORM_MODULE_H_
