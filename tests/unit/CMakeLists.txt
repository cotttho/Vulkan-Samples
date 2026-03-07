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
        ${CMAKE_SOURCE_DIR}/samples/extensions/tensor_and_data_graph/camera_super_resolution
        ${CMAKE_SOURCE_DIR}/samples/extensions/tensor_and_data_graph
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
        ${CMAKE_SOURCE_DIR}/samples/extensions/tensor_and_data_graph/camera_super_resolution
        ${CMAKE_SOURCE_DIR}/samples/extensions/tensor_and_data_graph
        ${CMAKE_SOURCE_DIR}/framework
)

# Add test manually instead of using gtest_discover_tests to avoid running the executable during build
add_test(NAME tensor_and_data_graph_common_test COMMAND tensor_and_data_graph_common_test)

# Add test for VGF library integration
add_executable(vgf_integration_test
    vgf_integration_test.cpp
)

target_link_libraries(vgf_integration_test
    PRIVATE
        gtest_main
        gmock
        vgf
)

target_include_directories(vgf_integration_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/third_party/ai-ml-sdk-vgf-library/include-c
)

# Add test manually instead of using gtest_discover_tests to avoid running the executable during build
add_test(NAME vgf_integration_test COMMAND vgf_integration_test)
