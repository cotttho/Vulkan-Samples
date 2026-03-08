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
#include <set>

#include "tensor_and_data_graph_common.h"
#include "filesystem/filesystem.hpp"

/**
 * @brief Test fixture for load_vgf function tests
 */
class LoadVgfTest : public ::testing::Test
{
  protected:
	void SetUp() override
	{
		// Initialize filesystem
		vkb::filesystem::init();
	}

	void TearDown() override
	{
		// Cleanup
	}

	// Helper to check if a VGF file exists
	bool vgf_file_exists(const std::string &path)
	{
		return vkb::filesystem::get()->is_file(path);
	}
};

/**
 * @brief Test loading a valid VGF file
 */
TEST_F(LoadVgfTest, LoadValidVgfFile)
{
	std::string vgf_path = "shaders/camera/camera_super_resolution/vgf/ai_benchmark_v5_xlsr_quant.tflite.mlirbc.vgf";
	
	// Skip test if VGF file doesn't exist
	if (!vgf_file_exists(vgf_path))
	{
		GTEST_SKIP() << "VGF file not found: " << vgf_path;
	}

	std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
	
	EXPECT_NO_THROW({
		VgfData vgf_data = load_vgf(vgf_path, constant_tensors);
		
		// Validate returned data
		EXPECT_FALSE(vgf_data.input_tensor_infos.empty()) << "Should have at least one input";
		EXPECT_FALSE(vgf_data.output_tensor_infos.empty()) << "Should have at least one output";
		EXPECT_FALSE(vgf_data.code.empty()) << "Should have SPIR-V code";
		EXPECT_FALSE(vgf_data.entry_point.empty()) << "Should have entry point";
	});
}

/**
 * @brief Test that load_vgf throws on non-existent file
 */
TEST_F(LoadVgfTest, ThrowsOnNonExistentFile)
{
	std::string invalid_path = "non_existent_file.vgf";
	std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
	
	EXPECT_THROW({
		load_vgf(invalid_path, constant_tensors);
	}, std::runtime_error);
}

/**
 * @brief Test that load_vgf validates input tensors
 */
TEST_F(LoadVgfTest, ValidatesInputTensors)
{
	std::string vgf_path = "shaders/camera/camera_super_resolution/vgf/ai_benchmark_v5_xlsr_quant.tflite.mlirbc.vgf";
	
	if (!vgf_file_exists(vgf_path))
	{
		GTEST_SKIP() << "VGF file not found: " << vgf_path;
	}

	std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
	VgfData vgf_data = load_vgf(vgf_path, constant_tensors);
	
	// Validate input tensor properties
	for (const auto &input : vgf_data.input_tensor_infos)
	{
		EXPECT_FALSE(input.dimensions.empty()) << "Input tensor should have dimensions";
		EXPECT_NE(input.format, VK_FORMAT_UNDEFINED) << "Input tensor should have valid format";
	}
}

/**
 * @brief Test that load_vgf validates output tensors
 */
TEST_F(LoadVgfTest, ValidatesOutputTensors)
{
	std::string vgf_path = "shaders/camera/camera_super_resolution/vgf/ai_benchmark_v5_xlsr_quant.tflite.mlirbc.vgf";
	
	if (!vgf_file_exists(vgf_path))
	{
		GTEST_SKIP() << "VGF file not found: " << vgf_path;
	}

	std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
	VgfData vgf_data = load_vgf(vgf_path, constant_tensors);
	
	// Validate output tensor properties
	for (const auto &output : vgf_data.output_tensor_infos)
	{
		EXPECT_FALSE(output.dimensions.empty()) << "Output tensor should have dimensions";
		EXPECT_NE(output.format, VK_FORMAT_UNDEFINED) << "Output tensor should have valid format";
	}
}

/**
 * @brief Test that load_vgf loads constant tensors correctly
 */
TEST_F(LoadVgfTest, LoadsConstantTensors)
{
	std::string vgf_path = "shaders/camera/camera_super_resolution/vgf/ai_benchmark_v5_xlsr_quant.tflite.mlirbc.vgf";
	
	if (!vgf_file_exists(vgf_path))
	{
		GTEST_SKIP() << "VGF file not found: " << vgf_path;
	}

	std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
	VgfData vgf_data = load_vgf(vgf_path, constant_tensors);
	
	// Validate constant tensors
	for (const auto &constant : constant_tensors)
	{
		ASSERT_NE(constant, nullptr) << "Constant tensor should not be null";
		EXPECT_FALSE(constant->dimensions.empty()) << "Constant tensor should have dimensions";
		EXPECT_FALSE(constant->constant_data.empty()) << "Constant tensor should have data";
		
		// Validate tensor description
		EXPECT_EQ(constant->tensor_description.sType, VK_STRUCTURE_TYPE_TENSOR_DESCRIPTION_ARM);
		EXPECT_EQ(constant->tensor_description.dimensionCount, constant->dimensions.size());
		EXPECT_NE(constant->tensor_description.format, VK_FORMAT_UNDEFINED);
		
		// Validate pipeline constant
		EXPECT_EQ(constant->pipeline_constant.sType, VK_STRUCTURE_TYPE_DATA_GRAPH_PIPELINE_CONSTANT_ARM);
		EXPECT_EQ(constant->pipeline_constant.pNext, &constant->tensor_description);
		EXPECT_NE(constant->pipeline_constant.pConstantData, nullptr);
	}
}

/**
 * @brief Test that constant tensor IDs are correct
 */
TEST_F(LoadVgfTest, ConstantTensorIDsAreCorrect)
{
	std::string vgf_path = "shaders/camera/camera_super_resolution/vgf/ai_benchmark_v5_xlsr_quant.tflite.mlirbc.vgf";
	
	if (!vgf_file_exists(vgf_path))
	{
		GTEST_SKIP() << "VGF file not found: " << vgf_path;
	}

	std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
	VgfData vgf_data = load_vgf(vgf_path, constant_tensors);
	
	// Validate that constant IDs are unique and reasonable
	std::set<uint32_t> constant_ids;
	for (const auto &constant : constant_tensors)
	{
		uint32_t id = constant->pipeline_constant.id;
		EXPECT_TRUE(constant_ids.insert(id).second) << "Constant ID " << id << " is duplicated";
	}
}

/**
 * @brief Test that SPIR-V code is valid
 */
TEST_F(LoadVgfTest, SpirvCodeIsValid)
{
	std::string vgf_path = "shaders/camera/camera_super_resolution/vgf/ai_benchmark_v5_xlsr_quant.tflite.mlirbc.vgf";
	
	if (!vgf_file_exists(vgf_path))
	{
		GTEST_SKIP() << "VGF file not found: " << vgf_path;
	}

	std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
	VgfData vgf_data = load_vgf(vgf_path, constant_tensors);
	
	// Validate SPIR-V code
	EXPECT_FALSE(vgf_data.code.empty()) << "SPIR-V code should not be empty";
	
	// Check SPIR-V magic number (0x07230203)
	if (!vgf_data.code.empty())
	{
		EXPECT_EQ(vgf_data.code[0], 0x07230203) << "First word should be SPIR-V magic number";
	}
}

/**
 * @brief Test that entry point is valid
 */
TEST_F(LoadVgfTest, EntryPointIsValid)
{
	std::string vgf_path = "shaders/camera/camera_super_resolution/vgf/ai_benchmark_v5_xlsr_quant.tflite.mlirbc.vgf";
	
	if (!vgf_file_exists(vgf_path))
	{
		GTEST_SKIP() << "VGF file not found: " << vgf_path;
	}

	std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
	VgfData vgf_data = load_vgf(vgf_path, constant_tensors);
	
	// Validate entry point
	EXPECT_FALSE(vgf_data.entry_point.empty()) << "Entry point should not be empty";
	EXPECT_GT(vgf_data.entry_point.length(), 0) << "Entry point should have non-zero length";
}

/**
 * @brief Test tensor binding IDs are unique
 */
TEST_F(LoadVgfTest, TensorBindingIDsAreUnique)
{
	std::string vgf_path = "shaders/camera/camera_super_resolution/vgf/ai_benchmark_v5_xlsr_quant.tflite.mlirbc.vgf";
	
	if (!vgf_file_exists(vgf_path))
	{
		GTEST_SKIP() << "VGF file not found: " << vgf_path;
	}

	std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
	VgfData vgf_data = load_vgf(vgf_path, constant_tensors);
	
	// Collect all binding IDs
	std::set<uint32_t> binding_ids;
	
	for (const auto &input : vgf_data.input_tensor_infos)
	{
		binding_ids.insert(input.binding);
	}
	
	for (const auto &output : vgf_data.output_tensor_infos)
	{
		binding_ids.insert(output.binding);
	}
	
	// Total unique bindings should equal total tensors
	size_t total_tensors = vgf_data.input_tensor_infos.size() + vgf_data.output_tensor_infos.size();
	EXPECT_EQ(binding_ids.size(), total_tensors) << "All tensor bindings should be unique";
}

/**
 * @brief Test tensor dimensions are valid
 */
TEST_F(LoadVgfTest, TensorDimensionsAreValid)
{
	std::string vgf_path = "shaders/camera/camera_super_resolution/vgf/ai_benchmark_v5_xlsr_quant.tflite.mlirbc.vgf";
	
	if (!vgf_file_exists(vgf_path))
	{
		GTEST_SKIP() << "VGF file not found: " << vgf_path;
	}

	std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
	VgfData vgf_data = load_vgf(vgf_path, constant_tensors);
	
	// Validate all tensor dimensions
	auto validate_dimensions = [](const std::vector<TensorInfo> &tensors, const std::string &type) {
		for (const auto &tensor : tensors)
		{
			EXPECT_FALSE(tensor.dimensions.empty()) << type << " tensor should have dimensions";
			EXPECT_LE(tensor.dimensions.size(), 8) << type << " tensor should have reasonable dimension count";
			
			for (size_t i = 0; i < tensor.dimensions.size(); ++i)
			{
				EXPECT_GT(tensor.dimensions[i], 0) << type << " tensor dimension " << i << " should be positive";
			}
		}
	};
	
	validate_dimensions(vgf_data.input_tensor_infos, "Input");
	validate_dimensions(vgf_data.output_tensor_infos, "Output");
}

/**
 * @brief Test that constant tensor data size is reasonable
 */
TEST_F(LoadVgfTest, ConstantTensorDataSizeIsReasonable)
{
	std::string vgf_path = "shaders/camera/camera_super_resolution/vgf/ai_benchmark_v5_xlsr_quant.tflite.mlirbc.vgf";
	
	if (!vgf_file_exists(vgf_path))
	{
		GTEST_SKIP() << "VGF file not found: " << vgf_path;
	}

	std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
	VgfData vgf_data = load_vgf(vgf_path, constant_tensors);
	
	for (const auto &constant : constant_tensors)
	{
		// Calculate minimum expected size from dimensions
		int64_t min_elements = 1;
		for (int64_t dim : constant->dimensions)
		{
			min_elements *= dim;
		}
		
		// Data size should be at least the minimum (may have padding/alignment)
		EXPECT_GE(constant->constant_data.size(), static_cast<size_t>(min_elements))
		    << "Constant tensor data size should be at least the product of dimensions";
		
		// Data size should not be unreasonably large (less than 10x the minimum)
		EXPECT_LE(constant->constant_data.size(), static_cast<size_t>(min_elements * 10))
		    << "Constant tensor data size should not be excessively larger than dimensions suggest";
	}
}

/**
 * @brief Test multiple inputs support
 */
TEST_F(LoadVgfTest, SupportsMultipleInputs)
{
	std::string vgf_path = "shaders/camera/camera_super_resolution/vgf/ai_benchmark_v5_xlsr_quant.tflite.mlirbc.vgf";
	
	if (!vgf_file_exists(vgf_path))
	{
		GTEST_SKIP() << "VGF file not found: " << vgf_path;
	}

	std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
	VgfData vgf_data = load_vgf(vgf_path, constant_tensors);
	
	// Should support at least one input (may have more)
	EXPECT_GE(vgf_data.input_tensor_infos.size(), 1) << "Should have at least one input";
	
	// If multiple inputs, validate each one
	if (vgf_data.input_tensor_infos.size() > 1)
	{
		for (size_t i = 0; i < vgf_data.input_tensor_infos.size(); ++i)
		{
			EXPECT_FALSE(vgf_data.input_tensor_infos[i].dimensions.empty())
			    << "Input " << i << " should have dimensions";
		}
	}
}

/**
 * @brief Test multiple outputs support
 */
TEST_F(LoadVgfTest, SupportsMultipleOutputs)
{
	std::string vgf_path = "shaders/camera/camera_super_resolution/vgf/ai_benchmark_v5_xlsr_quant.tflite.mlirbc.vgf";
	
	if (!vgf_file_exists(vgf_path))
	{
		GTEST_SKIP() << "VGF file not found: " << vgf_path;
	}

	std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
	VgfData vgf_data = load_vgf(vgf_path, constant_tensors);
	
	// Should support at least one output (may have more)
	EXPECT_GE(vgf_data.output_tensor_infos.size(), 1) << "Should have at least one output";
	
	// If multiple outputs, validate each one
	if (vgf_data.output_tensor_infos.size() > 1)
	{
		for (size_t i = 0; i < vgf_data.output_tensor_infos.size(); ++i)
		{
			EXPECT_FALSE(vgf_data.output_tensor_infos[i].dimensions.empty())
			    << "Output " << i << " should have dimensions";
		}
	}
}
