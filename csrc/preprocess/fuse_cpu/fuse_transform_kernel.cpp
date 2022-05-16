#include "archive/json_archive.h"
#include "archive/value_archive.h"
#include "preprocess/fuse_transform/fuse_transform.h"

namespace mmdeploy {
namespace cpu {
class FuseTransformKernel : public ::mmdeploy::FuseTransformKernel {
 public:
  explicit FuseTransformKernel(const Value& args) : ::mmdeploy::FuseTransformKernel(args) {}
  Result<Value> Process(const Value& input) {
    MMDEPLOY_INFO("input, {}", to_json(input).dump(2));
    return input;
  }
};

class FuseTransformKernelCreator : public Creator<::mmdeploy::FuseTransformKernel> {
 public:
  const char* GetName() const override { return "cpu"; }
  int GetVersion() const override { return 1; }
  std::unique_ptr<::mmdeploy::FuseTransformKernel> Create(const Value& args) override {
    return std::make_unique<FuseTransformKernel>(args);
  }
};

}  // namespace cpu
}  // namespace mmdeploy

using mmdeploy::FuseTransformKernel;
using mmdeploy::cpu::FuseTransformKernelCreator;
REGISTER_MODULE(FuseTransformKernel, FuseTransformKernelCreator);