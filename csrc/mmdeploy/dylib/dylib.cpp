#include "mmdeploy/dylib/dylib.h"

#include <vector>

#include "mmdeploy/core/logger.h"
#include "mmdeploy/core/utils/formatter.h"
#include "mmdeploy/dylib/config.h"

namespace mmdeploy::dylib {

class LibManager {
 public:
  LibManager() {
    MMDEPLOY_INFO("Optional Modules: {}", modules);
    std::string success_msg = "[success] Loading {} ";
    std::string failure_msg = "[fail] Loading {} ";

    auto &loader = Loader::Get();
    for (const auto &lib : modules) {
      std::string name = LIBPREFIX + lib + LIBSUFFIX;
      if (loader.Load(name)) {
        MMDEPLOY_INFO(success_msg, name);
      } else {
        MMDEPLOY_WARN(failure_msg, name);
      }
    }
  }
};

static LibManager manager;

}  // namespace mmdeploy::dylib
