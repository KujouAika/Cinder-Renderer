# Cinder

![Vulkan](https://img.shields.io/badge/Vulkan-1.3-%23AC162C?style=flat&logo=vulkan)
![C++](https://img.shields.io/badge/C%2B%2B-20-blue?style=flat&logo=c%2B%2B)
![Platform](https://img.shields.io/badge/Platform-Windows-lightgrey?style=flat&logo=windows)
![License](https://img.shields.io/badge/License-MIT-green)

Experiments with Vulkan1.3.

---

## 🗺️ Roadmap

### Phase I: Foundation & RHI Abstraction
- [x] **Device Strategy**: Multi-Queue support (Async Compute) & Physical device ranking.
- [x] **Memory Management**: **VMA** integration with sub-allocation logic.
- [ ] **Render Pass & Framebuffers**: Define attachments (Load/Store ops) & render targets.
- [ ] **Synchronization**: Fences & Semaphores for Frames-in-Flight.
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

### Phase IV: Next-Gen Geometry
- [ ] **Mesh Shaders**: Hardware pipeline implementation.
- [ ] **VisBuffer**: ID Buffer rendering & Material reconstruction.
- [ ] **Software Rasterizer**: Compute Shader rasterizer (64-bit Atomics).
- [ ] **Hybrid Pipeline**: Dynamic switching between HW/SW Rasterization.
- [ ] **Streaming**: Error-metric based LOD selection (DAG).

### Phase V: Global Illumination
- [ ] **SDF**: Mesh Distance Field & Global Clipmap generation.
- [ ] **SWRT**: Software Ray Tracing (Sphere Tracing).
- [ ] **Radiance Cache**: Probe hierarchy update.
- [ ] **Hybrid Shadows**: Blending CSM + SDF Shadows.
- [ ] **RTX 40**: Integration of **SER** and **OMM**.
- [ ] **Denoising**: Separate Temporal/Spatial filter chain.

### Phase VI: Engineering
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
