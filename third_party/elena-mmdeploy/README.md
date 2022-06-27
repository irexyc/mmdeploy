# Elena

[![pipeline status](http://gitlab.bj.sensetime.com/platform/dlcompiler/elena/badges/master/pipeline.svg)](http://gitlab.bj.sensetime.com/platform/dlcompiler/elena/commits/master)

[![coverage report](http://gitlab.bj.sensetime.com/platform/dlcompiler/elena/badges/master/coverage.svg)](http://platform.pages.gitlab.bj.sensetime.com/dlcompiler/elena/coverage/)

Elena is a deep learning compiler to bridge the gap between high-level tensor algorithm with varieties of devices, such as GPUs and emerging ASIPs.
Elena is famous for her versatile in languages, who earns the title of Oraculum Septilingue ("Seven-language Oracle").


# Installation
```shell
git clone --recursive git@gitlab.bj.sensetime.com:platform/dlcompiler/elena.git
mkdir build && cd build
cmake ..
make -j8
```
## Requirements
+ A recent c++ compiler supporting C++ 14 (g++-5 or higher)
+ CMake 3.8 or higher
+ CUDA 8.0 or higher

# Usage

## For MMDeploy
### Auto code-gen the image preprocessing operators written in C++
```shell
cd build/examples/MMDeploy
./OpFuse <path/of/OpList/json/file> <cpu or cuda> <path/of/generate/code>
```
### Fuse function interface
```
void FuseKernel(uint64_t resize_h, uint64_t resize_w, uint64_t crop_size, int32_t crop_top, int32_t crop_left, float norm_mean_0, float norm_mean_1, float norm_mean_2, float norm_std_0, float norm_std_1, float norm_std_2, uint64_t pad_h, uint64_t pad_w, int32_t pad_top, int32_t pad_left, int32_t pad_bottom, int32_t pad_right, float pad_value, uint8_t* __restrict__ src_raw_data, float* __restrict__ dst_raw_data, uint64_t src_h, uint64_t src_w, const char *format, const char *interpolation = "nearest");  // if no ResizeOp, use default "nearest" to replace 
```