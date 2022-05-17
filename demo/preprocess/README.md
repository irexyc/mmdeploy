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
./bin/preprocess <cpu or cuda> <path/of/preprocess/config/json/file> <path/of/an/image>
```
