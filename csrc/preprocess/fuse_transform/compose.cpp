// Copyright (c) OpenMMLab. All rights reserved.

#include "archive/json_archive.h"
#include "archive/value_archive.h"
#include "core/utils/formatter.h"
#include "fuse_transform.h"

namespace mmdeploy {

class MMDEPLOY_API Compose : public FuseTransform {
 public:
  explicit Compose(const Value& args, int version = 0) : FuseTransform(args) {
    assert(args.contains("context"));
    Value context;
    context = args["context"];
    context["stream"].get_to(stream_);
    for (auto cfg : args["transforms"]) {
      cfg["context"] = context;
      auto type = cfg.value("type", std::string{});
      MMDEPLOY_DEBUG("creating fuse_transform: {} with cfg: {}", type,
                     mmdeploy::to_json(cfg).dump(2));
      auto creator = Registry<FuseTransform>::Get().GetCreator(type, version);
      if (!creator) {
        MMDEPLOY_ERROR("unable to find creator: {}", type);
        throw std::invalid_argument("unable to find creator");
      }
      auto transform = creator->Create(cfg);
      if (!transform) {
        MMDEPLOY_ERROR("failed to create transform: {}", type);
        throw std::invalid_argument("failed to create transform");
      }
      transforms_.push_back(std::move(transform));
    }

    // fuse kernel
    Value cfg = {{"context", context}};
    auto creator = Registry<FuseTransformKernel>::Get().GetCreator(specified_platform_, version);
    if (nullptr == creator) {
      MMDEPLOY_ERROR("'FuseTransformKernel' is not supported on '{}' platform",
                     specified_platform_);
      throw std::domain_error("'FuseTransformKernel' is not supported on specified platform");
    }
    fuse_kernel_ = creator->Create(cfg);
  }
  ~Compose() override = default;

  Result<Value> Process(const Value& input) override {
    Value output = input;
    Value::Array intermediates;
    for (auto& transform : transforms_) {
      OUTCOME_TRY(auto t, transform->Process(output));
      output = std::move(t);
    }
    // one big kernel
    // shoud set correct input Tensor of output["img"]
    fuse_kernel_->Process(output);
    // end of one big kernel
    OUTCOME_TRY(stream_.Wait());
    return std::move(output);
  }

 private:
  std::vector<std::unique_ptr<FuseTransform>> transforms_;
  std::unique_ptr<FuseTransformKernel> fuse_kernel_;
  Stream stream_;
};

DECLARE_AND_REGISTER_MODULE(FuseTransform, Compose, 1);

}  // namespace mmdeploy