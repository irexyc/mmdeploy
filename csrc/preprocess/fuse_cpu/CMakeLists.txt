# Copyright (c) OpenMMLab. All rights reserved.
cmake_minimum_required(VERSION 3.14)
project(mmdeploy_cpu_fuse_transform_impl)

include(${CMAKE_SOURCE_DIR}/cmake/opencv.cmake)
include(${CMAKE_SOURCE_DIR}/cmake/MMDeploy.cmake)

set(SRCS fuse_transform_kernel.cpp)
mmdeploy_add_module(${PROJECT_NAME} "${SRCS}")
target_link_libraries(${PROJECT_NAME}
        PRIVATE mmdeploy::fuse_transform)
add_library(mmdeploy::fuse_transform_impl::cpu ALIAS ${PROJECT_NAME})
