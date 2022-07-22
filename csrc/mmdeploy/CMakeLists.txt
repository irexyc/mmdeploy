# Copyright (c) OpenMMLab. All rights reserved.

add_subdirectory(backend_ops)

if (MMDEPLOY_BUILD_SDK)
    # collect obj when build single library
    set(GLOBAL_OBJ "" CACHE INTERNAL "")

    # include OpenCV for SDK modules since many of them depends on it
    include(${CMAKE_SOURCE_DIR}/cmake/opencv.cmake)

    add_subdirectory(core)
    add_subdirectory(execution)
    add_subdirectory(utils)
    add_subdirectory(archive)
    add_subdirectory(device)
    add_subdirectory(graph)
    add_subdirectory(model)
    add_subdirectory(preprocess)
    add_subdirectory(net)
    add_subdirectory(codebase)
    add_subdirectory(apis/c/mmdeploy)

    # all-in-one library
    add_subdirectory(monolithic)
endif ()
