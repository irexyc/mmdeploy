## Build preprocess example
```bash
mkdir -p build && cd build
cmake .. -DCMAKE_CXX_COMPILER=g++-7 \
-DMMDEPLOY_BUILD_SDK=ON \
-DMMDEPLOY_TARGET_DEVICES="cuda;cpu" \
-Dpplcv_DIR=${PATH_OF_PPLCV}/cuda-build/install/lib/cmake/ppl
make -j10
```

## Run preprocess example
```bash
./bin/preprocess <cpu or cuda> <path/of/preprocess/config/json/file> <path/of/an/image> <fuse>

fuse 为0即为之前的pipeline，fuse 为1即为trace的pipeline
```

## Trace information


### cvtColorBGR

```
// static
{
    "type", "cvtColorBGR"
}

// runtime_args
{
    "src_pixel_format": "BGR"
}
```

###  CastFloat
```
// static
{
    "type", "CastFloat"
}
// runtime_args
{
    "src_data_type": "Int8"
}
```

### Resize
```
// case 1. 之前的size可固定
//static
{
    "type": "Resize",
    "interpolation": "bilinear",
    "dynamic", false,
    "size_hw": [0, 0]
}
// runtime_args
null

// case 2. 之前的size不确定
//static
{
    "type": "Resize",
    "interpolation": "bilinear",
    "dynamic", true,
}

// runtime_args
```
{
    "size_hw": [0, 0]
}

### Normalize
```
// static
{
    "type"" "Normalize",
    "mean"" [0, 0, 0], // may be length 1 or 3
    "std": [0, 0, 0],
}
// runtime_args
null
```

### Pad
```
// case 1. 之前的size可固定
// static
{
    "type": "Pad",
    "dynamic": false,
    "pad_val": 0,
    "tlbr": [0, 0, 0, 0],
    "size_hw": [0, 0],
};
// runtime_args
null

// case 2. 之前的size不确定
// static
{
    "type": "Pad",
    "dynamic": true,
    "pad_val": 0
}
// runtime_args
{
    "tlbr": [0, 0, 0, 0],
    "size_hw", [0, 0]
}
```

### Crop
```
// case 1. 之前的size可固定
// static
{
    "type": "CenterCrop",
    "tlbr": [0, 0, 0, 0],
    "size_hw": [0, 0],
    "dynamic": false
}
// runtime_args
null

// case 2. 之前的size不确定
// static
{
    "type": "CenterCrop",
    "size_hw": [0, 0],
    "dynamic", true
}
// runtime_args
{
    "tlbr": [0, 0, 0, 0]
}
```
### HWC2CHW
```
// static
{
    "type": "HWC2CHW"
}
// runtime_args
null
```
