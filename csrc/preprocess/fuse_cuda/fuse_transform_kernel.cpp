#include "archive/json_archive.h"
#include "archive/value_archive.h"
#include "preprocess/fuse_transform/fuse_transform.h"

namespace mmdeploy {
namespace cuda {
class FuseTransformKernel : public ::mmdeploy::FuseTransformKernel {
 public:
  explicit FuseTransformKernel(const Value& args) : ::mmdeploy::FuseTransformKernel(args) {}
  Result<Value> Process(const Value& input) {
    MMDEPLOY_INFO("input, {}", to_json(input).dump(2));
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
