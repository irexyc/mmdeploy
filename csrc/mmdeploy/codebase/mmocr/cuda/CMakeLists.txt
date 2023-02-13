# Copyright (c) OpenMMLab. All rights reserved.

project(mmdeploy_mmocr_cuda_impl)

if ("cuda" IN_LIST MMDEPLOY_TARGET_DEVICES)
    set(SRCS
        connected_component.cu
        utils.cu
        dbnet.cpp
        panet.cpp
        psenet.cpp
    )
    mmdeploy_add_module(${PROJECT_NAME} EXCLUDE MODULE OPTIONAL
        ${SRCS})
    target_include_directories(${PROJECT_NAME} PRIVATE
        ${CUDA_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} PRIVATE
        mmdeploy_opencv_utils
        mmdeploy::core
        ${CUDA_LIBRARIES}
        cuda)
    mmdeploy_export(${PROJECT_NAME})
endif ()
