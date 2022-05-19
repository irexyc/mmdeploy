#include <cuda_runtime.h>

#include "archive/json_archive.h"
#include "archive/value_archive.h"
#include "core/mat.h"
#include "preprocess/fuse_transform/fuse_transform.h"

namespace mmdeploy {
namespace cuda {
class FuseTransformKernel : public ::mmdeploy::FuseTransformKernel {
 public:
  explicit FuseTransformKernel(const Value& args) : ::mmdeploy::FuseTransformKernel(args) {}
  Result<Value> Process(const Value& input) {
    MMDEPLOY_DEBUG("input, {}", to_json(input).dump(2));

    Value output = input;
    Mat src_mat = output["ori_img"].get<Mat>();
    int src_height = src_mat.height();
    int src_width = src_mat.width();
    int src_channel = src_mat.channel();
    // DataType src_dt = src_mat.type()
    PixelFormat src_pft = src_mat.pixel_format();
    uint8_t* src_raw_data = src_mat.data<uint8_t>();  // may change depend on src_dt

    auto dst_shape = (output.contains("_img_shape")) ? output["_img_shape"].array()
                                                     : output["meta_keys"]["img_shape"].array();
    int dst_h = dst_shape[1].get<int>();
    int dst_w = dst_shape[2].get<int>();
    int dst_c = dst_shape[3].get<int>();
    // output tensor
    TensorDesc desc = {Device{"cuda"}, DataType::kFLOAT, {1, dst_c, dst_h, dst_w}};
    Tensor img(desc);
    float* dst_raw_data = img.data<float>();
    auto stream = ::mmdeploy::GetNative<cudaStream_t>(stream_);

    // call kernel

    // set result
    output["img"] = img;
    /*
    {
        "img":null,
        "img_metas":Object{...},
        "ori_img":"<any>",
        "trans_info":{
            "runtime_args":Array[7],
            "static":Array[7]
        }
    }
    */
    return input;
  }
};

class FuseTransformKernelCreator : public Creator<::mmdeploy::FuseTransformKernel> {
 public:
  const char* GetName() const override { return "cuda"; }
  int GetVersion() const override { return 1; }
  std::unique_ptr<::mmdeploy::FuseTransformKernel> Create(const Value& args) override {
    return std::make_unique<FuseTransformKernel>(args);
  }
};

}  // namespace cuda
}  // namespace mmdeploy

using mmdeploy::FuseTransformKernel;
using mmdeploy::cuda::FuseTransformKernelCreator;
REGISTER_MODULE(FuseTransformKernel, FuseTransformKernelCreator);
