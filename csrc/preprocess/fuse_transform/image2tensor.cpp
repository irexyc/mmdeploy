// Copyright (c) OpenMMLab. All rights reserved.

#include "archive/json_archive.h"
#include "archive/value_archive.h"
#include "fuse_transform.h"
#include "utils.h"

namespace mmdeploy {

class ImageToTensor : public FuseTransform {
 public:
  explicit ImageToTensor(const Value& args, int version = 0) : FuseTransform(args) {
    for (auto& key : args["keys"]) {
      arg_.keys.push_back(key.get<std::string>());
    }
  }
  ~ImageToTensor() override = default;

  Result<Value> Process(const Value& input) override {
    Value output = input;
    auto img_shape = output["img_shape"].array();
    assert(img_shape.size() == 4);
    assert(img_shape[3].get<int>() == 1 || img_shape[3].get<int>() == 3);

    // trace static info & runtime args
    Value trans_info;
    trans_info["static"].push_back({{"type", "HWC2CHW"}});
    trans_info["runtime_args"].push_back({});
    AddTransInfo(trans_info, output);
    assert(CheckTraceInfoLengthEqual(output) == true);

    MMDEPLOY_DEBUG("output: {}", to_json(output).dump(2));

    return output;
  }

 protected:
  struct to_img_tensor_arg_t {
    std::vector<std::string> keys;
  };
  using ArgType = struct to_img_tensor_arg_t;

  ArgType arg_;
};

DECLARE_AND_REGISTER_MODULE(FuseTransform, ImageToTensor, 1);

}  // namespace mmdeploy
