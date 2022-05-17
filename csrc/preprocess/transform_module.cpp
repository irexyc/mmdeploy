// Copyright (c) OpenMMLab. All rights reserved.

#include "transform_module.h"

#include "archive/value_archive.h"
#include "core/module.h"
#include "core/utils/formatter.h"
#include "experimental/module_adapter.h"
#include "preprocess/fuse_transform/fuse_transform.h"
#include "preprocess/transform/transform.h"

namespace mmdeploy {

TransformModule::~TransformModule() = default;

template <typename T>
void InitTransormModule(const Value& args, std::unique_ptr<Module>& transform) {
  const auto type = "Compose";
  auto creator = Registry<T>::Get().GetCreator(type, 1);
  if (!creator) {
    MMDEPLOY_ERROR("unable to find creator: {}", type);
    throw_exception(eEntryNotFound);
  }
  auto cfg = args;
  if (cfg.contains("device")) {
    MMDEPLOY_WARN("force using device: {}", cfg["device"].get<const char*>());
    auto device = Device(cfg["device"].get<const char*>());
    cfg["context"]["device"] = device;
    cfg["context"]["stream"] = Stream::GetDefault(device);
  }

  transform = creator->Create(cfg);
}

TransformModule::TransformModule(const Value& args) {
  bool can_fuse = args.value("fuse_transform", false);
  if (can_fuse) {
    InitTransormModule<FuseTransform>(args, transform_);
  } else {
    InitTransormModule<Transform>(args, transform_);
  }
}

Result<Value> TransformModule::operator()(const Value& input) {
  auto output = transform_->Process(input);
  if (!output) {
    MMDEPLOY_ERROR("error: {}", output.error().message().c_str());
  }
  auto& ret = output.value();
  if (ret.is_object()) {
    // pass
  } else if (ret.is_array() && ret.size() == 1 && ret[0].is_object()) {
    ret = ret[0];
  } else {
    MMDEPLOY_ERROR("unsupported return value: {}", ret);
    return Status(eNotSupported);
  }
  return ret;
}

class MMDEPLOY_API TransformModuleCreator : public Creator<Module> {
 public:
  const char* GetName() const override { return "Transform"; }
  int GetVersion() const override { return 0; }
  std::unique_ptr<Module> Create(const Value& value) override {
    return CreateTask(TransformModule{value});
  }
};

REGISTER_MODULE(Module, TransformModuleCreator);

}  // namespace mmdeploy
