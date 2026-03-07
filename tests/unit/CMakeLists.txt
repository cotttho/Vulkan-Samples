# Copyright (c) 2025, Arm Limited and Contributors
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.16)

# Unit test for camera_super_resolution sample
add_executable(camera_super_resolution_test
    camera_super_resolution_test.cpp
)

target_link_libraries(camera_super_resolution_test
    PRIVATE
        gtest_main
        gmock
        camera_super_resolution
        framework
        vkb__core
)

target_include_directories(camera_super_resolution_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/samples/camera/camera_super_resolution
        ${CMAKE_SOURCE_DIR}/samples/camera
        ${CMAKE_SOURCE_DIR}/framework
)

# Add test manually instead of using gtest_discover_tests to avoid running the executable during build
add_test(NAME camera_super_resolution_test COMMAND camera_super_resolution_test)

# Unit test for tensor_and_data_graph_common
add_executable(tensor_and_data_graph_common_test
    tensor_and_data_graph_common_test.cpp
)

target_link_libraries(tensor_and_data_graph_common_test
    PRIVATE
        gtest_main
        gmock
        camera_super_resolution
        framework
        vkb__core
)

target_include_directories(tensor_and_data_graph_common_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/samples/camera/camera_super_resolution
        ${CMAKE_SOURCE_DIR}/samples/camera
        ${CMAKE_SOURCE_DIR}/framework
)

# Add test manually instead of using gtest_discover_tests to avoid running the executable during build
add_test(NAME tensor_and_data_graph_common_test COMMAND tensor_and_data_graph_common_test)

# Unit test for load_vgf function
add_executable(load_vgf_test
    load_vgf_test.cpp
)

target_link_libraries(load_vgf_test
    PRIVATE
        gtest_main
        gmock
        camera_super_resolution
        framework
        vkb__core
        vkb__filesystem
)

target_include_directories(load_vgf_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/samples/camera/camera_super_resolution
        ${CMAKE_SOURCE_DIR}/samples/camera
        ${CMAKE_SOURCE_DIR}/framework
)

# Add test manually instead of using gtest_discover_tests to avoid running the executable during build
add_test(NAME load_vgf_test COMMAND load_vgf_test)
