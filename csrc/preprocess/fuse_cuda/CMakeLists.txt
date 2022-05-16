# Copyright (c) OpenMMLab. All rights reserved.
cmake_minimum_required(VERSION 3.14)
include(${CMAKE_SOURCE_DIR}/cmake/cuda.cmake NO_POLICY_SCOPE)
project(mmdeploy_cuda_fuse_transform_impl CUDA CXX)

include(${CMAKE_SOURCE_DIR}/cmake/MMDeploy.cmake)

set(SRCS fuse_transform_kernel.cpp)
mmdeploy_add_module(${PROJECT_NAME} "${SRCS}")
target_link_libraries(${PROJECT_NAME} PRIVATE
        mmdeploy::fuse_transform)
target_include_directories(${PROJECT_NAME}
        PRIVATE ${CUDA_TOOLKIT_ROOT_DIR}/include)
add_library(mmdeploy::fuse_transform_impl::cuda ALIAS ${PROJECT_NAME})
