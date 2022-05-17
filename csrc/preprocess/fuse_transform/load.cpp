// Copyright (c) OpenMMLab. All rights reserved.

#include "archive/json_archive.h"
#include "archive/value_archive.h"
#include "core/mat.h"
#include "core/types.h"
#include "fuse_transform.h"
#include "utils.h"

namespace mmdeploy {

std::string PixelFormatToString(PixelFormat fmt) {
  switch (fmt) {
    case PixelFormat::kBGR:
      return "BGR";
    case PixelFormat::kRGB:
      return "RGB";
    case PixelFormat::kGRAYSCALE:
      return "GRAYSCALE";
    case PixelFormat::kNV12:
      return "NV12";
    case PixelFormat::kNV21:
      return "NV21";
    case PixelFormat::kBGRA:
      return "BGRA";
  }
  throw_exception(eInvalidArgument);
}

std::string DataTypeToString(DataType dt) {
  switch (dt) {
    case DataType::kFLOAT:
      return "Float";
    case DataType::kHALF:
      return "Half";
    case DataType::kINT8:
      return "Int8";
    case DataType::kINT32:
      return "Int32";
    case DataType::kINT64:
      return "Int64";
  }
  throw_exception(eInvalidArgument);
}

class LoadImageFromFile : public FuseTransform {
 public:
  explicit LoadImageFromFile(const Value& args, int version = 0) : FuseTransform(args) {
    arg_.to_float32 = args.value("to_float32", false);
    arg_.color_type = args.value("color_type", std::string("color"));
  }

  ~LoadImageFromFile() override = default;

  Result<Value> Process(const Value& input) override {
    MMDEPLOY_DEBUG("input: {}", to_json(input).dump(2));
    assert(input.contains("ori_img"));

    // copy input data, and update its properties later
    Value output = input;

    Mat src_mat = input["ori_img"].get<Mat>();

    // trace static info & runtime args
    Value trans_info;
    if (arg_.color_type == "color" || arg_.color_type == "color_ignore_orientation") {
      trans_info["static"].push_back({{"type", "cvtColorBGR"}});
      output["img_pixel_format"] = "BGR";
    } else {
      trans_info["static"].push_back({{"type", "cvtColorGray"}});
      output["img_pixel_format"] = "Gray";
    }
    trans_info["runtime_args"].push_back(
        {{"src_pixel_format", PixelFormatToString(src_mat.pixel_format())}});

    if (arg_.to_float32) {
      trans_info["static"].push_back({{"type", "CastFloat"}});
      trans_info["runtime_args"].push_back({{"src_data_type", DataTypeToString(src_mat.type())}});
    }

    AddTransInfo(trans_info, output);
    assert(CheckTraceInfoLengthEqual(output) == true);

    output["img_shape"] = {1, src_mat.height(), src_mat.width(), src_mat.channel()};
    output["ori_shape"] = {1, src_mat.height(), src_mat.width(), src_mat.channel()};
    output["img_fields"].push_back("img");
    output["img"] = {};  // placeholder
    output["_img_data_type"] =
        arg_.to_float32 ? DataTypeToString(DataType::kFLOAT) : DataTypeToString(src_mat.type());

    MMDEPLOY_DEBUG("output: {}", to_json(output).dump(2));

    return output;
  }

 protected:
  struct prepare_image_arg_t {
    bool to_float32{false};
    std::string color_type{"color"};
  };
  using ArgType = struct prepare_image_arg_t;

  ArgType arg_;
};

DECLARE_AND_REGISTER_MODULE(FuseTransform, LoadImageFromFile, 1);

}  // namespace mmdeploy
