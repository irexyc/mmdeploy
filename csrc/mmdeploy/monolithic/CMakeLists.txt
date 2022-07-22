# Copyright (c) OpenMMLab. All rights reserved.

# all-in-one library
if (NOT MMDEPLOY_MONOLITHIC)
    return ()
endif ()

add_library(mmdeploy ${MMDEPLOY_LIB_TYPE})
target_link_libraries(mmdeploy PRIVATE ${GLOBAL_OBJ})
target_include_directories(mmdeploy PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/../apis/c>
    $<INSTALL_INTERFACE:include>)
set_target_properties(mmdeploy PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})

target_link_libraries(MMDeployLibs INTERFACE mmdeploy)
if (NOT MMDEPLOY_SHARED_LIBS)
    target_link_libraries(MMDeployStaticModules INTERFACE mmdeploy)
endif ()
mmdeploy_export(mmdeploy)
