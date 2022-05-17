// Copyright (c) OpenMMLab. All rights reserved.

#include "archive/json_archive.h"
#include "archive/value_archive.h"
#include "fuse_transform.h"
#include "utils.h"

namespace mmdeploy {

class MMDEPLOY_API Pad : public FuseTransform {
 public:
  explicit Pad(const Value& args, int version = 0) : FuseTransform(args) {
    arg_.size[0] = arg_.size[1] = 0;
    if (args.contains("size") && args["size"].is_number_integer()) {
      arg_.size[0] = arg_.size[1] = (args["size"].get<int>());
    }
    if (args.contains("size") && args["size"].is_array()) {
      if (args["size"].size() != 2) {
        throw std::invalid_argument("the length of size should be 2");
      }
      arg_.size[0] = args["size"][0].get<int>();
      arg_.size[1] = args["size"][1].get<int>();
    }

    arg_.size_divisor = args.value("size_divisor", 1);
    if (args.contains("pad_val")) {
      if (args["pad_val"].is_number()) {
        arg_.pad_val = args["pad_val"].get<float>();
      } else if (args["pad_val"].contains("img")) {
        arg_.pad_val = args["pad_val"]["img"][0].get<float>();
      } else {
        throw std::invalid_argument("args must be number or img dict");
      }
    } else {
      arg_.pad_val = 0.0f;
    }
    arg_.pad_to_square = args.value("pad_to_square", false);
    arg_.padding_mode = args.value("padding_mode", std::string("constant"));
  }
  ~Pad() override = default;

  Result<Value> Process(const Value& input) override {
    Value output = input;
    auto img_shape = output["img_shape"].array();
    bool img_shape_fixed = output.value("_img_shape_fixed", false);
    assert(img_shape.size() == 4);
    assert(img_shape[0].get<int>() == 1);
    assert(img_shape[3].get<int>() == 3 || img_shape[3].get<int>() == 1);

    int height = img_shape[1].get<int>();
    int width = img_shape[2].get<int>();
    int channel = img_shape[3].get<int>();

    // trace static info & runtime args
    Value trans_info;

    std::array<int, 4> padding{0, 0, 0, 0};
    std::vector<int> after_pad_shape;
    if (arg_.pad_to_square) {
      int max_size = std::max(height, width);
      padding = {0, 0, max_size - width, max_size - height};
      output["pad_fixed_size"].push_back(max_size);
      output["pad_fixed_size"].push_back(max_size);
      after_pad_shape = {1, max_size, max_size, channel};
    } else if (arg_.size[0] != 0 && arg_.size[1] != 0) {
      padding = {0, 0, arg_.size[1] - width, arg_.size[0] - height};
      output["pad_fixed_size"].push_back(arg_.size[0]);
      output["pad_fixed_size"].push_back(arg_.size[1]);
      after_pad_shape = {1, arg_.size[0], arg_.size[1], channel};
    } else if (arg_.size_divisor != 1) {
      auto pad_h = (height + arg_.size_divisor - 1) / arg_.size_divisor * arg_.size_divisor;
      auto pad_w = (width + arg_.size_divisor - 1) / arg_.size_divisor * arg_.size_divisor;
      padding = {0, 0, pad_w - width, pad_h - height};
      output["pad_size_divisor"] = arg_.size_divisor;
      output["pad_fixed_size"].push_back(pad_h);
      output["pad_fixed_size"].push_back(pad_w);
      after_pad_shape = {1, pad_h, pad_w, channel};
    } else {
      output["pad_fixed_size"].push_back(height);
      output["pad_fixed_size"].push_back(width);
      after_pad_shape = {1, height, width, channel};
    }
    output["img_shape"] = {after_pad_shape[0], after_pad_shape[1], after_pad_shape[2],
                           after_pad_shape[3]};

    // need pad
    if (std::count(begin(padding), end(padding), 0) != 4) {
      if (img_shape_fixed) {
        trans_info["static"].push_back({{"type", "Pad"},
                                        {"dynamic", false},
                                        {"pad_val", arg_.pad_val},
                                        {"tlbr", {padding[1], padding[0], padding[3], padding[2]}},
                                        {"size_hw", {after_pad_shape[1], after_pad_shape[2]}}});
        trans_info["runtime_args"].push_back({});
      } else {
        trans_info["static"].push_back(
            {{"type", "Pad"}, {"dynamic", true}, {"pad_val", arg_.pad_val}});
        trans_info["runtime_args"].push_back(
            {{"tlbr", {padding[1], padding[0], padding[3], padding[2]}},
             {"size_hw", {after_pad_shape[1], after_pad_shape[2]}}});
      }
    }

    AddTransInfo(trans_info, output);
    assert(CheckTraceInfoLengthEqual(output) == true);

    MMDEPLOY_DEBUG("output: {}", to_json(output).dump(2));

    return output;
  }

 protected:
  struct pad_arg_t {
    std::array<int, 2> size;
    int size_divisor;
    float pad_val;
    bool pad_to_square;
    std::string padding_mode;
  };
  using ArgType = struct pad_arg_t;
  ArgType arg_;
};

DECLARE_AND_REGISTER_MODULE(FuseTransform, Pad, 1);

}  // namespace mmdeploy
