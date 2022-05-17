
#include "utils.h"

#include "archive/json_archive.h"
#include "archive/value_archive.h"

namespace mmdeploy {

void AddTransInfo(Value& trans_info, Value& output) {
  if (!trans_info.contains("static") || !trans_info.contains("runtime_args")) {
    return;
  }
  for (auto&& val : trans_info["static"]) {
    output["trans_info"]["static"].push_back(val);
  }
  for (auto&& val : trans_info["runtime_args"]) {
    output["trans_info"]["runtime_args"].push_back(val);
  }
}

bool CheckTraceInfoLengthEqual(Value& output) {
  if (output.contains("trans_info")) {
    auto& trans_info = output["trans_info"];
    if (trans_info.contains("static") && trans_info.contains("runtime_args")) {
      return trans_info["static"].size() == trans_info["runtime_args"].size();
    }
  }
}

}  // namespace mmdeploy
