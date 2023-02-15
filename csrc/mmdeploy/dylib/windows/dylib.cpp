// Copyright (c) OpenMMLab. All rights reserved.

#include "mmdeploy/dylib/dylib.h"

#include <Windows.h>

namespace mmdeploy::dylib {
namespace windows {
class LoaderImpl : public Loader {
 public:
  LoaderImpl() = default;

  static LoaderImpl& Get() {
    static LoaderImpl loader{};
    return loader;
  }

  bool Load(std::string_view name) {
    HMODULE handle = LoadLibraryA(name.data());
    return (handle != nullptr);
  }
};

}  // namespace windows

using windows::LoaderImpl;
Loader& Loader::Get() { return LoaderImpl::Get(); }

}  // namespace mmdeploy::dylib
