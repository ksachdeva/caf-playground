set(SAMPLES_PATH ${CMAKE_CURRENT_SOURCE_DIR})
set(SAMPLES_SRC_PATH  "${SAMPLES_PATH}/src" )

if (WIN32)
set(LINK_OPTIONS shlwapi.lib)
endif()

if ("${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
set(LINK_OPTIONS -pthread -ldl -static-libgcc -static-libstdc++)
endif()

if (Darwin)
set(LINK_OPTIONS -pthread)
endif()

# this macro will help create the executables
# for various playground applications
macro(add_sample app_name app_path main_file)
    add_executable(${app_name} ${app_path}/${main_file}.cc)
    target_link_libraries(
        ${app_name}
        caf::caf
        ${LINK_OPTIONS}
    )
endmacro()

add_sample(hello-caf ${SAMPLES_SRC_PATH} hello-caf)


