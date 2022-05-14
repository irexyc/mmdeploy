#include "common.h"

#include "apis/c/common_internal.h"
#include "apis/c/handle.h"
#include "core/mat.h"

mmdeploy_value_t mmdeploy_value_copy(mmdeploy_value_t input) {
  return Guard([&] { return Take(Value(*Cast(input))); });
}

int mmdeploy_value_destroy(mmdeploy_value_t value) {
  delete Cast(value);
  return 0;
}

mmdeploy_value_t mmdeploy_common_create_input(const mm_mat_t* mats, int mat_count) {
  if (mat_count && mats == nullptr) {
    return nullptr;
  }
  try {
    auto input = std::make_unique<Value>(Value{Value::kArray});
    for (int i = 0; i < mat_count; ++i) {
      mmdeploy::Mat _mat{mats[i].height,         mats[i].width, PixelFormat(mats[i].format),
                         DataType(mats[i].type), mats[i].data,  Device{"cpu"}};
      input->front().push_back({{"ori_img", _mat}});
    }
    return reinterpret_cast<mmdeploy_value_t>(input.release());
  } catch (const std::exception& e) {
    MMDEPLOY_ERROR("unhandled exception: {}", e.what());
  } catch (...) {
    MMDEPLOY_ERROR("unknown exception caught");
  }
  return nullptr;
}