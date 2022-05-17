// Copyright (c) OpenMMLab. All rights reserved.

#include "archive/json_archive.h"
#include "archive/value_archive.h"
#include "fuse_transform.h"
#include "utils.h"

namespace mmdeploy {

class CenterCrop : public FuseTransform {
 public:
  explicit CenterCrop(const Value& args, int version = 0) : FuseTransform(args) {
    if (!args.contains(("crop_size"))) {
      throw std::invalid_argument("'crop_size' is expected");
    }
    if (args["crop_size"].is_number_integer()) {
      int crop_size = args["crop_size"].get<int>();
      arg_.crop_size[0] = arg_.crop_size[1] = crop_size;
    } else if (args["crop_size"].is_array() && args["crop_size"].size() == 2) {
      arg_.crop_size[0] = args["crop_size"][0].get<int>();
      arg_.crop_size[1] = args["crop_size"][1].get<int>();
    } else {
      throw std::invalid_argument("'crop_size' should be integer or an int array of size 2");
    }
  }
  ~CenterCrop() override = default;

  Result<Value> Process(const Value& input) override {
    Value output = input;

    auto img_shape = output["img_shape"].array();
    bool img_shape_fixed = output.value("_img_shape_fixed", false);
    int h = img_shape[1].get<int>();
    int w = img_shape[2].get<int>();
    int channel = img_shape[3].get<int>();
    int crop_height = arg_.crop_size[0];
    int crop_width = arg_.crop_size[1];

    int y1 = std::max(0, int(std::round((h - crop_height) / 2.0)));
    int x1 = std::max(0, int(std::round((w - crop_width) / 2.0)));
    int y2 = std::min(h, y1 + crop_height) - 1;
    int x2 = std::min(w, x1 + crop_width) - 1;

    std::vector<int> shape = {1, y2 - y1 + 1, x2 - x1 + 1, channel};
    output["img_shape"] = {shape[0], shape[1], shape[2], shape[3]};
    if (input.contains("scale_factor")) {
      // image has been processed by `Resize` transform before.
      // Compute cropped image's offset against the original image
      assert(input["scale_factor"].is_array() && input["scale_factor"].size() >= 2);
      float w_scale = input["scale_factor"][0].get<float>();
      float h_scale = input["scale_factor"][1].get<float>();
      output["offset"].push_back(x1 / w_scale);
      output["offset"].push_back(y1 / h_scale);
    } else {
      output["offset"].push_back(x1);
      output["offset"].push_back(y1);
    }

    // trace static info & runtime args
    output["_img_shape_fixed"] = true;
    Value trans_info;
    if (img_shape_fixed) {
      trans_info["static"].push_back({{"type", "CenterCrop"},
                                      {"tlbr", {y1, x1, h - shape[1] - y1, w - shape[2] - x1}},
                                      {"size_hw", {shape[1], shape[2]}},
                                      {"dynamic", false}});
      trans_info["runtime_args"].push_back({});
    } else {
      trans_info["static"].push_back(
          {{"type", "CenterCrop"}, {"size_hw", {shape[1], shape[2]}}, {"dynamic", true}});
      trans_info["runtime_args"].push_back(
          {{"tlbr", {y1, x1, h - shape[1] - y1, w - shape[2] - x1}}});
    }

    AddTransInfo(trans_info, output);
    assert(CheckTraceInfoLengthEqual(output) == true);

    MMDEPLOY_DEBUG("output: {}", to_json(output).dump(2));
    return output;
  }

 protected:
  struct center_crop_arg_t {
    std::array<int, 2> crop_size;
  };
  using ArgType = struct center_crop_arg_t;

  ArgType arg_;
};

DECLARE_AND_REGISTER_MODULE(FuseTransform, CenterCrop, 1);

}  // namespace mmdeploy
