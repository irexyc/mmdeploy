# Copyright (c) OpenMMLab. All rights reserved.
cmake_minimum_required(VERSION 3.14)
project(mmdeploy_transform_module)

add_subdirectory(transform)
add_subdirectory(fuse_transform)
add_subdirectory(cpu)
add_subdirectory(fuse_cpu)
if ("cuda" IN_LIST MMDEPLOY_TARGET_DEVICES)
    add_subdirectory(cuda)
    add_subdirectory(fuse_cuda)
endif ()

include(${CMAKE_SOURCE_DIR}/cmake/MMDeploy.cmake)
mmdeploy_add_module(${PROJECT_NAME} transform_module.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE mmdeploy::transform
    mmdeploy::fuse_transform)
add_library(mmdeploy::transform_module ALIAS ${PROJECT_NAME})
