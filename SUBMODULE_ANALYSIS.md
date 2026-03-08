# Git Submodule Dependency Analysis

## Overview
This document analyzes all remaining git submodules to determine which are essential for the camera samples and which could potentially be removed.

## Current Repository Scope
This repository contains only **camera samples** (specifically `camera_super_resolution`), not the full Vulkan Samples collection. The framework was inherited from the main Vulkan Samples repository and includes many dependencies that may not be needed for camera-only samples.

## Submodule Analysis

### ✅ ESSENTIAL - Cannot Remove

#### 1. `third_party/volk` - Vulkan Meta-Loader
- **Status**: REQUIRED
- **Used by**: Framework core (all Vulkan API calls)
- **Purpose**: Dynamic Vulkan function loading
- **Evidence**: Linked by framework, used throughout core/

#### 2. `third_party/vulkan` - Vulkan Headers
- **Status**: REQUIRED
- **Used by**: Framework core, all Vulkan code
- **Purpose**: Vulkan API definitions
- **Evidence**: Included everywhere, fundamental dependency

#### 3. `third_party/vma` - Vulkan Memory Allocator
- **Status**: REQUIRED
- **Used by**: Framework core (memory management)
- **Purpose**: Efficient Vulkan memory allocation
- **Evidence**: Linked by framework, used in core/allocated.cpp

#### 4. `third_party/glm` - OpenGL Mathematics
- **Status**: REQUIRED
- **Used by**: Framework (matrices, vectors, transforms)
- **Purpose**: Math library for graphics
- **Evidence**: Linked by framework, used throughout

#### 5. `third_party/spdlog` - Logging Library
- **Status**: REQUIRED
- **Used by**: Framework core, all components
- **Purpose**: Structured logging
- **Evidence**: Linked by vkb__core and framework
- **Note**: Depends on fmt

#### 6. `third_party/fmt` - Formatting Library
- **Status**: REQUIRED (dependency of spdlog)
- **Used by**: spdlog, framework
- **Purpose**: String formatting
- **Evidence**: Linked by spdlog and framework

#### 7. `third_party/ai-ml-sdk-vgf-library` - VGF Library
- **Status**: REQUIRED
- **Used by**: Camera samples directly
- **Purpose**: Load and decode VGF model files
- **Evidence**: 
  - `#include <vgf/decoder.h>` in camera_super_resolution.cpp
  - Used in tensor_and_data_graph_common.cpp
  - Linked explicitly by camera_super_resolution

#### 8. `third_party/flatbuffers` - Serialization Library
- **Status**: REQUIRED (dependency of VGF library)
- **Used by**: VGF library internally
- **Purpose**: Binary serialization format
- **Evidence**: VGF library depends on it

#### 9. `third_party/spirv-cross` - SPIR-V Cross-Compiler
- **Status**: REQUIRED
- **Used by**: Framework (shader reflection)
- **Purpose**: Parse and reflect SPIR-V shaders
- **Evidence**: Linked by framework, used in spirv_reflection.cpp

### 🟡 PLATFORM-SPECIFIC - Conditionally Required

#### 10. `third_party/glfw` - Window/Input Library
- **Status**: REQUIRED (except Android/iOS/Direct-to-Display)
- **Used by**: Framework platform layer (desktop)
- **Purpose**: Window creation and input handling
- **Evidence**: Linked by framework on desktop platforms
- **Note**: Not used on Android (uses game-activity) or iOS

#### 11. `third_party/hwcpipe` - Hardware Counters
- **Status**: ANDROID ONLY
- **Used by**: Framework stats (Android only)
- **Purpose**: Hardware performance counters
- **Evidence**: Only built and linked on Android
- **Recommendation**: Keep (no harm, Android-specific)

### ⚠️ QUESTIONABLE - May Not Be Needed for Camera Samples

#### 12. `third_party/imgui` - Immediate Mode GUI
- **Status**: LINKED but usage unclear
- **Used by**: Framework (GUI rendering)
- **Purpose**: Debug UI and overlays
- **Evidence**: Linked by framework
- **Analysis**: Camera samples may not need GUI
- **Recommendation**: KEEP for now (useful for debugging/stats display)

#### 13. `third_party/stb` - Image Loading
- **Status**: LINKED but usage unclear for camera samples
- **Used by**: Framework (scene_graph/components/image/stb.cpp)
- **Purpose**: Load PNG/JPG images
- **Evidence**: Linked by framework
- **Analysis**: Camera samples may not load image files
- **Recommendation**: KEEP (lightweight, header-only)

#### 14. `third_party/ktx` - Khronos Texture Library
- **Status**: LINKED but usage unclear for camera samples
- **Used by**: Framework (scene_graph/components/image/ktx.cpp)
- **Purpose**: Load KTX texture files
- **Evidence**: Linked by framework
- **Analysis**: Camera samples may not load texture files
- **Recommendation**: KEEP (may be used for output/visualization)

#### 15. `third_party/astc` - ASTC Texture Encoder
- **Status**: LINKED but usage unclear for camera samples
- **Used by**: Framework (scene_graph/components/image/astc.cpp)
- **Purpose**: Decode ASTC compressed textures
- **Evidence**: Linked by framework
- **Analysis**: Camera samples may not use ASTC textures
- **Recommendation**: KEEP (may be used for output/visualization)

#### 16. `third_party/tinygltf` - glTF Parser
- **Status**: LINKED but NOT USED by camera samples
- **Used by**: Framework (gltf_loader.cpp)
- **Purpose**: Load 3D models in glTF format
- **Evidence**: 
  - Linked by framework
  - camera_super_resolution.cpp includes gltf_loader.h but doesn't use it
  - No GLTFLoader, load_scene, or load_model calls in camera samples
- **Analysis**: Camera samples don't load 3D models
- **Recommendation**: COULD REMOVE if framework is refactored

### 🔵 OPTIONAL - Feature Flags

#### 17. `third_party/tracy` - Profiler
- **Status**: OPTIONAL (VKB_PROFILING=OFF by default)
- **Used by**: Framework (when VKB_PROFILING=ON)
- **Purpose**: Performance profiling
- **Evidence**: Only built when VKB_PROFILING is enabled
- **Recommendation**: KEEP (useful for development, no cost when disabled)

## Summary

### Cannot Remove (13 submodules)
Essential for camera samples to function:
- volk, vulkan, vma, glm, spdlog, fmt
- ai-ml-sdk-vgf-library, flatbuffers
- spirv-cross
- glfw (desktop), hwcpipe (Android)
- imgui, stb, ktx, astc (useful for visualization)

### Could Remove with Framework Refactoring (1 submodule)
- **tinygltf** - Not used by camera samples, only by framework's gltf_loader

### Optional (1 submodule)
- **tracy** - Only used when profiling is enabled

## Recommendations

### Short Term
1. ✅ **Already removed**: opencl (unused)
2. **Keep all others**: They're either essential or lightweight enough not to matter

### Long Term (If Camera-Only Repository)
If this repository is meant to be camera-samples-only (not general Vulkan samples):

1. **Consider removing**:
   - `tinygltf` - Not used by camera samples
   - Could refactor framework to remove gltf_loader dependency

2. **Consider creating a minimal framework**:
   - Remove scene graph components (not used by camera samples)
   - Remove gltf_loader (not used by camera samples)
   - This would eliminate the tinygltf dependency

3. **Keep everything else**:
   - All other dependencies are either essential or useful for camera samples

## Verification Commands

To verify if a submodule is used:
```bash
# Check for includes
rg "#include.*<library>" --type cpp

# Check for CMake links
rg "target_link_libraries.*library" --type cmake

# Check for actual usage
rg "LibraryClass|library_function" --type cpp
```

## Conclusion

After removing OpenCL, all remaining submodules are either:
1. Essential for Vulkan functionality
2. Essential for camera samples (VGF library)
3. Platform-specific requirements
4. Lightweight and useful for debugging/visualization
5. Optional features (tracy)

The only candidate for removal is **tinygltf**, but it would require refactoring the framework to remove the gltf_loader component. This is a low-priority optimization since tinygltf is header-only and doesn't significantly impact build times or repository size.
