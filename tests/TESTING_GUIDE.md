# Testing Guide for Camera Super Resolution Sample

## Overview

This guide provides comprehensive information about testing the camera_super_resolution sample in the Vulkan Samples project.

## Quick Start

### 1. Build with Tests Enabled

```bash
# Configure with tests enabled
cmake -G Xcode -Bbuild/mac-xcode \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_OSX_SYSROOT=macosx \
    -DCMAKE_OSX_DEPLOYMENT_TARGET=11.0 \
    -DVKB_BUILD_TESTS=ON

# Build everything including tests
cmake --build build/mac-xcode --config Release --target all -j$(sysctl -n hw.ncpu)
```

### 2. Run All Tests

```bash
cd build/mac-xcode
ctest --output-on-failure -C Release
```

### 3. Run Specific Tests

```bash
# Unit tests
./tests/unit/Release/camera_super_resolution_test
./tests/unit/Release/vgf_integration_test

# Integration tests
./tests/integration/Release/sample_integration_test
./tests/integration/Release/vulkan_init_test
```

## Test Categories

### Unit Tests

Unit tests verify individual components work correctly in isolation.

#### Camera Super Resolution Tests (`camera_super_resolution_test`)
- **Instance Creation**: Validates sample instantiation
- **VGF Data Initialization**: Tests VGF data structure setup
- **Tensor Validation**: Tests tensor dimensions and formats
- **Parameter Validation**: Tests zoom levels and output dimensions
- **Memory Alignment**: Tests memory alignment requirements

#### Tensor and Data Graph Common Tests (`tensor_and_data_graph_common_test`)
- **MultidimensionalArrayView**: Tests array view functionality
- **Data Structures**: Tests TensorInfo and VgfData structures
- **Validation Functions**: Tests tensor validation logic
- **Type Sizes**: Tests data type size calculations
- **Memory Alignment**: Tests tensor memory alignment

#### Load VGF Tests (`load_vgf_test`)
- **Valid VGF Loading**: Tests loading a valid VGF file
- **Error Handling**: Tests error cases (non-existent files, corrupt data)
- **Input/Output Validation**: Tests multiple inputs and outputs support
- **Constant Tensors**: Tests constant tensor loading with correct IDs
- **SPIR-V Code**: Tests SPIR-V module validation
- **Binding IDs**: Tests uniqueness of tensor binding IDs
- **Dimensions**: Tests tensor dimension validation
- **Data Size**: Tests constant tensor data size validation

#### VGF Integration Tests (`vgf_integration_test`)
- **Decoder Creation**: Tests VGF decoder instantiation
- **Error Codes**: Tests VGF error handling
- **File Format**: Tests VGF file format validation
- **Tensor Validation**: Checks tensor dimensions and calculations
- **Zoom Level Validation**: Verifies zoom parameter handling
- **Output Dimensions**: Tests output size calculations
- **Descriptor Pool**: Validates descriptor pool requirements
- **Memory Alignment**: Checks memory alignment requirements

#### VGF Integration Tests
- **Decoder Creation**: Tests VGF decoder initialization
- **Error Handling**: Validates error code handling
- **File Format**: Checks VGF file format validation
- **Buffer Requirements**: Tests buffer size requirements
- **Tensor Metadata**: Validates tensor metadata handling
- **Graph Structure**: Tests graph structure validation

### Integration Tests

Integration tests verify that components work together correctly.

#### Sample Integration Tests
- **Command Line Parsing**: Tests argument parsing
- **Configuration**: Validates sample configuration
- **Asset Paths**: Checks asset path validation
- **Lifecycle**: Tests sample lifecycle stages
- **Frame Timing**: Validates frame timing calculations
- **Benchmark Mode**: Tests benchmark functionality
- **Headless Mode**: Validates headless operation

#### Vulkan Initialization Tests
- **Instance Creation**: Tests Vulkan instance setup
- **Extensions**: Validates required extensions
- **Validation Layers**: Tests validation layer setup
- **Device Selection**: Checks physical device selection
- **Queue Families**: Validates queue family requirements
- **Swapchain**: Tests swapchain configuration
- **Descriptor Pools**: Validates descriptor pool setup
- **Tensor Features**: Tests ARM tensor extension features

## Test Execution Options

### Running with Filters

```bash
# Run only tensor-related tests
./tests/unit/Release/camera_super_resolution_test --gtest_filter="*Tensor*"

# Run all tests except integration tests
./tests/unit/Release/camera_super_resolution_test --gtest_filter="-*Integration*"

# Run only load_vgf tests
./tests/unit/Release/load_vgf_test --gtest_filter="LoadVgfTest.*"
```

### Running Individual Test Suites

```bash
# Run camera super resolution unit tests
export DYLD_LIBRARY_PATH=/Users/thocot01/VulkanSDK/1.4.328.1/macOS/lib:$DYLD_LIBRARY_PATH
export LD_LIBRARY_PATH=$PWD/deploy/lib:$LD_LIBRARY_PATH
export VK_ADD_LAYER_PATH=$PWD/deploy/share/vulkan/explicit_layer.d
export VK_INSTANCE_LAYERS=VK_LAYER_ML_Graph_Emulation:VK_LAYER_ML_Tensor_Emulation
./build/mac-xcode/tests/unit/bin/Release/arm64/Release/camera_super_resolution_test

# Run tensor and data graph common tests
./build/mac-xcode/tests/unit/bin/Release/arm64/Release/tensor_and_data_graph_common_test

# Run load_vgf tests
./build/mac-xcode/tests/unit/bin/Release/arm64/Release/load_vgf_test

# Run VGF integration tests
./build/mac-xcode/tests/unit/bin/Release/arm64/Release/vgf_integration_test
```

### Running with Repeat

```bash
# Run tests 10 times to check for flakiness
./tests/unit/Release/camera_super_resolution_test --gtest_repeat=10
```

### Running with Shuffle

```bash
# Run tests in random order
./tests/unit/Release/camera_super_resolution_test --gtest_shuffle
```

### Verbose Output

```bash
# Show all test output
ctest --verbose

# Or with individual test executables
./tests/unit/Release/camera_super_resolution_test --gtest_print_time=1
```

## Writing New Tests

### Test Structure

```cpp
#include <gtest/gtest.h>
#include <gmock/gmock.h>

// Test fixture (optional)
class MyTestFixture : public ::testing::Test
{
  protected:
    void SetUp() override
    {
        // Setup code
    }

    void TearDown() override
    {
        // Cleanup code
    }
};

// Test with fixture
TEST_F(MyTestFixture, TestName)
{
    // Test code
    EXPECT_EQ(1, 1);
}

// Standalone test
TEST(TestSuiteName, TestName)
{
    // Test code
    ASSERT_TRUE(true);
}
```

### Common Assertions

```cpp
// Equality
EXPECT_EQ(expected, actual);
ASSERT_EQ(expected, actual);

// Inequality
EXPECT_NE(val1, val2);

// Boolean
EXPECT_TRUE(condition);
EXPECT_FALSE(condition);

// Comparison
EXPECT_LT(val1, val2);  // Less than
EXPECT_LE(val1, val2);  // Less than or equal
EXPECT_GT(val1, val2);  // Greater than
EXPECT_GE(val1, val2);  // Greater than or equal

// Floating point
EXPECT_FLOAT_EQ(expected, actual);
EXPECT_DOUBLE_EQ(expected, actual);

// Null checks
EXPECT_EQ(ptr, nullptr);
EXPECT_NE(ptr, nullptr);

// String
EXPECT_STREQ(str1, str2);
```

### Adding Tests to Build

1. Create test file in `tests/unit/` or `tests/integration/`
2. Add to corresponding `CMakeLists.txt`:

```cmake
add_executable(my_new_test
    my_new_test.cpp
)

target_link_libraries(my_new_test
    PRIVATE
        gtest_main
        gmock
        # Add other dependencies
)

gtest_discover_tests(my_new_test)
```

## Continuous Integration

Tests run automatically on:
- Push to main/develop branches
- Pull requests to main/develop branches

See `.github/workflows/tests.yml` for CI configuration.

## Debugging Tests

### Running in Debugger

```bash
# macOS with lldb
lldb ./tests/unit/Release/camera_super_resolution_test
(lldb) run --gtest_filter="TestName"

# Or with Xcode
open build/mac-xcode/vulkan_samples.xcodeproj
# Set scheme to test target and run
```

### Common Issues

#### Tests Don't Build
- Ensure `VKB_BUILD_TESTS=ON` is set
- Check CMake configuration output
- Verify all dependencies are available

#### Tests Fail to Link
- Check that all required libraries are linked
- Verify target dependencies in CMakeLists.txt

#### Tests Crash
- Run with debugger to get stack trace
- Check for null pointer dereferences
- Verify resource initialization

#### Tests Are Flaky
- Use `--gtest_repeat` to reproduce
- Check for race conditions
- Verify proper cleanup in TearDown()

## Best Practices

### Test Design
1. **One Concept Per Test**: Each test should verify one specific behavior
2. **Clear Names**: Test names should describe what is being tested
3. **Arrange-Act-Assert**: Structure tests with setup, execution, and verification
4. **Independent Tests**: Tests should not depend on each other
5. **Fast Tests**: Keep unit tests fast (< 1 second)

### Test Maintenance
1. **Update Tests with Code**: Keep tests in sync with implementation
2. **Remove Obsolete Tests**: Delete tests for removed features
3. **Refactor Tests**: Apply same quality standards as production code
4. **Document Complex Tests**: Add comments for non-obvious test logic

### Coverage Goals
- Aim for 80%+ code coverage for critical paths
- Test edge cases and error conditions
- Test both success and failure scenarios
- Include performance-critical code in benchmarks

## Performance Testing

### Benchmark Mode Tests

```bash
# Run sample in benchmark mode
./build/mac-xcode/app/bin/Release/arm64/Release/vulkan_samples \
    sample camera_super_resolution \
    --benchmark \
    --stop-after-frame 1000
```

### Profiling Tests

```bash
# Build with profiling enabled
cmake -DVKB_PROFILING=ON -DVKB_BUILD_TESTS=ON

# Run with Tracy profiler
# (Requires Tracy profiler to be running)
```

## Resources

- [Google Test Documentation](https://google.github.io/googletest/)
- [Google Mock Documentation](https://google.github.io/googletest/gmock_for_dummies.html)
- [Vulkan Samples Documentation](https://docs.vulkan.org/samples/latest/)
- [CMake Testing Documentation](https://cmake.org/cmake/help/latest/manual/ctest.1.html)

## Support

For issues or questions about testing:
1. Check this guide and README.md
2. Review existing tests for examples
3. Check CI logs for test failures
4. Open an issue with test failure details
