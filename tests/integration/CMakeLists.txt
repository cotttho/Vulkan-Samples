# Copyright (c) 2025, Arm Limited and Contributors
#
# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.16)

# Integration test for full sample execution
add_executable(sample_integration_test
    sample_integration_test.cpp
)

target_link_libraries(sample_integration_test
    PRIVATE
        gtest_main
        gmock
        framework
        vkb__core
)

target_include_directories(sample_integration_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/framework
        ${CMAKE_SOURCE_DIR}/components/core/include
)

# Add test manually instead of using gtest_discover_tests to avoid running the executable during build
add_test(NAME sample_integration_test COMMAND sample_integration_test)

# Integration test for Vulkan initialization
add_executable(vulkan_init_test
    vulkan_init_test.cpp
)

target_link_libraries(vulkan_init_test
    PRIVATE
        gtest_main
        gmock
        framework
        vkb__core
        volk
)

target_include_directories(vulkan_init_test
    PRIVATE
        ${CMAKE_SOURCE_DIR}/framework
        ${CMAKE_SOURCE_DIR}/components/core/include
)

# Add test manually instead of using gtest_discover_tests to avoid running the executable during build
add_test(NAME vulkan_init_test COMMAND vulkan_init_test)
