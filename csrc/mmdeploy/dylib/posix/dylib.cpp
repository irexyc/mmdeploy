// Copyright (c) OpenMMLab. All rights reserved.

#include "mmdeploy/dylib/dylib.h"

#include <dlfcn.h>

namespace mmdeploy::dylib {

namespace posix {

class LoaderImpl : public Loader {
 public:
  LoaderImpl() = default;

  static LoaderImpl& Get() {
    static LoaderImpl loader{};
    return loader;
  }

  bool Load(std::string_view name) {
    void* handle = dlopen(name.data(), RTLD_NOW | RTLD_GLOBAL);
    return (handle != nullptr);
  }
};

}  // namespace posix

using posix::LoaderImpl;
Loader& Loader::Get() { return LoaderImpl::Get(); }

}  // namespace mmdeploy::dylib
