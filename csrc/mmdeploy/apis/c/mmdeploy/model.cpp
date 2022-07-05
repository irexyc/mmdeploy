// Copyright (c) OpenMMLab. All rights reserved.

// clang-format off
#include "model.h"

#include <memory>

#include "common_internal.h"
#include "mmdeploy/core/logger.h"
#include "mmdeploy/core/model.h"
// clang-format on

using namespace mmdeploy;

int mmdeploy_model_create_by_path(const char* path, mmdeploy_model_t* model) {
  try {
    auto ptr = std::make_unique<Model>(path);
    *model = reinterpret_cast<mmdeploy_model_t>(ptr.release());
    return MMDEPLOY_SUCCESS;
  } catch (const std::exception& e) {
    MMDEPLOY_ERROR("failed to create model: {}", e.what());
  } catch (...) {
    MMDEPLOY_ERROR("unknown exception caught");
  }
  return MMDEPLOY_E_FAIL;
}

int mmdeploy_model_create(const void* buffer, int size, mmdeploy_model_t* model) {
  try {
    auto ptr = std::make_unique<Model>(buffer, size);
    *model = reinterpret_cast<mmdeploy_model_t>(ptr.release());
    return MMDEPLOY_SUCCESS;
  } catch (const std::exception& e) {
    MMDEPLOY_ERROR("failed to create model: {}", e.what());
  } catch (...) {
    MMDEPLOY_ERROR("unknown exception caught");
  }
  return MMDEPLOY_E_FAIL;
}

void mmdeploy_model_destroy(mmdeploy_model_t model) { delete reinterpret_cast<Model*>(model); }

int mmdeploy_environment_add_model(mmdeploy_environment_t env, const char* name,
                                   mmdeploy_model_t model) {
  auto e = (mmdeploy::Environment*)env;
  if (std::count_if(e->models_.begin(), e->models_.end(),
                    [name](const auto& x) { return x.first == name; })) {
    MMDEPLOY_ERROR("{} already exists", name);
    return MMDEPLOY_E_INVALID_ARG;
  }
  e->models_.emplace_back(name, *Cast(model));
  return MMDEPLOY_SUCCESS;
}