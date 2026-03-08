# Framework Simplification Plan

## Goal
Simplify the framework to only include components needed for camera samples while maintaining cross-platform support (Windows, Android, macOS, Linux, iOS).

## Current State Analysis

### What Camera Samples Actually Use
Based on analysis of `camera_super_resolution`:
- VulkanSample base class (device, instance, render context management)
- Custom tensor and data graph pipeline components
- Basic rendering (RenderPipeline, custom BlitSubpass)
- GUI (ImGui integration)
- Stats collection
- Platform abstraction (window, application)
- Resource management (images, buffers, descriptors)
- VGF file loading
- STB image loading/resizing

### What Camera Samples DON'T Use
- ❌ Scene graph system (`scene_graph/`)
- ❌ glTF loader (`gltf_loader.cpp`, `hpp_gltf_loader.cpp`)
- ❌ Built-in subpasses (ForwardSubpass, LightingSubpass, GeometrySubpass)
- ❌ Postprocessing pipeline
- ❌ Heightmap
- ❌ ApiVulkanSample class (uses VulkanSample directly)

## Removal Plan

### Phase 1: Remove Scene Graph System
**Impact**: Removes ~30 files, eliminates tinygltf dependency

#### Files to Remove
```
framework/scene_graph/
├── component.h/cpp
├── node.h/cpp
├── scene.h/cpp
├── script.h/cpp
├── hpp_scene.h
├── components/
│   ├── aabb.h/cpp
│   ├── camera.h/cpp
│   ├── perspective_camera.h/cpp
│   ├── orthographic_camera.h/cpp
│   ├── image.h/cpp
│   ├── light.h/cpp
│   ├── material.h/cpp
│   ├── mesh.h/cpp
│   ├── pbr_material.h/cpp
│   ├── sampler.h/cpp
│   ├── sub_mesh.h/cpp
│   ├── texture.h/cpp
│   ├── transform.h/cpp
│   ├── image/astc.h/cpp
│   ├── image/ktx.h/cpp
│   ├── image/stb.h/cpp
│   ├── hpp_image.h/cpp
│   ├── hpp_material.h
│   ├── hpp_mesh.h
│   ├── hpp_sub_mesh.h
│   └── hpp_texture.h
└── scripts/
    ├── free_camera.h/cpp
    ├── node_animation.h/cpp
    └── animation.h/cpp
```

#### CMakeLists.txt Changes
Remove from `framework/CMakeLists.txt`:
- `SCENE_GRAPH_FILES` variable
- `SCENE_GRAPH_COMPONENT_FILES` variable
- `SCENE_GRAPH_SCRIPTS_FILES` variable
- Remove from `PROJECT_FILES` list

### Phase 2: Remove glTF Loader
**Impact**: Removes 2 files, eliminates tinygltf dependency

#### Files to Remove
```
framework/
├── gltf_loader.h
├── gltf_loader.cpp
├── hpp_gltf_loader.h
└── hpp_gltf_loader.cpp (if exists)
```

#### Code Changes Required
1. **framework/vulkan_sample.h**
   - Remove: `#include "hpp_gltf_loader.h"`
   - Remove: `void load_scene(const std::string &path);` method
   - Remove: `SceneType *scene{nullptr};` member

2. **framework/api_vulkan_sample.h**
   - Remove: `std::unique_ptr<vkb::sg::SubMesh> load_model(...)` method

3. **framework/hpp_api_vulkan_sample.h**
   - Remove: `std::unique_ptr<vkb::scene_graph::components::HPPSubMesh> load_model(...)` method

4. **framework/api_vulkan_sample.cpp**
   - Remove: `load_model()` implementation

5. **framework/hpp_api_vulkan_sample.cpp**
   - Remove: `load_model()` implementation

6. **samples/camera/camera_super_resolution/camera_super_resolution.cpp**
   - Remove: `#include "gltf_loader.h"` (line 28)

#### CMakeLists.txt Changes
Remove from `framework/CMakeLists.txt`:
- Remove `gltf_loader.cpp` from `FRAMEWORK_FILES`
- Remove `gltf_loader.h` from `FRAMEWORK_FILES`

### Phase 3: Remove Built-in Rendering Subpasses
**Impact**: Removes 6 files, keeps custom subpass support

#### Files to Remove
```
framework/rendering/subpasses/
├── forward_subpass.h/cpp
├── lighting_subpass.h/cpp
├── geometry_subpass.h/cpp
└── hpp_forward_subpass.h
```

**Keep**: `framework/rendering/subpass.h` (base class needed for BlitSubpass)

#### CMakeLists.txt Changes
Remove from `framework/CMakeLists.txt`:
- `RENDERING_SUBPASSES_FILES` variable
- Remove from `PROJECT_FILES` list

### Phase 4: Remove Unused Framework Components
**Impact**: Removes 4 files

#### Files to Remove
```
framework/
├── heightmap.h
├── heightmap.cpp
├── api_vulkan_sample.h
├── api_vulkan_sample.cpp
├── hpp_api_vulkan_sample.h
└── hpp_api_vulkan_sample.cpp
```

**Rationale**:
- `heightmap.*` - Not used by camera samples
- `api_vulkan_sample.*` - Camera samples use VulkanSample directly
- `hpp_api_vulkan_sample.*` - Not used

#### CMakeLists.txt Changes
Remove from `framework/CMakeLists.txt`:
- Remove these files from `FRAMEWORK_FILES`

### Phase 5: Remove Postprocessing Pipeline
**Impact**: Removes 5 files

#### Files to Remove
```
framework/rendering/
├── postprocessing_pipeline.h/cpp
├── postprocessing_pass.h/cpp
├── postprocessing_renderpass.h/cpp
└── postprocessing_computepass.h/cpp
```

#### CMakeLists.txt Changes
Remove from `framework/CMakeLists.txt`:
- Remove these files from `RENDERING_FILES`

### Phase 6: Remove tinygltf Submodule
**Impact**: Removes git submodule

#### Steps
1. Remove from `.gitmodules`:
```
[submodule "third_party/tinygltf"]
	path = third_party/tinygltf
	url = https://github.com/syoyo/tinygltf
```

2. Remove from `third_party/CMakeLists.txt`:
```cmake
# tinygltf
add_library(tinygltf INTERFACE)
set(TINYGLTF_DIR ${CMAKE_CURRENT_SOURCE_DIR}/tinygltf)
target_sources(tinygltf INTERFACE ${TINYGLTF_DIR}/tiny_gltf.h ${TINYGLTF_DIR}/json.hpp)
target_include_directories(tinygltf SYSTEM INTERFACE ${TINYGLTF_DIR})
```

3. Remove from `framework/CMakeLists.txt`:
```cmake
target_link_libraries(${PROJECT_NAME} PUBLIC
    # ... other libs ...
    tinygltf  # REMOVE THIS LINE
    # ... other libs ...
)
```

4. Remove from `third_party/README.adoc`:
- Remove tinygltf entry

5. Execute git commands:
```bash
git add .gitmodules
git rm -f third_party/tinygltf
rm -rf .git/modules/third_party/tinygltf
```

### Phase 7: Optional - Remove Unused Texture Libraries
**Impact**: Could remove ktx, astc if not used for visualization

**Analysis Needed**: Check if camera samples use KTX/ASTC for output visualization
- If yes: Keep them
- If no: Could remove, but they're lightweight

**Recommendation**: Keep for now, useful for texture output/debugging

## Implementation Order

### Step 1: Remove Scene Graph (Safest First)
1. Remove scene graph files
2. Update framework/CMakeLists.txt
3. Test build on all platforms

### Step 2: Remove glTF Loader
1. Remove gltf_loader files
2. Update vulkan_sample.h
3. Remove api_vulkan_sample files
4. Update framework/CMakeLists.txt
5. Test build on all platforms

### Step 3: Remove Built-in Subpasses
1. Remove subpass files
2. Update framework/CMakeLists.txt
3. Test build on all platforms

### Step 4: Remove Postprocessing & Heightmap
1. Remove files
2. Update framework/CMakeLists.txt
3. Test build on all platforms

### Step 5: Remove tinygltf Submodule
1. Update all CMakeLists.txt files
2. Remove submodule
3. Test build on all platforms

## Testing Strategy

After each phase:
1. **Build test**: `cmake --build build/mac-xcode --config Release --target camera_super_resolution -j$(sysctl -n hw.ncpu)`
2. **Unit tests**: Run all camera sample tests
3. **Runtime test**: Run camera_super_resolution sample
4. **Cross-platform**: Test on at least 2 platforms before proceeding

## Expected Benefits

### Code Reduction
- **~40 files removed** (~15,000+ lines of code)
- **1 submodule removed** (tinygltf)
- **Simpler dependency tree**

### Build Time Improvement
- Fewer files to compile
- Simpler template instantiations
- Reduced header dependencies

### Maintenance Benefits
- Clearer code purpose (camera-focused)
- Easier to understand for new contributors
- Less code to maintain

### Repository Size
- Smaller git history (after submodule removal)
- Faster clones

## Risks & Mitigation

### Risk 1: Breaking Platform-Specific Code
**Mitigation**: Test on all platforms after each phase

### Risk 2: Hidden Dependencies
**Mitigation**: Use compiler errors to identify missing dependencies, add back if needed

### Risk 3: Future Sample Additions
**Mitigation**: Document what was removed and why, can be added back if needed

## Rollback Plan

Each phase is independent and can be rolled back via git:
```bash
git revert <commit-hash>
```

Keep each phase as a separate commit for easy rollback.

## Platform Support Verification

After all phases, verify builds on:
- ✅ macOS (current platform)
- ⬜ Windows
- ⬜ Linux
- ⬜ Android
- ⬜ iOS

## Summary

This plan removes ~40 files and 1 submodule while maintaining:
- ✅ All platform support
- ✅ Camera sample functionality
- ✅ Custom tensor/data graph support
- ✅ GUI and stats
- ✅ Resource management
- ✅ Rendering pipeline

The framework will be ~30% smaller and focused on camera ML inference use cases.
