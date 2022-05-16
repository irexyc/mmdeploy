// Copyright (c) OpenMMLab. All rights reserved.

#include "archive/json_archive.h"
#include "archive/value_archive.h"
#include "fuse_transform.h"
#include "utils.h"

namespace mmdeploy {

class MMDEPLOY_API Normalize : public FuseTransform {
 public:
  explicit Normalize(const Value& args, int version = 0) : FuseTransform(args) {
    if (!args.contains("mean") || !args.contains("std")) {
      MMDEPLOY_ERROR("no 'mean' or 'std' is configured");
      throw std::invalid_argument("no 'mean' or 'std' is configured");
    }
    for (auto& v : args["mean"]) {
      arg_.mean.push_back(v.get<float>());
    }
    for (auto& v : args["std"]) {
      arg_.std.push_back(v.get<float>());
    }
    arg_.to_rgb = args.value("to_rgb", true);
  }
  ~Normalize() override = default;

  Result<Value> Process(const Value& input) override {
    Value output = input;
    auto img_shape = output["img_shape"].array();
    std::string img_data_type = output["_img_data_type"].get<std::string>();
    std::string img_pixel_format = output["img_pixel_format"].get<std::string>();
    assert(img_shape.size() == 4);
    assert(img_shape[3].get<int>() == arg_.mean.size());

    for (auto& v : arg_.mean) {
      output["img_norm_cfg"]["mean"].push_back(v);
    }
    for (auto v : arg_.std) {
      output["img_norm_cfg"]["std"].push_back(v);
    }
    output["img_norm_cfg"]["to_rgb"] = arg_.to_rgb;
    output["_img_data_type"] = "Float";

    // trace static info & runtime args
    // 1. cast
    Value trans_info;
    trans_info["static"].push_back({{"type", "CastFloat"}});
    trans_info["runtime_args"].push_back({{"src_date_type", img_data_type}});
    // 2. to_rgb
    if (img_pixel_format == "BGR" && arg_.to_rgb) {
      trans_info["static"].push_back({{"type", "cvtColorRGB"}});
      trans_info["runtime_args"].push_back({{"src_pixel_format", img_pixel_format}});
    }
    // 3.normalize
    trans_info["static"].push_back({{"type", "Normalize"},
                                    {"mean", output["img_norm_cfg"]["mean"]},
                                    {"std", output["img_norm_cfg"]["std"]}});
    trans_info["runtime_args"].push_back({});

    AddTransInfo(trans_info, output);
    assert(CheckTraceInfoLengthEqual(output) == true);

    MMDEPLOY_DEBUG("output: {}", to_json(output).dump(2));

    return output;
  }

 protected:
  struct normalize_arg_t {
    std::vector<float> mean;
    std::vector<float> std;
    bool to_rgb;
  };
  using ArgType = struct normalize_arg_t;
  ArgType arg_;
};

DECLARE_AND_REGISTER_MODULE(FuseTransform, Normalize, 1);

}  // namespace mmdeploy
