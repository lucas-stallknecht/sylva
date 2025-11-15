# 🌿 Sylva — GPU-Driven Foliage Renderer

**Sylva** is a prototype showcasing **GPU-driven foliage rendering**, combining compute-based generation and raster rendering using [**Daxa**](https://daxa.dev/), a modern Vulkan wrapper.

Sylva requires the **Vulkan SDK** to be installed and available in your system’s environment.
You can obtain it from [LunarG’s Vulkan SDK](https://vulkan.lunarg.com/).

---

## Roadmap

- [x] ImGui integration + camera controls
- [x] Terrain generation (noise-based height, albedo and normal maps)
- [x] Tessellated terrain rendering
  - [ ] Dynamic levels
- [ ] Grass generation (chunking and clumping) and rendering
- [ ] Wind simulation
- [ ] Grass shading (Phong + screen-space shadows)
- [ ] Optimization (culling, LODs)
- [ ] Additional foliage (moss, etc.)

---

## 🛠️ Building Sylva

Sylva is developed in C++20 and built using CMake (version 3.21 or higher). It uses build presets for streamlined configuration.

### Example (Windows + MSVC)

#### 1. Configure the project

```bash
cmake --preset cl-x86_64-windows-msvc
```

#### 2. Build (Debug preset)

```bash
cmake --build --preset cl-x86_64-windows-msvc-debug
```

#### 3. Run the executable

```bash
.\build\cl-x86_64-windows-msvc\Debug\sylva.exe
```

(Adjust the path as needed for your platform and configuration.)

<details>
  <summary><b>Show Current Project Progress</b></summary>

_Procedural terrain generation_

![stage_preview](misc/stage_preview.png)

</details>
