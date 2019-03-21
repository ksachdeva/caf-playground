if (${CMAKE_SYSTEM_NAME} STREQUAL "Windows")
    set(IS_WINDOWS ON)
    add_definitions("-DIS_WINDOWS")
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Darwin")
    set(IS_OSX ON)
    add_definitions("-DIS_OSX")
endif()

if (${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
    set(IS_LINUX ON)
    add_definitions("-DIS_LINUX")
endif()