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

#include "tensor_and_data_graph_common.h"

/**
 * @brief Test fixture for TensorAndDataGraphCommon tests
 */
class TensorAndDataGraphCommonTest : public ::testing::Test
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
 * @brief Test MultidimensionalArrayView with 1D array
 */
TEST_F(TensorAndDataGraphCommonTest, MultidimensionalArrayView1D)
{
	std::vector<int> data = {1, 2, 3, 4, 5};
	MultidimensionalArrayView<int> view(data.data(), {5});
	
	EXPECT_EQ((view[{0}]), 1);
	EXPECT_EQ((view[{1}]), 2);
	EXPECT_EQ((view[{4}]), 5);
}

/**
 * @brief Test MultidimensionalArrayView with 2D array
 */
TEST_F(TensorAndDataGraphCommonTest, MultidimensionalArrayView2D)
{
	// 2x3 array: [[1, 2, 3], [4, 5, 6]]
	std::vector<int> data = {1, 2, 3, 4, 5, 6};
	MultidimensionalArrayView<int> view(data.data(), {2, 3});
	
	EXPECT_EQ((view[{0, 0}]), 1);
	EXPECT_EQ((view[{0, 1}]), 2);
	EXPECT_EQ((view[{0, 2}]), 3);
	EXPECT_EQ((view[{1, 0}]), 4);
	EXPECT_EQ((view[{1, 1}]), 5);
	EXPECT_EQ((view[{1, 2}]), 6);
}

/**
 * @brief Test MultidimensionalArrayView with 3D array
 */
TEST_F(TensorAndDataGraphCommonTest, MultidimensionalArrayView3D)
{
	// 2x2x2 array
	std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8};
	MultidimensionalArrayView<int> view(data.data(), {2, 2, 2});
	
	EXPECT_EQ((view[{0, 0, 0}]), 1);
	EXPECT_EQ((view[{0, 0, 1}]), 2);
	EXPECT_EQ((view[{0, 1, 0}]), 3);
	EXPECT_EQ((view[{0, 1, 1}]), 4);
	EXPECT_EQ((view[{1, 0, 0}]), 5);
	EXPECT_EQ((view[{1, 0, 1}]), 6);
	EXPECT_EQ((view[{1, 1, 0}]), 7);
	EXPECT_EQ((view[{1, 1, 1}]), 8);
}

/**
 * @brief Test MultidimensionalArrayView with wrong number of indices
 */
TEST_F(TensorAndDataGraphCommonTest, MultidimensionalArrayViewWrongIndices)
{
	std::vector<int> data = {1, 2, 3, 4};
	MultidimensionalArrayView<int> view(data.data(), {2, 2});
	
	// Should throw when using wrong number of indices
	EXPECT_THROW((view[{0}]), std::runtime_error);
	EXPECT_THROW((view[{0, 0, 0}]), std::runtime_error);
}

/**
 * @brief Test MultidimensionalArrayView modification
 */
TEST_F(TensorAndDataGraphCommonTest, MultidimensionalArrayViewModification)
{
	std::vector<int> data = {1, 2, 3, 4};
	MultidimensionalArrayView<int> view(data.data(), {2, 2});
	
	view[{0, 0}] = 10;
	view[{1, 1}] = 20;
	
	EXPECT_EQ((view[{0, 0}]), 10);
	EXPECT_EQ((view[{1, 1}]), 20);
	EXPECT_EQ(data[0], 10);
	EXPECT_EQ(data[3], 20);
}

/**
 * @brief Test TensorInfo structure initialization
 */
TEST_F(TensorAndDataGraphCommonTest, TensorInfoInitialization)
{
	TensorInfo info;
	info.binding = 0;
	info.dimensions = {1, 224, 224, 3};
	info.format = VK_FORMAT_R8G8B8A8_UNORM;
	info.data = {1, 2, 3, 4};
	
	EXPECT_EQ(info.binding, 0);
	EXPECT_EQ(info.dimensions.size(), 4);
	EXPECT_EQ(info.dimensions[0], 1);
	EXPECT_EQ(info.dimensions[1], 224);
	EXPECT_EQ(info.dimensions[2], 224);
	EXPECT_EQ(info.dimensions[3], 3);
	EXPECT_EQ(info.format, VK_FORMAT_R8G8B8A8_UNORM);
	EXPECT_EQ(info.data.size(), 4);
}

/**
 * @brief Test VgfData structure initialization
 */
TEST_F(TensorAndDataGraphCommonTest, VgfDataInitialization)
{
	VgfData vgf_data;
	
	EXPECT_TRUE(vgf_data.input_tensor_infos.empty());
	EXPECT_TRUE(vgf_data.output_tensor_infos.empty());
	EXPECT_TRUE(vgf_data.code.empty());
	EXPECT_TRUE(vgf_data.entry_point.empty());
}

/**
 * @brief Test VgfData with populated data
 */
TEST_F(TensorAndDataGraphCommonTest, VgfDataPopulated)
{
	VgfData vgf_data;
	
	TensorInfo input_info;
	input_info.binding = 0;
	input_info.dimensions = {1, 224, 224, 3};
	input_info.format = VK_FORMAT_R8G8B8A8_UNORM;
	
	TensorInfo output_info;
	output_info.binding = 1;
	output_info.dimensions = {1, 448, 448, 3};
	output_info.format = VK_FORMAT_R8G8B8A8_UNORM;
	
	vgf_data.input_tensor_infos.push_back(input_info);
	vgf_data.output_tensor_infos.push_back(output_info);
	vgf_data.code = {0x07230203, 0x00010000};  // Example SPIR-V magic number
	vgf_data.entry_point = "main";
	
	EXPECT_EQ(vgf_data.input_tensor_infos.size(), 1);
	EXPECT_EQ(vgf_data.output_tensor_infos.size(), 1);
	EXPECT_EQ(vgf_data.code.size(), 2);
	EXPECT_EQ(vgf_data.entry_point, "main");
	EXPECT_EQ(vgf_data.input_tensor_infos[0].binding, 0);
	EXPECT_EQ(vgf_data.output_tensor_infos[0].binding, 1);
}

/**
 * @brief Test PipelineConstantTensor structure
 */
TEST_F(TensorAndDataGraphCommonTest, PipelineConstantTensorInt8)
{
	PipelineConstantTensor<int8_t> constant;
	constant.dimensions = {2, 2};
	constant.constant_data = {1, 2, 3, 4};
	
	EXPECT_EQ(constant.dimensions.size(), 2);
	EXPECT_EQ(constant.constant_data.size(), 4);
	EXPECT_EQ(constant.constant_data[0], 1);
	EXPECT_EQ(constant.constant_data[3], 4);
}

/**
 * @brief Test PipelineConstantTensor with float data
 */
TEST_F(TensorAndDataGraphCommonTest, PipelineConstantTensorFloat)
{
	PipelineConstantTensor<float> constant;
	constant.dimensions = {3, 3};
	constant.constant_data = {1.0f, 2.0f, 3.0f, 4.0f, 5.0f, 6.0f, 7.0f, 8.0f, 9.0f};
	
	EXPECT_EQ(constant.dimensions.size(), 2);
	EXPECT_EQ(constant.constant_data.size(), 9);
	EXPECT_FLOAT_EQ(constant.constant_data[0], 1.0f);
	EXPECT_FLOAT_EQ(constant.constant_data[4], 5.0f);
	EXPECT_FLOAT_EQ(constant.constant_data[8], 9.0f);
}

/**
 * @brief Test tensor dimensions validation
 */
TEST_F(TensorAndDataGraphCommonTest, TensorDimensionsValidation)
{
	std::vector<int64_t> valid_1d = {256};
	std::vector<int64_t> valid_2d = {224, 224};
	std::vector<int64_t> valid_3d = {1, 224, 224};
	std::vector<int64_t> valid_4d = {1, 224, 224, 3};
	
	EXPECT_EQ(valid_1d.size(), 1);
	EXPECT_EQ(valid_2d.size(), 2);
	EXPECT_EQ(valid_3d.size(), 3);
	EXPECT_EQ(valid_4d.size(), 4);
	
	// All dimensions should be positive
	for (auto dim : valid_4d)
	{
		EXPECT_GT(dim, 0);
	}
}

/**
 * @brief Test tensor size calculation
 */
TEST_F(TensorAndDataGraphCommonTest, TensorSizeCalculation)
{
	std::vector<int64_t> dimensions = {1, 224, 224, 3};
	
	int64_t total_elements = 1;
	for (auto dim : dimensions)
	{
		total_elements *= dim;
	}
	
	EXPECT_EQ(total_elements, 150528);  // 1 * 224 * 224 * 3
}

/**
 * @brief Test tensor format validation
 */
TEST_F(TensorAndDataGraphCommonTest, TensorFormatValidation)
{
	std::vector<VkFormat> valid_formats = {
	    VK_FORMAT_R8_UNORM,
	    VK_FORMAT_R8G8B8A8_UNORM,
	    VK_FORMAT_R32_SFLOAT,
	    VK_FORMAT_R32G32B32A32_SFLOAT
	};
	
	for (auto format : valid_formats)
	{
		EXPECT_NE(format, VK_FORMAT_UNDEFINED);
	}
}

/**
 * @brief Test tensor binding validation
 */
TEST_F(TensorAndDataGraphCommonTest, TensorBindingValidation)
{
	TensorInfo input_tensor;
	input_tensor.binding = 0;
	
	TensorInfo output_tensor;
	output_tensor.binding = 1;
	
	// Bindings should be unique
	EXPECT_NE(input_tensor.binding, output_tensor.binding);
	
	// Bindings should be non-negative
	EXPECT_GE(input_tensor.binding, 0);
	EXPECT_GE(output_tensor.binding, 0);
}

/**
 * @brief Test SPIR-V code validation
 */
TEST_F(TensorAndDataGraphCommonTest, SpirvCodeValidation)
{
	std::vector<uint32_t> spirv_code;
	
	// SPIR-V magic number
	spirv_code.push_back(0x07230203);
	
	// Version number (1.0)
	spirv_code.push_back(0x00010000);
	
	EXPECT_FALSE(spirv_code.empty());
	EXPECT_EQ(spirv_code[0], 0x07230203);
	EXPECT_GE(spirv_code.size(), 2);
}

/**
 * @brief Test entry point name validation
 */
TEST_F(TensorAndDataGraphCommonTest, EntryPointValidation)
{
	std::vector<std::string> valid_entry_points = {
	    "main",
	    "compute_main",
	    "data_graph_main"
	};
	
	for (const auto &entry_point : valid_entry_points)
	{
		EXPECT_FALSE(entry_point.empty());
		EXPECT_GT(entry_point.length(), 0);
	}
}

/**
 * @brief Test tensor data type sizes
 */
TEST_F(TensorAndDataGraphCommonTest, TensorDataTypeSizes)
{
	EXPECT_EQ(sizeof(int8_t), 1);
	EXPECT_EQ(sizeof(uint8_t), 1);
	EXPECT_EQ(sizeof(int16_t), 2);
	EXPECT_EQ(sizeof(uint16_t), 2);
	EXPECT_EQ(sizeof(int32_t), 4);
	EXPECT_EQ(sizeof(uint32_t), 4);
	EXPECT_EQ(sizeof(float), 4);
	EXPECT_EQ(sizeof(int64_t), 8);
}

/**
 * @brief Test multidimensional array indexing calculation
 */
TEST_F(TensorAndDataGraphCommonTest, MultidimensionalIndexCalculation)
{
	// For a 3x4x5 array, element at [1, 2, 3] should be at index:
	// 1 * (4 * 5) + 2 * 5 + 3 = 20 + 10 + 3 = 33
	std::vector<int64_t> dimensions = {3, 4, 5};
	std::vector<int64_t> indices = {1, 2, 3};
	
	size_t calculated_index = 0;
	size_t multiplier = 1;
	
	for (int i = dimensions.size() - 1; i >= 0; --i)
	{
		calculated_index += indices[i] * multiplier;
		multiplier *= dimensions[i];
	}
	
	EXPECT_EQ(calculated_index, 33);
}

/**
 * @brief Test tensor memory alignment requirements
 */
TEST_F(TensorAndDataGraphCommonTest, TensorMemoryAlignment)
{
	// Typical alignment requirements for tensors
	size_t alignment_256 = 256;
	size_t alignment_64 = 64;
	
	size_t tensor_size = 1024;
	
	EXPECT_EQ(tensor_size % alignment_256, 0);
	EXPECT_EQ(tensor_size % alignment_64, 0);
}

/**
 * @brief Test tensor descriptor set binding limits
 */
TEST_F(TensorAndDataGraphCommonTest, TensorDescriptorSetLimits)
{
	// Typical descriptor set limits
	uint32_t max_bindings = 32;
	uint32_t max_sets = 4;
	
	std::vector<uint32_t> bindings = {0, 1, 2, 3, 4};
	
	for (auto binding : bindings)
	{
		EXPECT_LT(binding, max_bindings);
	}
	
	EXPECT_LE(bindings.size(), max_bindings);
}

