// Copyright (c) OpenMMLab. All rights reserved.

#include "archive/json_archive.h"
#include "archive/value_archive.h"
#include "core/tensor.h"
#include "fuse_transform.h"
#include "utils.h"

using namespace std;

namespace mmdeploy {

class MMDEPLOY_API Resize : public FuseTransform {
 public:
  explicit Resize(const Value& args, int version = 0) : FuseTransform(args) {
    arg_.keep_ratio = args.value<bool>("keep_ratio", false);
    if (args.contains("size")) {
      if (args["size"].is_number_integer()) {
        auto size = args["size"].get<int>();
        arg_.img_scale = {size, size};
      } else if (args["size"].is_array()) {
        if (args["size"].size() != 2) {
          MMDEPLOY_ERROR("'size' expects an array of size 2, but got {}", args["size"].size());
          throw std::length_error("'size' expects an array of size 2");
        }
        auto height = args["size"][0].get<int>();
        auto width = args["size"][1].get<int>();
        arg_.img_scale = {height, width};
      } else {
        MMDEPLOY_ERROR("'size' is expected to be an integer or and array of size 2");
        throw std::domain_error("'size' is expected to be an integer or and array of size 2");
      }
    }
    arg_.interpolation = args.value<string>("interpolation", "bilinear");

    vector<string> interpolations{"nearest", "bilinear", "bicubic", "area", "lanczos"};
    if (std::find(interpolations.begin(), interpolations.end(), arg_.interpolation) ==
        interpolations.end()) {
      MMDEPLOY_ERROR("'{}' interpolation is not supported", arg_.interpolation);
      throw std::invalid_argument("unexpected interpolation");
    }
  }
  ~Resize() override = default;

  Result<Value> Process(const Value& input) override {
    Value output = input;
    auto img_shape = output["img_shape"].array();
    assert(img_shape.size() == 4);

    int h = img_shape[1].get<int>();
    int w = img_shape[2].get<int>();
    int channel = img_shape[3].get<int>();
    int dst_h = 0;
    int dst_w = 0;
    float scale_factor = 0.f;
    bool fixed_scale = false;

    if (input.contains("scale")) {
      assert(input["scale"].is_array() && input["scale"].size() == 2);
      dst_h = input["scale"][0].get<int>();
      dst_w = input["scale"][1].get<int>();
      fixed_scale = true;
    } else if (input.contains("scale_factor")) {
      assert(input["scale_factor"].is_number());
      scale_factor = input["scale_factor"].get<float>();
      dst_h = int(h * scale_factor + 0.5);
      dst_w = int(w * scale_factor + 0.5);
    } else if (!arg_.img_scale.empty()) {
      MMDEPLOY_WARN(
          "neither 'scale' or 'scale_factor' is provided in input value. "
          "'img_scale' will be used");
      if (-1 == arg_.img_scale[1]) {
        // mmcls: adaptive_side, default is short
        if (w < h) {
          dst_w = arg_.img_scale[0];
          dst_h = dst_w * h / w;
        } else {
          dst_h = arg_.img_scale[0];
          dst_w = dst_h * w / h;
        }
      } else {
        dst_h = arg_.img_scale[0];
        dst_w = arg_.img_scale[1];
        fixed_scale = true;
      }
    } else {
      MMDEPLOY_ERROR("no resize related parameter is provided");
      return Status(eInvalidArgument);
    }

    int max_long_edge, max_short_edge;
    if (arg_.keep_ratio) {
      max_long_edge = dst_w;
      max_short_edge = dst_h;
      if (max_long_edge < max_short_edge) {
        std::swap(max_long_edge, max_short_edge);
      }
      scale_factor = std::min(max_long_edge * 1.0 / (1.0 * std::max(h, w)),
                              max_short_edge * 1.0 / (1.0 * std::min(h, w)));
      dst_w = int(w * scale_factor + 0.5);
      dst_h = int(h * scale_factor + 0.5);
    }

    auto w_scale = dst_w * 1.0 / w;
    auto h_scale = dst_h * 1.0 / h;
    output["scale_factor"] = {w_scale, h_scale, w_scale, h_scale};
    output["img_shape"] = {1, dst_h, dst_w, channel};
    output["keep_ratio"] = arg_.keep_ratio;
    output["_img_shape_fixed"] = arg_.keep_ratio == false && fixed_scale;

    // trace static info & runtime args
    // TODO:
    //   There are some cases not considered. Like adaptive side, scale_factor
    Value trans_info;
    if (arg_.keep_ratio) {
      trans_info["static"].push_back({{"type", "Resize"},
                                      {"keep_ratio", true},
                                      {"scale", {max_short_edge, max_long_edge}},
                                      {"interpolation", arg_.interpolation}});
    } else {
      trans_info["static"].push_back(
          {{"type", "Resize"},
           {"keep_ratio", false},
           {"scale", {dst_h, dst_w}},  // in some cases, it is not static.
           {"interpolation", arg_.interpolation}});
    }
    trans_info["runtime_args"].push_back({{"scale", {dst_h, dst_w}}});

    AddTransInfo(trans_info, output);
    assert(CheckTraceInfoLengthEqual(output) == true);

    MMDEPLOY_DEBUG("{}", to_json(output).dump(2));

    return output;
  }

 protected:
  struct resize_arg_t {
    std::array<int, 2> img_scale;
    std::string interpolation{"bilinear"};
    bool keep_ratio{true};
  };
  using ArgType = resize_arg_t;

 protected:
  ArgType arg_;
};

DECLARE_AND_REGISTER_MODULE(FuseTransform, Resize, 1);

}  // namespace mmdeploy
