# SPDX-License-Identifier: Apache-2.0
#
# Licensed under the Apache License, Version 2.0 the "License";
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

add_sample_with_tags(
        ID "camera_super_resolution"
        CATEGORY "extensions/tensor_and_data_graph"
        AUTHOR "Arm Ltd."
        NAME "Camera Super Resolution (XLSR)"
        DESCRIPTION "Demonstrates machine learning inference using XLSR super resolution model"
        TAGS
        "arm"
        FILES
        "camera_super_resolution.cpp"
        "camera_super_resolution.h"
        SHADER_FILES_GLSL
        "tensor_and_data_graph/camera_super_resolution/camera_visualization.comp"
        SHADER_FILES_SPVASM
)