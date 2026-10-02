cmake_minimum_required(VERSION 3.10)

# Project name and version
project(VIRTUALT_SOCKET_LIB VERSION 1.0.0)

# C standard
set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Compiler flags (cross-platform)
if(MSVC)
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} /W3")
else()
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra")
endif()

# Add static library for vt_socket
add_library(vt_socket STATIC
    Library/vt_socket/vt_socket.c
)

# Include paths
target_include_directories(vt_socket PUBLIC
    ${CMAKE_SOURCE_DIR}/Library/vt_socket
)

# Link Winsock on Windows
if(MSVC)
    target_link_libraries(vt_socket ws2_32)
endif()

# Add static library for vt_process
add_library(vt_process STATIC
    Library/vt_process/vt_process.c
)

# Include paths
target_include_directories(vt_process PUBLIC
    ${CMAKE_SOURCE_DIR}/Library/vt_process
    ${CMAKE_SOURCE_DIR}/Library/vt_socket
)

# Link vt_socket with vt_process
target_link_libraries(vt_process vt_socket)

# Link Winsock on Windows
if(MSVC)
    target_link_libraries(vt_process ws2_32)
endif()

# Add Unity test framework as a static library
add_library(unity STATIC
    Tools/unity/src/unity.c
)

# Include paths for Unity
target_include_directories(unity PUBLIC
    ${CMAKE_SOURCE_DIR}/Tools/unity/src
)

# Test executable
add_executable(vt_process_tests
    Tests/test_vt_process.c
    Common/thought_test_common.c
)

target_include_directories(vt_process_tests PRIVATE
    ${CMAKE_SOURCE_DIR}/Tests
    ${CMAKE_SOURCE_DIR}/Common
    ${CMAKE_SOURCE_DIR}/Library/vt_process
    ${CMAKE_SOURCE_DIR}/Library/vt_socket
    ${CMAKE_SOURCE_DIR}/Tools/unity/src
)

target_link_libraries(vt_process_tests vt_process vt_socket unity)

# Simple process launch test executable
add_executable(test_process_launch
    Tests/test_process_launch.c
)

target_link_libraries(test_process_launch kernel32)