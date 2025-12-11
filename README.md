# Cinder

![Vulkan](https://img.shields.io/badge/Vulkan-1.3-%23AC162C?style=flat&logo=vulkan)
![C++](https://img.shields.io/badge/C%2B%2B-20-blue?style=flat&logo=c%2B%2B)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey?style=flat&logo=windows)
![License](https://img.shields.io/badge/License-MIT-green)

Experiments with Vulkan1.3.

---

## 🗺️ Roadmap

### Phase I: The Foundation (Legacy & Basics)
*Establish the baseline Vulkan context and get geometry on the screen.*

- [x] **Context Initialization**: Setup Instance, Physical Device, Logical Device, and Queue families.
- [x] **Swapchain Architecture**: Implement Swapchain, Image Views, and Frame Presentation logic.
- [x] **Hello Triangle**: Render the first triangle using Legacy RenderPass/Framebuffer compatibility path.
- [ ] **Basic RHI**: Abstract the main loop, implement Staging Buffers, and basic Camera (MVP) logic.

### Phase II: RHI Modernization
*Transition from legacy Vulkan 1.0 constructs to modern 1.3+ standards.*

- [ ] **Dynamic Rendering**: Remove all `VkRenderPass` and `VkFramebuffer` objects; migrate to `VK_KHR_dynamic_rendering`.
- [ ] **Synchronization 2**: Replace binary semaphores with **Timeline Semaphores** and implement `vkCmdPipelineBarrier2`.
- [ ] **Imageless Framebuffers**: Implement fallback architecture for mobile tile-based rendering optimizations.
- [ ] **RHI Abstraction layer**: Finalize the separation between high-level rendering logic and low-level Vulkan calls.

### Phase III: The Bindless Revolution
*Overhaul resource management to enable "Bindless" workflows.*

- [ ] **Buffer Device Address (BDA)**: Implement `GpuPtr<T>` to access vertex/index buffers via physical GPU pointers.
- [ ] **Bindless Textures**: Implement `descriptorIndexing` (UpdateAfterBind) for global texture arrays.
- [ ] **Material System**: Design a Texture-ID based material system utilizing the bindless heap.
- [ ] **Descriptor Buffers**: Integrate `VK_EXT_descriptor_buffer` for manual descriptor memory management.

### Phase IV: Shader Infrastructure (Slang)
*Integrate Slang language to manage complexity and compile times.*

- [ ] **Graphics Pipeline Library (GPL)**: Implement fast pipeline linking to solve PSO compilation stuttering.
- [ ] **Slang Integration**: Embed the Slang compiler and replace HLSL build chain.
- [ ] **Modern Shader Interface**: Wrap BDA and Bindless resources into Slang `Parameter Blocks`.
- [ ] **Shader Modularization**: Refactor core generic logic using Slang Interfaces (`IGeometry`, `IMaterial`).

### Phase V: GPU-Driven Rendering
*Move scene management and culling from CPU to GPU.*

- [ ] **Indirect Drawing**: Implement `vkCmdDrawIndexedIndirect` with GPU-generated commands.
- [ ] **Compute Culling (Basic)**: Implement Frustum Culling and Instance Culling using Compute Shaders.
- [ ] **Two-Phase Culling**: Implement Hi-Z Occlusion Culling with temporal reprojection.
- [ ] **Scene Management**: Support high-density instance rendering with GPU-side compaction.
- [ ] **Performance Polish**: Final profiling (Nsight/RGP) and memory barrier tuning.

### Phase X: Next-Gen Others
- [ ] **Asset Pipeline**: Custom binary mesh loader.
- [ ] **Visuals**: Disney PBR lighting & CSM with PCF & LUT based **Sky Atmosphere**.
- [ ] **Profiling**: **ImGui** & **RenderDoc API**.
- [ ] **Mesh Shaders**: Hardware pipeline implementation.
- [ ] **VisBuffer**: ID Buffer rendering & Material reconstruction.
- [ ] **Software Rasterizer**: Compute Shader rasterizer (64-bit Atomics).
- [ ] **Hybrid Pipeline**: Dynamic switching between HW/SW Rasterization.
- [ ] **Streaming**: Error-metric based LOD selection (DAG).
- [ ] **SDF**: Mesh Distance Field & Global Clipmap generation.
- [ ] **SWRT**: Software Ray Tracing (Sphere Tracing).
- [ ] **Radiance Cache**: Probe hierarchy update.
- [ ] **Hybrid Shadows**: Blending CSM + SDF Shadows.
- [ ] **RTX 40**: Integration of **SER** and **OMM**.
- [ ] **Denoising**: Separate Temporal/Spatial filter chain.
- [ ] **Runtime Config**: Full ImGui-based feature toggling system.
- [ ] **HDR**: 10-bit Output & Tone Mapping.
- [ ] **DLSS 4**: Streamline SDK integration (SR + FG).
- [ ] **Optimization**: TaskGraph load balancing & LRU streaming.

---

## 🚀 Getting Started

### Prerequisites
* **Visual Studio 2022** (C++ Desktop Development)
* **Vulkan SDK** (Vulkan 1.3+ with SDL2, GLM)
* **CMake** (3.25+)
