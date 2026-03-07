#!/bin/bash
# Copyright (c) 2025, Arm Limited and Contributors
#
# SPDX-License-Identifier: Apache-2.0

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}Running Vulkan Samples Tests${NC}"
echo "================================"

# Check if build directory exists
if [ ! -d "build/mac-xcode" ]; then
    echo -e "${RED}Error: build/mac-xcode directory not found${NC}"
    echo "Please build the project first with VKB_BUILD_TESTS=ON"
    exit 1
fi

cd build/mac-xcode

# Run all tests with CTest
echo -e "\n${YELLOW}Running all tests with CTest...${NC}"
if ctest --output-on-failure -C Release; then
    echo -e "${GREEN}✓ All tests passed!${NC}"
else
    echo -e "${RED}✗ Some tests failed${NC}"
    exit 1
fi

# Run individual test suites for detailed output
echo -e "\n${YELLOW}Running unit tests...${NC}"
if [ -f "tests/unit/Release/camera_super_resolution_test" ]; then
    ./tests/unit/Release/camera_super_resolution_test
else
    echo -e "${YELLOW}Warning: camera_super_resolution_test not found${NC}"
fi

if [ -f "tests/unit/Release/vgf_integration_test" ]; then
    ./tests/unit/Release/vgf_integration_test
else
    echo -e "${YELLOW}Warning: vgf_integration_test not found${NC}"
fi

echo -e "\n${YELLOW}Running integration tests...${NC}"
if [ -f "tests/integration/Release/sample_integration_test" ]; then
    ./tests/integration/Release/sample_integration_test
else
    echo -e "${YELLOW}Warning: sample_integration_test not found${NC}"
fi

if [ -f "tests/integration/Release/vulkan_init_test" ]; then
    ./tests/integration/Release/vulkan_init_test
else
    echo -e "${YELLOW}Warning: vulkan_init_test not found${NC}"
fi

echo -e "\n${GREEN}================================${NC}"
echo -e "${GREEN}Test execution completed!${NC}"
