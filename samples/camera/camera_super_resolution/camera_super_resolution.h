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

#pragma once

#include "vulkan_sample.h"

#include "../tensor_and_data_graph_common.h"

#include <memory>
#include <vector>

/**
* @brief Demonstrates how to use the VK_ARM_tensors and VK_ARM_data_graph extensions to run a camera super resolution model (XLSR).
*/

class CameraSuperResolution : public vkb::VulkanSampleC
{
 public:
   CameraSuperResolution();
   ~CameraSuperResolution();

   void request_gpu_features(vkb::PhysicalDevice &gpu) override;

   bool prepare(const vkb::ApplicationOptions &options) override;

   bool resize(uint32_t width, uint32_t height) override;

   void draw_renderpass(vkb::core::CommandBufferC &command_buffer, RenderTargetType &render_target) override;

   void draw_gui() override;

 private:
   void prepare_descriptor_pool();

   void prepare_input_tensor();
   void prepare_output_tensor();
   void prepare_output_image(uint32_t width, uint32_t height);

   void prepare_data_graph_pipeline();
   void prepare_data_graph_pipeline_descriptor_set();

   void prepare_visualization_pipeline();
   void prepare_visualization_pipeline_descriptor_set();

   std::vector<std::unique_ptr<PipelineConstantTensor<int8_t>>> constant_tensors;
   VgfData vgf_data;

   bool enable_neural_network = true;
   int zoom = 1;

   std::unique_ptr<Tensor>     input_tensor;
   std::unique_ptr<TensorView> input_tensor_view;

   std::unique_ptr<Tensor>     output_tensor;
   std::unique_ptr<TensorView> output_tensor_view;

   std::unique_ptr<vkb::core::Image>     output_image;
   std::unique_ptr<vkb::core::ImageView> output_image_view;

   // Structs to hold everything needed for constant tensors (Weights and Bias)
   std::unique_ptr<PipelineConstantTensor<float>> weights_constant_tensor;
   std::unique_ptr<PipelineConstantTensor<float>> bias_constant_tensor;

   // Common descriptor pool which can allocate descriptors for tensors and images.
   // We're only allocating a small number of descriptors of a few types, so this simple approach works quite well.
   VkDescriptorPool descriptor_pool = VK_NULL_HANDLE;

   std::unique_ptr<DataGraphPipelineLayout>  data_graph_pipeline_layout;
   std::unique_ptr<DataGraphPipeline>        data_graph_pipeline;
   std::unique_ptr<DataGraphPipelineSession> data_graph_pipeline_session;
   VkDescriptorSet                           data_graph_pipeline_descriptor_set = VK_NULL_HANDLE;

   std::unique_ptr<ComputePipelineLayoutWithTensors> visualization_pipeline_layout;
   std::unique_ptr<ComputePipelineWithTensors>       visualization_pipeline;
   VkDescriptorSet                                   visualization_pipeline_descriptor_set = VK_NULL_HANDLE;
};

std::unique_ptr<CameraSuperResolution> create_camera_super_resolution();
