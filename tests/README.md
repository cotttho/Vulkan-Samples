# Vulkan Samples Tests

This directory contains unit and integration tests for the Vulkan Samples project, with a focus on the camera_super_resolution sample.

## Building Tests

To build the tests, enable the `VKB_BUILD_TESTS` CMake option:

```bash
cmake -G Xcode -Bbuild/mac-xcode -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_SYSROOT=macosx \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DVKB_BUILD_TESTS=ON

cmake --build build/mac-xcode --config Release --target all -j$(sysctl -n hw.ncpu)
```

## Running Tests

### Run All Tests

```bash
cd build/mac-xcode
ctest --output-on-failure
```

### Run Specific Test Suite

```bash
# Run unit tests only
./tests/unit/Release/camera_super_resolution_test

# Run VGF integration tests
./tests/unit/Release/vgf_integration_test

# Run sample integration tests
./tests/integration/Release/sample_integration_test

# Run Vulkan initialization tests
./tests/integration/Release/vulkan_init_test
```

### Run Tests with Verbose Output

```bash
ctest --verbose
```

### Run Tests with Specific Filter

```bash
# Run only tests matching a pattern
./tests/unit/Release/camera_super_resolution_test --gtest_filter="TensorTest.*"
```

## Test Structure

### Unit Tests (`tests/unit/`)

Unit tests focus on testing individual components and functions in isolation:

- **camera_super_resolution_test.cpp**: Tests for the camera super resolution sample
  - Instance creation
  - VGF data structure initialization
  - Tensor dimension validation
  - Zoom level validation
  - Output dimension calculations
  - Descriptor pool requirements
  - Memory alignment

- **vgf_integration_test.cpp**: Tests for VGF library integration
  - Decoder creation
  - Error code handling
  - File format validation
  - Buffer requirements
  - Tensor metadata
  - Graph structure validation

### Integration Tests (`tests/integration/`)

Integration tests verify that components work together correctly:

- **sample_integration_test.cpp**: Tests for full sample execution
  - Command line argument parsing
  - Sample configuration
  - Asset path validation
  - Sample lifecycle
  - Frame timing
  - Benchmark mode
  - Headless mode

- **vulkan_init_test.cpp**: Tests for Vulkan initialization
  - Instance creation
  - Extension requirements
  - Validation layers
  - Physical device selection
  - Queue family requirements
  - Swapchain configuration
  - Descriptor pools
  - Tensor feature requirements

## Test Framework

The tests use [Google Test](https://github.com/google/googletest) (gtest) and Google Mock (gmock) frameworks.

### Writing New Tests

1. Create a new test file in the appropriate directory (`unit/` or `integration/`)
2. Include the necessary headers:
   ```cpp
   #include <gtest/gtest.h>
   #include <gmock/gmock.h>
   ```
3. Write test cases using the `TEST` or `TEST_F` macros
4. Add the test file to the corresponding `CMakeLists.txt`

### Test Naming Conventions

- Test suite names should be descriptive and use PascalCase
- Test names should describe what is being tested
- Use `TEST_F` for tests that need a fixture
- Use `TEST` for simple standalone tests

Example:
```cpp
TEST(TensorTest, ValidateTensorDimensions)
{
    // Test code here
}
```

## Continuous Integration

Tests are automatically run as part of the CI/CD pipeline. All tests must pass before code can be merged.

## Test Coverage

To generate test coverage reports (requires additional tools):

```bash
# Build with coverage flags
cmake -DCMAKE_BUILD_TYPE=Debug -DVKB_BUILD_TESTS=ON -DCMAKE_CXX_FLAGS="--coverage"
cmake --build build --target all
ctest
lcov --capture --directory . --output-file coverage.info
genhtml coverage.info --output-directory coverage_report
```

## Troubleshooting

### Tests Fail to Build

- Ensure `VKB_BUILD_TESTS=ON` is set
- Check that all dependencies are properly installed
- Verify that the Vulkan SDK is properly configured

### Tests Fail to Run

- Make sure you're running from the correct directory
- Check that required assets are available
- Verify Vulkan drivers are properly installed
- For macOS, ensure the Vulkan SDK environment is set up

### Specific Test Failures

- Check test output for detailed error messages
- Run tests with `--gtest_filter` to isolate failing tests
- Use `--gtest_repeat` to check for flaky tests

## Contributing

When adding new features or fixing bugs:

1. Write tests that cover the new functionality
2. Ensure all existing tests still pass
3. Add documentation for new test cases
4. Follow the existing test structure and naming conventions

## License

Copyright (c) 2025, Arm Limited and Contributors

SPDX-License-Identifier: Apache-2.0
