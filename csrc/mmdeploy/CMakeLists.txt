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
    if (MMDEPLOY_MONOLITHIC)
        add_library(mmdeploy ${MMDEPLOY_LIB_TYPE})
        target_link_libraries(mmdeploy PRIVATE ${GLOBAL_OBJ})
        target_include_directories(mmdeploy PUBLIC
            $<INSTALL_INTERFACE:include>)
        set_target_properties(mmdeploy PROPERTIES
            VERSION ${PROJECT_VERSION}
            SOVERSION ${PROJECT_VERSION_MAJOR})
        target_link_libraries(MMDeployLibs INTERFACE mmdeploy)
        mmdeploy_export(mmdeploy)
    endif ()
endif ()
