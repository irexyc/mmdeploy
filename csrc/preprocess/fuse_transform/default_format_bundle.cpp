// Copyright (c) OpenMMLab. All rights reserved.

#include "archive/json_archive.h"
#include "archive/value_archive.h"
#include "fuse_transform.h"
#include "utils.h"

namespace mmdeploy {
class MMDEPLOY_API DefaultFormatBundle : public FuseTransform {
 public:
  explicit DefaultFormatBundle(const Value& args, int version = 0) : FuseTransform(args) {
    if (args.contains("img_to_float") && args["img_to_float"].is_boolean()) {
      arg_.img_to_float = args["img_to_float"].get<bool>();
    }
  }
  ~DefaultFormatBundle() override = default;

  Result<Value> Process(const Value& input) override {
    Value output = input;
    auto img_shape = output["img_shape"].array();
    std::string img_data_type = output["_img_data_type"].get<std::string>();
    output["_img_data_type"] = "Float";

    // trace static info & runtime args
    Value trans_info;
    if (img_data_type == "Int8" && arg_.img_to_float) {
      trans_info["static"].push_back({{"type", "CastFloat"}});
      trans_info["runtime_args"].push_back({{"src_date_type", img_data_type}});
    }

    // set default meta keys
    if (!output.contains("pad_shape")) {
      for (auto v : img_shape) {
        output["pad_shape"].push_back(v.get<int>());
      }
    }
    if (!output.contains("scale_factor")) {
      output["scale_factor"].push_back(1.0);
    }
    if (!output.contains("img_norm_cfg")) {
      int channel = img_shape[3].get<int>();
      for (int i = 0; i < channel; i++) {
        output["img_norm_cfg"]["mean"].push_back(0.0);
        output["img_norm_cfg"]["std"].push_back(1.0);
      }
      output["img_norm_cfg"]["to_rgb"] = false;
    }

    // transpose
    trans_info["static"].push_back({{"type", "HWC2CHW"}});
    trans_info["runtime_args"].push_back({});
    AddTransInfo(trans_info, output);
    assert(CheckTraceInfoLengthEqual(output) == true);

    MMDEPLOY_DEBUG("output: {}", to_json(output).dump(2));
    return output;
  }

 protected:
  struct default_format_bundle_arg_t {
    bool img_to_float = true;
  };
  using ArgType = struct default_format_bundle_arg_t;

  ArgType arg_;
};

DECLARE_AND_REGISTER_MODULE(FuseTransform, DefaultFormatBundle, 1);

}  // namespace mmdeploy