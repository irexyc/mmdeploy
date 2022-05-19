// Copyright (c) OpenMMLab. All rights reserved.

#include "archive/json_archive.h"
#include "archive/value_archive.h"
#include "core/utils/formatter.h"
#include "fuse_transform.h"

namespace mmdeploy {

class Compose : public FuseTransform {
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
      transform_types_.push_back(type);
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
  ~Compose() override {
    for (int i = 0; i < transforms_.size(); ++i) {
      MMDEPLOY_INFO("transform avg-cost: {}ms, name: trace-{}", d[i + 1] / run_times,
                    transform_types_[i]);
    }
    MMDEPLOY_INFO("transform avg-cost: {}ms, name: trace-fuse",
                  d[transforms_.size() + 1] / run_times);
  }

  Result<Value> Process(const Value& input) override {
    Value output = input;
    Value::Array intermediates;
    for (int i = 0; i < transforms_.size(); ++i) {
      auto t0 = std::chrono::high_resolution_clock::now();
      auto& transform = transforms_[i];
      auto& name = transform_types_[i];

      OUTCOME_TRY(auto t, transform->Process(output));
      output = std::move(t);

      OUTCOME_TRY(stream_.Wait());

      auto t1 = std::chrono::high_resolution_clock::now();
      d[i + 1] += std::chrono::duration<double, std::milli>(t1 - t0).count();
      // MMDEPLOY_INFO("transform cost: {}ms, name: trace-{}",
      //             std::chrono::duration<double, std::milli>(t1 - t0).count(), name);
    }
    // one big kernel
    // should set correct input Tensor of output["img"]
    auto t0 = std::chrono::high_resolution_clock::now();

    fuse_kernel_->Process(output);

    auto t1 = std::chrono::high_resolution_clock::now();
    d[transforms_.size() + 1] += std::chrono::duration<double, std::milli>(t1 - t0).count();
    // end of one big kernel
    run_times++;
    OUTCOME_TRY(stream_.Wait());
    return std::move(output);
  }

 private:
  double d[20]{};
  int run_times{};
  std::vector<std::unique_ptr<FuseTransform>> transforms_;
  std::vector<std::string> transform_types_;
  std::unique_ptr<FuseTransformKernel> fuse_kernel_;
  Stream stream_;
};

DECLARE_AND_REGISTER_MODULE(FuseTransform, Compose, 1);

}  // namespace mmdeploy
