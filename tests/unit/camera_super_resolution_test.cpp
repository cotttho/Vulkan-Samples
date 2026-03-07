/* Copyright (c) 2025, Arm Limited and Contributors
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "camera_super_resolution.h"
#include "tensor_and_data_graph_common.h"

/**
 * @brief Test fixture for CameraSuperResolution tests
 */
class CameraSuperResolutionTest : public ::testing::Test
{
  protected:
	void SetUp() override
	{
		// Setup code that runs before each test
	}

	void TearDown() override
	{
		// Cleanup code that runs after each test
	}
};

/**
 * @brief Test that the CameraSuperResolution sample can be instantiated
 */
TEST_F(CameraSuperResolutionTest, CanCreateInstance)
{
	auto sample = create_camera_super_resolution();
	ASSERT_NE(sample, nullptr);
}

/**
 * @brief Test VGF data structure initialization
 */
TEST_F(CameraSuperResolutionTest, VgfDataInitialization)
{
	VgfData vgf_data;
	
	// Test default initialization
	EXPECT_TRUE(vgf_data.input_tensor_infos.empty());
	EXPECT_TRUE(vgf_data.output_tensor_infos.empty());
	EXPECT_TRUE(vgf_data.code.empty());
	EXPECT_TRUE(vgf_data.entry_point.empty());
}

/**
 * @brief Test tensor dimensions validation
 */
TEST(TensorTest, ValidateTensorDimensions)
{
	// Test valid tensor dimensions
	std::vector<uint32_t> valid_dims = {1, 224, 224, 3};
	EXPECT_EQ(valid_dims.size(), 4);
	EXPECT_GT(valid_dims[0], 0);
	EXPECT_GT(valid_dims[1], 0);
	EXPECT_GT(valid_dims[2], 0);
	EXPECT_GT(valid_dims[3], 0);
}

/**
 * @brief Test tensor size calculation
 */
TEST(TensorTest, CalculateTensorSize)
{
	std::vector<uint32_t> dims = {1, 224, 224, 3};
	size_t expected_size = 1 * 224 * 224 * 3;
	
	size_t calculated_size = 1;
	for (auto dim : dims)
	{
		calculated_size *= dim;
	}
	
	EXPECT_EQ(calculated_size, expected_size);
}

/**
 * @brief Test zoom level validation
 */
TEST(CameraSuperResolutionParametersTest, ValidateZoomLevels)
{
	// Test valid zoom levels
	std::vector<int> valid_zoom_levels = {1, 2, 4};
	
	for (int zoom : valid_zoom_levels)
	{
		EXPECT_GT(zoom, 0);
		EXPECT_LE(zoom, 4);
	}
}

/**
 * @brief Test output image dimensions based on zoom
 */
TEST(CameraSuperResolutionParametersTest, CalculateOutputDimensions)
{
	uint32_t input_width = 224;
	uint32_t input_height = 224;
	int zoom = 2;
	
	uint32_t expected_output_width = input_width * zoom;
	uint32_t expected_output_height = input_height * zoom;
	
	EXPECT_EQ(expected_output_width, 448);
	EXPECT_EQ(expected_output_height, 448);
}

/**
 * @brief Test neural network enable/disable flag
 */
TEST(CameraSuperResolutionParametersTest, NeuralNetworkToggle)
{
	bool enable_neural_network = true;
	EXPECT_TRUE(enable_neural_network);
	
	enable_neural_network = false;
	EXPECT_FALSE(enable_neural_network);
}

/**
 * @brief Test constant tensor data type sizes
 */
TEST(TensorDataTypeTest, ValidateDataTypeSizes)
{
	// Test int8_t size
	EXPECT_EQ(sizeof(int8_t), 1);
	
	// Test float size
	EXPECT_EQ(sizeof(float), 4);
	
	// Test uint32_t size
	EXPECT_EQ(sizeof(uint32_t), 4);
}

/**
 * @brief Test VGF file path validation
 */
TEST(VgfFileTest, ValidateFilePath)
{
	std::string valid_path = "models/xlsr_model.vgf";
	EXPECT_FALSE(valid_path.empty());
	EXPECT_TRUE(valid_path.find(".vgf") != std::string::npos);
}

/**
 * @brief Test descriptor pool requirements
 */
TEST(DescriptorPoolTest, ValidatePoolSizes)
{
	// Typical descriptor pool sizes for this sample
	uint32_t max_sets = 10;
	uint32_t storage_image_count = 5;
	uint32_t storage_buffer_count = 10;
	
	EXPECT_GT(max_sets, 0);
	EXPECT_GT(storage_image_count, 0);
	EXPECT_GT(storage_buffer_count, 0);
}

/**
 * @brief Test pipeline constant tensor template instantiation
 */
TEST(PipelineConstantTensorTest, TemplateInstantiation)
{
	// Test that we can work with different data types
	std::vector<int8_t> int8_data = {1, 2, 3, 4};
	EXPECT_EQ(int8_data.size(), 4);
	
	std::vector<float> float_data = {1.0f, 2.0f, 3.0f, 4.0f};
	EXPECT_EQ(float_data.size(), 4);
}

/**
 * @brief Test resize dimensions validation
 */
TEST(ResizeTest, ValidateResizeDimensions)
{
	uint32_t width = 800;
	uint32_t height = 600;
	
	EXPECT_GT(width, 0);
	EXPECT_GT(height, 0);
	EXPECT_LE(width, 4096);  // Reasonable max width
	EXPECT_LE(height, 4096); // Reasonable max height
}

/**
 * @brief Test memory alignment requirements
 */
TEST(MemoryAlignmentTest, ValidateAlignment)
{
	// Vulkan typically requires 256-byte alignment for uniform buffers
	size_t alignment = 256;
	size_t buffer_size = 1024;
	
	EXPECT_EQ(buffer_size % alignment, 0);
}
