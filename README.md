# Cinder

![Vulkan](https://img.shields.io/badge/Vulkan-1.3-%23AC162C?style=flat&logo=vulkan)
![C++](https://img.shields.io/badge/C%2B%2B-20-blue?style=flat&logo=c%2B%2B)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey?style=flat&logo=windows)
![License](https://img.shields.io/badge/License-MIT-green)

---

## 🗺️ Roadmap

### Phase I: Foundation & RHI Abstraction
- [x] **Device Strategy**: Multi-Queue support (Async Compute) & Physical device ranking.
- [ ] **Memory Management**: **VMA** integration with sub-allocation logic.
- [ ] **RHI Abstraction**: Encapsulating commands into `RHICommandList`.
- [ ] **Shader System**: Automated layout reflection via `SPIRV-Reflect`.
- [ ] **Bindless**: **Descriptor Indexing** implementation (Global Texture Array).

### Phase II: The GPU Driven Core
- [ ] **Vertex Data**: Implementing **Pull-Mode Vertex Fetching** (SSBO replacement for VBOs).
- [ ] **GPU Scene**: Uploading instance data (Transforms/MaterialIDs) to persistent GPU buffers.
- [ ] **Culling**: Implementing Compute Shader-based **Frustum Culling**.
- [ ] **MDI**: Integrating **Multi-Draw Indirect** for single-draw-call rendering.
- [ ] **Async Compute**: Offloading culling tasks to a dedicated compute queue with ownership transfers.

### Phase III: Visuals & Engineering Polish
- [ ] **Asset Pipeline**: Custom binary mesh loader.
- [ ] **Visuals**: Disney PBR lighting & CSM with PCF & LUT based **Sky Atmosphere**.
- [ ] **Profiling**: **ImGui** & **RenderDoc API**.

### Phase IV: Mobile Deferred (PLS)
- [ ] **Pipeline Architecture**: Pipeline based on Vulkan Subpass and Metal Tile Shading.
- [ ] **Memory Optimization**: Implement **Memoryless GBuffer**.
- [ ] **Bandwidth Verification**: Ensuring minimal memory bandwidth usage.
- [ ] **Lighting Calculation**: Implementing the Pixel Local Storage (PLS) pattern.

### Phase V: Mobile Mesh Shader
- [ ] **Data Preprocessing**: Integrate `meshoptimizer` to partition mesh geometry into **Meshlets**.
- [ ] **API Adaptation**: **Object/Mesh Shader** API on Metal and `VK_EXT_mesh_shader` extension on Vulkan.
- [ ] **Geometry Culling**: Cluster-based Frustum Culling and Backface Culling.

### Phase VI: Mobile SDF GI
- [ ] **SDF Generation**: Convert static meshes into **Mesh Distance Fields (MDF)** 3D textures.
- [ ] **Ray Marching**: Lightweight **Sphere Tracing** Shaders.
- [ ] **Scene Management**: A simplified **Global Distance Field**.
- [ ] **Hybrid Rendering**: Blend with the main rendering pipeline.

### Phase VII: Next-Gen Geometry
- [ ] **Mesh Shaders**: Hardware pipeline implementation.
- [ ] **VisBuffer**: ID Buffer rendering & Material reconstruction.
- [ ] **Software Rasterizer**: Compute Shader rasterizer (64-bit Atomics).
- [ ] **Hybrid Pipeline**: Dynamic switching between HW/SW Rasterization.
- [ ] **Streaming**: Error-metric based LOD selection (DAG).

### Phase VIII: Global Illumination
- [ ] **SDF**: Mesh Distance Field & Global Clipmap generation.
- [ ] **SWRT**: Software Ray Tracing (Sphere Tracing).
- [ ] **Radiance Cache**: Probe hierarchy update.
- [ ] **Hybrid Shadows**: Blending CSM + SDF Shadows.
- [ ] **RTX 40**: Integration of **SER** and **OMM**.
- [ ] **Denoising**: Separate Temporal/Spatial filter chain.

### Phase IX: Engineering
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
