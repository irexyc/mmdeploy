# Copyright (c) OpenMMLab. All rights reserved.
cmake_minimum_required(VERSION 3.14)
project(mmdeploy_preprocess_demo)

find_package(OpenCV REQUIRED)
find_package(spdlog REQUIRED)

set(TARGET_NAME preprocess)
add_executable(${TARGET_NAME} image_preprocess.cpp)
target_include_directories(${TARGET_NAME} PRIVATE ${CMAKE_SOURCE_DIR}/csrc/
    ${CMAKE_SOURCE_DIR}/csrc/utils/opencv/
    ${CMAKE_SOURCE_DIR}/csrc/experimental
    ${CMAKE_SOURCE_DIR}/third_party/outcome
    ${CMAKE_SOURCE_DIR}/third_party/json)

target_link_libraries(${TARGET_NAME} -Wl,--whole-archive ${MMDEPLOY_STATIC_MODULES} -Wl,--no-whole-archive)
target_link_libraries(${TARGET_NAME} -Wl,--no-as-needed ${MMDEPLOY_DYNAMIC_MODULES} -Wl,--as-needed)
target_link_libraries(${TARGET_NAME} ${OpenCV_LIBS} spdlog::spdlog)
