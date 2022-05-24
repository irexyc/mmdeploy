
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

void extract_runtime_args(Value& output, std::vector<int>& resize_hw, float& pad_val,
                          std::vector<int>& padding_tlbr, std::vector<int>& padding_size_hw,
                          std::vector<int>& crop_tlbr, std::vector<int>& crop_size_hw) {
  auto static_info = output["trans_info"]["static"];
  auto runtime_args = output["trans_info"]["runtime_args"];

  for (int i = 0; i < static_info.size(); i++) {
    auto trans = static_info[i];
    auto args = runtime_args[i];
    std::string type = trans["type"].get<std::string>();
    if (type == "Resize") {
      Value size_hw;
      if (trans["dynamic"].get<bool>()) {
        size_hw = args["size_hw"];
      } else {
        size_hw = trans["size_hw"];
      }
      resize_hw[0] = size_hw[0].get<int>();
      resize_hw[1] = size_hw[1].get<int>();
    } else if (type == "Pad") {
      pad_val = trans["pad_val"].get<float>();
      Value size_hw, tlbr;
      if (trans["dynamic"].get<bool>()) {
        size_hw = args["size_hw"];
        tlbr = args["tlbr"];
      } else {
        size_hw = trans["size_hw"];
        tlbr = trans["tlbr"];
      }
      padding_size_hw[0] = size_hw[0].get<int>();
      padding_size_hw[1] = size_hw[1].get<int>();
      for (int i = 0; i < 4; i++) {
        padding_tlbr[i] = tlbr[i].get<int>();
      }
    } else if (type == "CenterCrop") {
      Value size_hw = trans["size_hw"];
      Value tlbr;
      if (trans["dynamic"].get<bool>()) {
        tlbr = args["tlbr"];
      } else {
        tlbr = trans["tlbr"];
      }
      crop_size_hw[0] = size_hw[0].get<int>();
      crop_size_hw[1] = size_hw[1].get<int>();
      for (int i = 0; i < 4; i++) {
        crop_tlbr[i] = tlbr[i].get<int>();
      }
    }
  }
}

}  // namespace mmdeploy
