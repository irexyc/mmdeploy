
#include "utils.h"

#include "archive/json_archive.h"
#include "archive/value_archive.h"

namespace mmdeploy {

void AddTransInfo(Value& trans_info, Value& output) {
  for (auto&& val : trans_info["static"]) {
    output["trans_info"]["static"].push_back(val);
  }
  for (auto&& val : trans_info["runtime_args"]) {
    output["trans_info"]["runtime_args"].push_back(val);
  }
}

bool CheckTraceInfoLengthEqual(Value& output) {
  return output["trans_info"]["static"].size() == output["trans_info"]["runtime_args"].size();
}

}  // namespace mmdeploy
