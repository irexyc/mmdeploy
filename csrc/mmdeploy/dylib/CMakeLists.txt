# Copyright (c) OpenMMLab. All rights reserved.

project(mmdeploy_dylib)

set(SRCS dylib.cpp)
set(DLLIB)

if (WIN32)
    list(APPEND SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/windows/dylib.cpp
    )
else ()
    list(APPEND SRCS
        ${CMAKE_CURRENT_SOURCE_DIR}/posix/dylib.cpp
    )
    set(DLLIB "dl")
endif ()

_mmdeploy_flatten_modules(_MODEL_LIST MMDeployOptionalModules)
set(_MODULE_STR ${_MODEL_LIST})
list(TRANSFORM _MODULE_STR REPLACE "(.+)" "\"\\1\"")
string(JOIN ",\n    " _MODULE_STR ${_MODULE_STR})
set(_MMDEPLOY_OPTIONAL_MODULES ${_MODULE_STR})

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/config.h.in
    ${CMAKE_CURRENT_SOURCE_DIR}/config.h)

mmdeploy_add_module(${PROJECT_NAME} ${SRCS})
target_link_libraries(${PROJECT_NAME} PUBLIC ${DLLIB})
