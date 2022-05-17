// Copyright (c) OpenMMLab. All rights reserved.

#include <chrono>
#include <fstream>

#include "archive/json_archive.h"
#include "core/logger.h"
#include "core/mat.h"
#include "json.hpp"
#include "preprocess/transform_module.h"
#include "opencv_utils.h"

using namespace mmdeploy;
using namespace std;
using nlohmann::json;

int main(int argc, char* argv[]) {
  if (argc < 5) {
    std::cerr << "usage: preprocess <cpu or cuda> <path/of/preprocess/config/json/file> "
                 "<path/of/an/image> <fuse>"
              << std::endl;
  }

  auto platform = argv[1];
  auto cfg_path = argv[2];
  auto img_path = argv[3];

  auto fuse = false;
  if (argc == 5 && atoi(argv[4]) != 0) {
    MMDEPLOY_WARN("using fuse-transform mode");
    fuse = true;
  } else {
    MMDEPLOY_WARN("using normal-transform mode");
  }

  // read preprocess config json file
  ifstream ifs(cfg_path);
  if (!ifs.is_open()) {
    std::cerr << "failed to open preprocess config file: " << cfg_path << std::endl;
    return -1;
  }
  std::string json_cfg((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());

  try {
    // Create a `TransformModule` instance using `cfg`
    std::cout << "create an instance of `TransformModule`" << std::endl;
    auto value_cfg = ::mmdeploy::from_json<Value>(json::parse(json_cfg));
    auto transform_cfg = value_cfg["pipeline"]["tasks"][0];

    const Device device{platform};
    Stream stream{device};
    transform_cfg["context"]["device"] = device;
    transform_cfg["context"]["stream"] = stream;
    if (fuse) {
      transform_cfg["fuse_transform"] = true;
    } else {
      transform_cfg["fuse_transform"] = false;
    }

    TransformModule transform_module(transform_cfg);

    // Prepare input data for `transform_module`
    std::cout << "read an image and convert it to `Value`" << std::endl;
    auto mat = cv::imread(img_path, cv::IMREAD_COLOR);
    auto mmdeploy_mat = cpu::CVMat2Mat(mat, PixelFormat::kBGR);
    Value input{{"ori_img", mmdeploy_mat}};

    // Do image preprocessing
    MMDEPLOY_INFO("start to do image processing on '{}'", platform);
    constexpr int test_count = 5000;
    auto t0 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < test_count; ++i) {
      auto output = transform_module(input);
      assert(!output.has_error());

      // Value out = output.value();
      // MMDEPLOY_INFO("output: {}", to_json(out).dump(2));
      // Mat src_mat = out["ori_img"].get<Mat>();
      // int height = src_mat.height();
      // int width = src_mat.width();
      // int channel = src_mat.channel();
      // PixelFormat pft = src_mat.pixel_format();
      // DataType dt = src_mat.type();
      // char* raw_data = src_mat.data<char>();

      // Value info_static = out["trans_info"]["static"];
      // Value info_runtime_args = out["trans_info"]["runtime_args"];
    }
    auto t1 = std::chrono::high_resolution_clock::now();
    MMDEPLOY_INFO("end to do image processing, cost: {}ms",
         std::chrono::duration<double, std::milli>(t1 - t0).count() / test_count);
  } catch (const std::exception& e) {
    std::cerr << "Exception happened: " << e.what() << std::endl;
    return -1;
  }

  return 0;
}
