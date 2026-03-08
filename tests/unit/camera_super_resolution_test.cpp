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
