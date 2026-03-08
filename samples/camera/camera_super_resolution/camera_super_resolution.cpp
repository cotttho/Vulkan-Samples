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

#include "camera_super_resolution.h"

#include <gui.h>
#include <iomanip>
#include <sstream>
#include "stb_image.h"
#include "stb_image_resize.h"
#include "common/vk_common.h"
#include "filesystem/filesystem.hpp"
#include "gltf_loader.h"
#include "stats/stats.h"
#include <vgf/decoder.h>


CameraSuperResolution::CameraSuperResolution()
{
   set_api_version(VK_API_VERSION_1_3);        // Required by the emulation layers

   // Declare that we need the data graph and tensor extensions
   add_device_extension("VK_ARM_tensors");
   add_device_extension("VK_ARM_data_graph");
   // These extensions are dependencies of the above, so we need to add them too.
   add_device_extension("VK_KHR_maintenance5");
   add_device_extension("VK_KHR_deferred_host_operations");
}

CameraSuperResolution::~CameraSuperResolution()
{
   if (data_graph_pipeline_descriptor_set != VK_NULL_HANDLE)
   {
	   vkFreeDescriptorSets(get_device().get_handle(), descriptor_pool, 1, &data_graph_pipeline_descriptor_set);
   }
   if (descriptor_pool != VK_NULL_HANDLE)
   {
	   vkDestroyDescriptorPool(get_device().get_handle(), descriptor_pool, nullptr);
   }

   // Make sure resources created in the render pipeline are destroyed before the Device gets destroyed.
   set_render_pipeline(nullptr);
}

/**
* @brief Overridden to declare that we require some physical device features to be enabled.
*/
void CameraSuperResolution::request_gpu_features(vkb::PhysicalDevice &gpu)
{
   REQUEST_REQUIRED_FEATURE(gpu, VkPhysicalDeviceVulkan12Features, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, shaderInt8);
   REQUEST_REQUIRED_FEATURE(gpu, VkPhysicalDeviceVulkan13Features, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, synchronization2);

   // Enable the features for tensors and data graphs which we intend to use.
   REQUEST_REQUIRED_FEATURE(gpu, VkPhysicalDeviceTensorFeaturesARM, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TENSOR_FEATURES_ARM, tensors);
   REQUEST_REQUIRED_FEATURE(gpu, VkPhysicalDeviceTensorFeaturesARM, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_TENSOR_FEATURES_ARM, shaderTensorAccess);
   REQUEST_REQUIRED_FEATURE(gpu, VkPhysicalDeviceDataGraphFeaturesARM, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_DATA_GRAPH_FEATURES_ARM, dataGraph);

   // Update-after-bind is required for the emulation layer
   REQUEST_REQUIRED_FEATURE(gpu, VkPhysicalDeviceVulkan12Features, VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, descriptorBindingUniformBufferUpdateAfterBind);
}

/**
* @brief Overridden to create and set up Vulkan resources.
*/
bool CameraSuperResolution::prepare(const vkb::ApplicationOptions &options)
{
   if (!VulkanSample::prepare(options))
   {
	   return false;
   }

   volkLoadDevice(get_device().get_handle());

   // We use the GUI framework for labels on the visualization
   create_gui(*window, &get_stats());

   // Create Vulkan resources
   prepare_descriptor_pool();

   vgf_data = load_vgf("shaders/camera/camera_super_resolution/vgf/ai_benchmark_v5_xlsr_quant.tflite.mlirbc.vgf", constant_tensors);

   prepare_input_tensor();
   prepare_output_tensor();
   prepare_output_image(get_render_context().get_surface_extent().width, get_render_context().get_surface_extent().height);
   prepare_data_graph_pipeline();
   prepare_data_graph_pipeline_descriptor_set();
   prepare_visualization_pipeline();
   prepare_visualization_pipeline_descriptor_set();

   // Create a RenderPipeline to blit `output_image` to the swapchain
   std::unique_ptr<vkb::RenderPipeline> render_pipeline = std::make_unique<vkb::RenderPipeline>();
   render_pipeline->add_subpass(std::make_unique<BlitSubpass>(get_render_context()));
   set_render_pipeline(std::move(render_pipeline));

   return true;
}

/*
* Creates a descriptor pool which can be used to allocate descriptors for tensor and image bindings.
* Note we can't use vkb::DescriptorPool because it doesn't know about tensors.
*/
void CameraSuperResolution::prepare_descriptor_pool()
{
   std::vector<VkDescriptorPoolSize> descriptor_pool_sizes = {
	   {VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 10},        // Fairly arbitrary counts
	   {VK_DESCRIPTOR_TYPE_TENSOR_ARM, 10},
   };

   VkDescriptorPoolCreateInfo create_info{VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO};
   create_info.poolSizeCount = descriptor_pool_sizes.size();
   create_info.pPoolSizes    = descriptor_pool_sizes.data();
   create_info.maxSets       = 10;        // Fairly arbitrary
   create_info.flags         = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;

   VK_CHECK(vkCreateDescriptorPool(get_device().get_handle(), &create_info, nullptr, &descriptor_pool));
}

/*
* Creates the Tensor used as input to the neural network and fills it with some initial data.
* Also creates a Tensor View (analogous to an Image View).
*/
void CameraSuperResolution::prepare_input_tensor()
{
   auto image_data = vkb::filesystem::get()->read_file_binary("shaders/camera/camera_super_resolution/assets/div2k_007_lr.png");
   int width, height, channels;
   unsigned char* pixels = stbi_load_from_memory(image_data.data(), static_cast<int>(image_data.size()), &width, &height, &channels, 3);

   int resize_w = vgf_data.input_tensor_infos[0].dimensions[2];
   int resize_h = vgf_data.input_tensor_infos[0].dimensions[1]; // Example dimensions
   // Calculate required memory size: width * height * channels
   unsigned char *resized_pixels = (unsigned char*)malloc(resize_w * resize_h * 3);
   int success = stbir_resize_uint8(
	   pixels,  width,  height,  0,
	   resized_pixels, resize_w, resize_h, 0,
	   3
   );
   input_tensor = std::make_unique<Tensor>(get_device(),
										   TensorBuilder({vgf_data.input_tensor_infos[0].dimensions})
											   .with_tiling(VK_TENSOR_TILING_LINEAR_ARM)
											   .with_usage(VK_TENSOR_USAGE_DATA_GRAPH_BIT_ARM)
											   .with_format(vgf_data.input_tensor_infos[0].format)
											   .with_vma_required_flags(VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT));

   unsigned char* mapped_data = reinterpret_cast<unsigned char*>(input_tensor->map());
   std::memcpy(mapped_data, resized_pixels, resize_w * resize_h * 3);

   input_tensor->unmap();
   stbi_image_free(pixels);

   input_tensor_view = std::make_unique<TensorView>(*input_tensor);
}

/*
* Creates the Tensor used as output from the neural network.
* Also creates a Tensor View (analogous to an Image View).
*/
void CameraSuperResolution::prepare_output_tensor()
{
   output_tensor = std::make_unique<Tensor>(get_device(),
											TensorBuilder(
												{vgf_data.output_tensor_infos[0].dimensions})
												.with_usage(
													VK_TENSOR_USAGE_SHADER_BIT_ARM |
													VK_TENSOR_USAGE_DATA_GRAPH_BIT_ARM)
												.with_format(
													vgf_data.output_tensor_infos[0].format)
   );

   output_tensor_view = std::make_unique<TensorView>(*output_tensor);
}

/*
* Creates the Image used to visualize the two tensors, which is then blitted to the Swapchain.
* Also creates an Image View.
*/
void CameraSuperResolution::prepare_output_image(uint32_t width, uint32_t height)
{
   output_image      = std::make_unique<vkb::core::Image>(get_device(),
														  vkb::core::ImageBuilder(VkExtent3D{width, height, 1})
															  .with_format(VK_FORMAT_R8G8B8A8_UNORM)
															  .with_usage(VK_IMAGE_USAGE_STORAGE_BIT | VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_SRC_BIT));
   output_image_view = std::make_unique<vkb::core::ImageView>(*output_image, VK_IMAGE_VIEW_TYPE_2D, VK_FORMAT_R8G8B8A8_UNORM);
}


/*
* Creates the Pipeline Layout, a Data Graph Pipeline and a Data Graph Pipeline Session used to run the neural network.
*/
void CameraSuperResolution::prepare_data_graph_pipeline()
{
   // Create the Pipeline Layout. This is equivalent to the pipeline layout for compute or data graph pipelines, describing what bind points are available.
   // The neural network has its input tensor on binding 0 and its output tensor at binding 1.
   //
   // In order to create the layout, we just need to know which binding slots are tensors - no further details needed yet.
   std::set<uint32_t> tensor_bindings = {0, 1};
   data_graph_pipeline_layout         = std::make_unique<DataGraphPipelineLayout>(get_device(), tensor_bindings);

   // Create a Pipeline from the layout. This is equivalent to a graphics or compute pipeline and contains a shader module which describes the
   // neural network to execute (see `conv2d.spvasm` for the SPIR-V code). It also requires the description (shape etc.) of the tensors that will
   // be bound to the pipeline.
   std::map<uint32_t, std::map<uint32_t, const VkTensorDescriptionARM *>> tensor_descriptions;
   // All bindings are in set 0
   tensor_descriptions[0] =
	   {
		   // Binding 0 is the input tensor
		   {0, &input_tensor->get_description()},
		   // Binding 1 is the output tensor
		   {1, &output_tensor->get_description()}};

   // Add weights and bias constant tensors, which were prepared and stored earlier.
   std::vector<VkDataGraphPipelineConstantARM *> data_graph_pipeline_constants;
   for (const auto &tensor : constant_tensors)
   {
	   data_graph_pipeline_constants.push_back(&tensor->pipeline_constant);
   }

   VkShaderModule shader_module = vkb::load_shader_from_vector(vgf_data.code, get_device().get_handle());

   data_graph_pipeline = std::make_unique<DataGraphPipeline>(get_device(),
															 data_graph_pipeline_layout->get_handle(),
															 shader_module,
															 //"main",
															 vgf_data.entry_point.c_str(),
															 tensor_descriptions,
															 data_graph_pipeline_constants);

   // Create a Pipeline Session for the Pipeline. Unlike compute and graphics pipelines, data graph pipelines require
   // additional state to be stored (e.g. for intermediate results). This is stored separately to the pipeline itself in
   // 'pipeline session' resource. This requires memory to be allocated and bound to it (similar to a buffer), which is all handled
   // inside our helper class DataGraphPipelineSession
   VmaAllocationCreateInfo alloc_create_info = {};
   data_graph_pipeline_session               = std::make_unique<DataGraphPipelineSession>(get_device(), data_graph_pipeline->get_handle(), alloc_create_info);
}

/*
* Allocates and fills in a Descriptor Set to provide bindings to the Data Graph Pipeline.
*/
void CameraSuperResolution::prepare_data_graph_pipeline_descriptor_set()
{
   // Allocate descriptor set using the layout of the Data Graph Pipeline
   VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
   alloc_info.descriptorPool              = descriptor_pool;
   alloc_info.descriptorSetCount          = 1;
   alloc_info.pSetLayouts                 = &data_graph_pipeline_layout->get_descriptor_set_layout();
   VK_CHECK(vkAllocateDescriptorSets(get_device().get_handle(), &alloc_info, &data_graph_pipeline_descriptor_set));

   // Write bindings to it, telling it which tensors to use as input and output
   std::map<uint32_t, VkWriteDescriptorSetTensorARM> tensor_bindings =
	   {
		   // Binding 0 is the input tensor
		   {0, VkWriteDescriptorSetTensorARM{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_TENSOR_ARM, nullptr, 1, &input_tensor_view->get_handle()}},
		   // Binding 1 is the output tensor
		   {1, VkWriteDescriptorSetTensorARM{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_TENSOR_ARM, nullptr, 1, &output_tensor_view->get_handle()}}};
   write_descriptor_set(get_device().get_handle(), data_graph_pipeline_descriptor_set, {}, tensor_bindings);
}

/*
* Creates the Pipeline Layout and a Compute Pipeline used to run the compute shader which copies input and
* output tensors to an image, so we can see their contents on the screen.
*/
void CameraSuperResolution::prepare_visualization_pipeline()
{
   // Load the compute shader

   vkb::ShaderModule &visualization_comp =
	   get_device().get_resource_cache().request_shader_module(VK_SHADER_STAGE_COMPUTE_BIT, vkb::ShaderSource{"camera/camera_super_resolution/camera_visualization.comp.spv"});

   // Create pipeline layout from the reflected shader code. Note that this will include bindings to Tensor resources, so we use our own
   // class to do this, rather than the sample framework's vkb::PipelineLayout.
   visualization_pipeline_layout = std::make_unique<ComputePipelineLayoutWithTensors>(get_device(), visualization_comp);

   // Create pipeline from this layout and the shader module. Similar to the layout, we use our own class rather than vkb::ComputePipeline.
   visualization_pipeline = std::make_unique<ComputePipelineWithTensors>(get_device(), visualization_pipeline_layout->get_handle(), visualization_comp);
}

/*
* Allocates and fills in a Descriptor Set to provide bindings to the visualization Compute Pipeline.
*/
void CameraSuperResolution::prepare_visualization_pipeline_descriptor_set()
{
   // Allocate descriptor set (if not already allocated; when this function is called due to window resize we just update the existing set rather than allocating a new one)
   if (visualization_pipeline_descriptor_set == VK_NULL_HANDLE)
   {
	   VkDescriptorSetAllocateInfo alloc_info = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO};
	   alloc_info.descriptorPool              = descriptor_pool;
	   alloc_info.descriptorSetCount          = 1;
	   alloc_info.pSetLayouts                 = &visualization_pipeline_layout->get_descriptor_set_layouts().begin()->second;
	   VK_CHECK(vkAllocateDescriptorSets(get_device().get_handle(), &alloc_info, &visualization_pipeline_descriptor_set));
   }

   // Write bindings to it
   std::map<uint32_t, VkWriteDescriptorSetTensorARM> tensor_bindings =
	   {
		   // Binding 0 is the input tensor
		   {0, VkWriteDescriptorSetTensorARM{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_TENSOR_ARM, nullptr, 1, &input_tensor_view->get_handle()}},
		   // Binding 1 is the output tensor
		   {1, VkWriteDescriptorSetTensorARM{VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_TENSOR_ARM, nullptr, 1, &output_tensor_view->get_handle()}}};

   std::map<uint32_t, VkDescriptorImageInfo> image_bindings =
	   {
		   // Binding 2 is the output image
		   {2, VkDescriptorImageInfo{VK_NULL_HANDLE, output_image_view->get_handle(), VK_IMAGE_LAYOUT_GENERAL}}};

   write_descriptor_set(get_device().get_handle(), visualization_pipeline_descriptor_set, image_bindings, tensor_bindings);
}

/**
* @brief Overridden to recreate the output_image when the window is resized.
*/
bool CameraSuperResolution::resize(uint32_t width, uint32_t height)
{
   // Can't destroy the old image until any outstanding commands are completed
   get_device().wait_idle();

   // Destroy old image and create new one with the new width/height
   prepare_output_image(width, height);

   // Update the descriptor set for the visualization pipeline, so that it writes to the new image
   prepare_visualization_pipeline_descriptor_set();

   return true;
}

/**
* @brief Overridden to do the main rendering on each frame - dispatch our neural network inference and visualize the results.
*/
void CameraSuperResolution::draw_renderpass(vkb::core::CommandBufferC &command_buffer, RenderTargetType &render_target)
{
   // Bind and run data graph pipeline.
   vkCmdBindPipeline(command_buffer.get_handle(), VK_PIPELINE_BIND_POINT_DATA_GRAPH_ARM, data_graph_pipeline->get_handle());
   vkCmdBindDescriptorSets(command_buffer.get_handle(), VK_PIPELINE_BIND_POINT_DATA_GRAPH_ARM, data_graph_pipeline_layout->get_handle(),
						   0, 1, &data_graph_pipeline_descriptor_set, 0, nullptr);
   vkCmdDispatchDataGraphARM(command_buffer.get_handle(), data_graph_pipeline_session->get_handle(), VK_NULL_HANDLE);

   // Barrier for `output_tensor` (written to by the data graph pipeline above, and read from by the visualization compute shader below)
   VkTensorMemoryBarrierARM tensor_barrier = {VK_STRUCTURE_TYPE_TENSOR_MEMORY_BARRIER_ARM};
   tensor_barrier.tensor                   = output_tensor->get_handle();
   tensor_barrier.srcStageMask             = VK_PIPELINE_STAGE_2_DATA_GRAPH_BIT_ARM;
   tensor_barrier.srcAccessMask            = VK_ACCESS_2_SHADER_WRITE_BIT;
   tensor_barrier.dstStageMask             = VK_PIPELINE_STAGE_2_COMPUTE_SHADER_BIT;
   tensor_barrier.dstAccessMask            = VK_ACCESS_2_SHADER_READ_BIT;
   VkDependencyInfo dependency_info        = {VK_STRUCTURE_TYPE_DEPENDENCY_INFO};
   dependency_info.pNext                   = &tensor_barrier;
   vkCmdPipelineBarrier2(command_buffer.get_handle(), &dependency_info);

   // Transition `output_image` to layout for being written to by the visualization compute shader.
   // We don't care about the old contents so can use VK_IMAGE_LAYOUT_UNDEFINED as the old layout.
   vkb::ImageMemoryBarrier output_image_barrier;
   output_image_barrier.old_layout      = VK_IMAGE_LAYOUT_UNDEFINED;
   output_image_barrier.new_layout      = VK_IMAGE_LAYOUT_GENERAL;
   output_image_barrier.src_stage_mask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
   output_image_barrier.src_access_mask = VK_ACCESS_SHADER_READ_BIT;
   output_image_barrier.dst_stage_mask  = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
   output_image_barrier.dst_access_mask = VK_ACCESS_SHADER_WRITE_BIT;
   command_buffer.image_memory_barrier(*output_image_view, output_image_barrier);

   // Bind and run visualization compute pipeline
   vkCmdBindPipeline(command_buffer.get_handle(), VK_PIPELINE_BIND_POINT_COMPUTE, visualization_pipeline->get_handle());
   vkCmdBindDescriptorSets(command_buffer.get_handle(), VK_PIPELINE_BIND_POINT_COMPUTE, visualization_pipeline_layout->get_handle(),
						   0, 1, &visualization_pipeline_descriptor_set, 0, nullptr);

   struct {
	   uint32_t enable_nn;
	   uint32_t zoom;
	   uint32_t width;
	   uint32_t height;
   } push_data;
   push_data.enable_nn = enable_neural_network ? 1 : 0; // Current runtime value
   push_data.zoom = zoom;
   push_data.width     = output_image->get_extent().width;
   push_data.height    = output_image->get_extent().height;

   // Pass the output_image size as a push constant
   vkCmdPushConstants(command_buffer.get_handle(), visualization_pipeline_layout->get_handle(), VK_SHADER_STAGE_COMPUTE_BIT, 0,
					  sizeof(push_data), &push_data);
   uint32_t group_count_x = (render_target.get_extent().width + 7) / 8;        // The visualization shader has a group size of 8
   uint32_t group_count_y = (render_target.get_extent().height + 7) / 8;
   vkCmdDispatch(command_buffer.get_handle(), group_count_x, group_count_y, 1);

   // Barrier for `output_image` (written by the visualization compute shader above, read by the BlitSubpass below)
   vkb::ImageMemoryBarrier output_image_barrier2;
   output_image_barrier2.old_layout      = VK_IMAGE_LAYOUT_GENERAL;
   output_image_barrier2.new_layout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   output_image_barrier2.src_stage_mask  = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
   output_image_barrier2.src_access_mask = VK_ACCESS_SHADER_WRITE_BIT;
   output_image_barrier2.dst_stage_mask  = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
   output_image_barrier2.dst_access_mask = VK_ACCESS_SHADER_READ_BIT;
   command_buffer.image_memory_barrier(*output_image_view, output_image_barrier2);

   // Call the inherited draw_renderpass function to run our blitting pass to display output_image on the screen, and also draw the GUI.
   // The output_image may have been recreated due to window resize since the last frame, so make sure the BlitSubpass has the latest one.
   static_cast<BlitSubpass &>(*get_render_pipeline().get_subpasses()[0]).set_source(output_image_view.get());
   VulkanSample::draw_renderpass(command_buffer, render_target);
}

/**
* @brief Overridden to show labels for visualized input and output tensors.
*/
void CameraSuperResolution::draw_gui()
{
   get_gui().show_options_window(
	   [this]() {
		   ImGui::Checkbox("Enable Neural Network", &enable_neural_network);
		   ImGui::SliderInt("Zoom", &zoom, 1, 10);
	   },
	   2);
}

std::unique_ptr<CameraSuperResolution> create_camera_super_resolution()
{
   return std::make_unique<CameraSuperResolution>();
}
