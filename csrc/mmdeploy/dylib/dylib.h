// Copyright (c) OpenMMLab. All rights reserved.

#ifndef DYLIB_DYLIB_H_
#define DYLIB_DYLIB_H_

#include <cstdio>
#include <string>

#ifdef _WIN32
#define LIBPREFIX
#define LIBSUFFIX ".dll"
#elif defined(__APPLE__)
#define LIBPREFIX "lib"
#define LIBSUFFIX ".dylib"
#else
#define LIBPREFIX "lib"
#define LIBSUFFIX ".so"
#endif

namespace mmdeploy::dylib {

class Loader {
 public:
  virtual ~Loader() = default;

  Loader(const Loader&) = delete;
  Loader& operator=(const Loader&) = delete;
  Loader(Loader&&) = delete;
  Loader& operator=(Loader&&) = delete;

  static Loader& Get();

  virtual bool Load(std::string_view name) = 0;

 protected:
  Loader() = default;
};

}  // namespace mmdeploy::dylib

#endif  // DYLIB_DYLIB_H_
