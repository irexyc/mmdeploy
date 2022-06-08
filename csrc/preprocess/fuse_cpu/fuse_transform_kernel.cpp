#include <sstream>
#include <fstream>
#include <iostream>  
#include <string>
#include <dlfcn.h> // dlopen
#include <sys/stat.h> // mkdir
#include <unistd.h>   // rmdir
#include <cstdio>  // remove
#include <cstdlib> // mkdtemp, system

#include "archive/json_archive.h"
#include "archive/value_archive.h"
#include "core/mat.h"
#include "preprocess/fuse_transform/fuse_transform.h"
#include "preprocess/fuse_transform/utils.h"

#include "fuse_impl/common_kernel.h"

namespace mmdeploy {
namespace cpu {
class FuseTransformKernel : public ::mmdeploy::FuseTransformKernel {
 public:
  explicit FuseTransformKernel(const Value& args) : ::mmdeploy::FuseTransformKernel(args) {}
  Result<Value> Process(const Value& input) {

    // MMDEPLOY_INFO("input, {}", to_json(input).dump(2));
    Value output = input;
    Mat src_mat = output["ori_img"].get<Mat>();
    int src_h = src_mat.height();
    int src_w = src_mat.width();
    int src_c = src_mat.channel();
    // DataType src_dt = src_mat.type()
    PixelFormat src_pft = src_mat.pixel_format();
    uint8_t* src_raw_data = src_mat.data<uint8_t>();  // may change depend on src_dt


    auto dst_shape = (output.contains("_img_shape")) ? output["_img_shape"].array()
                                                     : output["meta_keys"]["img_shape"].array();
    int dst_h = dst_shape[1].get<int>();
    int dst_w = dst_shape[2].get<int>();
    int dst_c = dst_shape[3].get<int>();

    // output tensor
    TensorDesc desc = {Device{"cpu"}, DataType::kFLOAT, {1, dst_c, dst_h, dst_w}};
    Tensor img(desc);
    float* dst_raw_data = img.data<float>();

    std::vector<int> resize_hw(2, 0);
    float pad_val = 0;
    std::vector<int> padding_tlbr(4, 0);
    std::vector<int> padding_size_hw(2, 0);
    std::vector<int> crop_tlbr(4, 0);
    std::vector<int> crop_size_hw(2, 0);
    extract_runtime_args(output, resize_hw, pad_val, padding_tlbr, padding_size_hw, crop_tlbr,
                         crop_size_hw);

    // auto print_vec = [](std::string name, std::vector<int>& vec) {
    //   std::stringstream ss;
    //   for (int i = 0; i < vec.size(); i++) {
    //     ss << vec[i] << " ";
    //   }
    //   MMDEPLOY_INFO("{} - {}", name, ss.str());
    // };
    // print_vec("padding_tlbr", padding_tlbr);
    // print_vec("crop_tlbr", crop_tlbr);
    // print_vec("resize_hw", resize_hw);
    // print_vec("padding_size_hw", padding_size_hw);
    // print_vec("crop_size_hw", crop_size_hw);


    /* -------------------------------------------*/
    mkdir("/tmp/mmdeploy", 0755);
    char path[] = "/tmp/mmdeploy/XXXXXX";
    mkdtemp(path);

    std::string json_path = (std::string)path + "/static.json"; // name maybe change

    //TODO
    // 1. write the static.json into json_path
    // 2. generate tag and judge whether it's exist

    std::string elena_path = "./bin/trace_test"; // need put elena exec it in build/bin/.

    FILE *json_file = fopen(json_path.c_str(), "r");
    FILE *elena_file = fopen(elena_path.c_str(), "r");

    if ( !json_file || !elena_file ) {
      std::cout << "Error opening json or elena binary file\n";
      exit(1);
    }
    auto codegen_cmd = (std::string) elena_path + " " + json_path;
    system(codegen_cmd.c_str());

    auto cc = (std::string)path + "./source.c";
    // auto cc = (std::string) "./bin/source.c";
    auto so = (std::string)path + "./libsource.so";
    // auto so = (std::string) "./bin/libsource.so";
    auto shared_cmd = (std::string) "g++ -shared -fPIC -O3 -o " + so + " " + cc;
    system(shared_cmd.c_str());

    auto dlHandle_ = dlopen(so.c_str(), RTLD_NOW); // 立即决定 返回前解除所有未决定的符号
    if (!dlHandle_) {
        std::cout << "Unable to load target code";
        exit(1);
    }


    // if(src_format == "BGR") {
      auto func_ = (void (*)(uint64_t, uint64_t, int32_t, int32_t, uint64_t, uint64_t, int32_t, int32_t, int32_t, int32_t, int16_t*, int16_t*, int32_t*, int32_t*, uint8_t*, float*, uint64_t, uint64_t))dlsym(dlHandle_, "BGR_Kernel");
    //}
    
    if (!func_) {
        std::cout << "Target function not found";
        exit(1);
    }

    int resize_h = resize_hw[0];
    int resize_w = resize_hw[1];

    int crop_top = crop_tlbr[0];
    int crop_left = crop_tlbr[1];

    int pad_h = padding_size_hw[0];
    int pad_w = padding_size_hw[1];


    // call kernel
    short* cubfh; 
    short* cubfw;
    int* inth;
    int* intw;

    auto t0 = std::chrono::high_resolution_clock::now();
    if(resize_h && resize_w ) { // add bilinear judegement
      cubfh = new short[resize_h*2];
      cubfw = new short[resize_w*2];
      inth = new int[resize_h*2];
      intw = new int[resize_w*2];

      resize_preprocess(src_h, src_w, resize_h, resize_w, cubfh, cubfw, inth, intw);
    }

    func_(resize_h, resize_w, crop_top, crop_left, pad_h, pad_w, padding_tlbr[0], padding_tlbr[1], padding_tlbr[2], padding_tlbr[3], cubfh, cubfw, inth, intw, src_raw_data, dst_raw_data, src_h, src_w); //dynamic

    delete[] cubfh;
    delete[] cubfw;
    delete[] inth;
    delete[] intw;

    auto t1 = std::chrono::high_resolution_clock::now();
    MMDEPLOY_INFO("elena fused time, cost: {}ms",
         std::chrono::duration<double, std::milli>(t1 - t0).count());

    // float sum = 0.0;

    // for(int i = 0; i < dst_h * dst_w * dst_c; i++){
    //   sum += dst_raw_data[i];
    // }

    // std::cout << "final sum = " << sum << std::endl;

    // set result
    output["img"] = img;
    /*
    {
        "img":null,
        "img_metas":Object{...},
        "ori_img":"<any>",
        "trans_info":{
            "runtime_args":Array[7],
            "static":Array[7]
        }
    }
    */
    return output;
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
