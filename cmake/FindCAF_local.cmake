if (NOT IS_OSX)
    message(FATAL_ERROR "Locally compiled CAF is supported on OSX for now")
endif()    

message(STATUS "======== USING LOCALLY COMPILED VERSION OF CAF ===============")

message(STATUS "LOCAL CAF PATH - " $ENV{CAF_LOCAL_COMPILE_ROOT})

if (CMAKE_BUILD_TYPE MATCHES Debug)
    set(CAF_BUILD_DIR "_debug")
else()
    set(CAF_BUILD_DIR "_release")
endif()

set(CAF_INCLUDE_PATH $ENV{CAF_LOCAL_COMPILE_ROOT}/libcaf_core $ENV{CAF_LOCAL_COMPILE_ROOT}/libcaf_io)

# this is needed because the auto generated build config is in this directory
set(CAF_INCLUDE_PATH ${CAF_INCLUDE_PATH} $ENV{CAF_LOCAL_COMPILE_ROOT}/${CAF_BUILD_DIR}/libcaf_core)

set (CAF_LIB_CORE_PATH $ENV{CAF_LOCAL_COMPILE_ROOT}/${CAF_BUILD_DIR}/lib/libcaf_core.dylib)
set (CAF_LIB_IO_PATH  $ENV{CAF_LOCAL_COMPILE_ROOT}/${CAF_BUILD_DIR}/lib/libcaf_io.dylib)
set (CAF_LIB_OPENSSL_PATH  $ENV{CAF_LOCAL_COMPILE_ROOT}/${CAF_BUILD_DIR}/lib/libcaf_openssl.dylib)

set (CAF_LIBRARIES ${CAF_LIB_CORE_PATH} ${CAF_LIB_IO_PATH} ${CAF_LIB_OPENSSL_PATH})
set (CAF_INCLUDE_DIRS ${CAF_INCLUDE_PATH})  
