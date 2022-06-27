# ROCM Module
find_rocm(${ELENA_USE_ROCM})

if(ELENA_USE_ROCM)
  if(NOT ROCM_FOUND)
    message(FATAL_ERROR "Cannot find ROCM, ELENA_USE_ROCM=" ${ELENA_USE_ROCM})
  endif()
  message(STATUS "Build with ROCM support")
  file(GLOB ELENA_RUNTIME_ROCM_SRC src/Runtime/Rocm/*.cpp)
  list(APPEND ELENA_RUNTIME_SRC ${ELENA_RUNTIME_ROCM_SRC})
  list(APPEND ELENA_RUNTIME_LINKER_LIBS ${ROCM_HIPHCC_LIBRARY} ${ROCM_HIPRTC_LIBRARY} ${ROCM_ROCBLAS_LIBRARY})
endif(ELENA_USE_ROCM)