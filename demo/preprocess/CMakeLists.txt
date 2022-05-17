# Copyright (c) OpenMMLab. All rights reserved.
cmake_minimum_required(VERSION 3.14)
project(mmdeploy_preprocess_demo)

find_package(OpenCV REQUIRED)
# find_package(spdlog REQUIRED)

set(TARGET_NAME preprocess)
add_executable(${TARGET_NAME} image_preprocess.cpp)
# target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/csrc/)

mmdeploy_load_static(${TARGET_NAME} MMDeployStaticModules)
mmdeploy_load_dynamic(${TARGET_NAME} MMDeployDynamicModules)
target_link_libraries(${TARGET_NAME} PRIVATE MMDeployLibs ${OpenCV_LIBS})
